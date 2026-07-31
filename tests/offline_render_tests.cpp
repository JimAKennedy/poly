// M053 S11 (T02): Offline SMF render tests.
//
// poly::renderPatternToSMF is the single primitive behind the WebUI Export chip
// (Save-As and drag-to-DAW). Its contract is that it renders the CURRENT
// pattern to a valid Standard MIDI File blob with NO DAW transport involvement:
// there is no TransportContext argument, so it works identically whether the
// host is playing, stopped, or absent (the offline path fabricates its own
// playing transport internally). These tests pin that contract:
//   - produces note events from a plain SceneState (no live transport),
//   - is deterministic (same inputs -> byte-identical blob),
//   - hardens every out-of-range argument instead of throwing/hanging.

#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

#include <gtest/gtest.h>

#include "poly/offline_render.h"
#include "poly/presets.h"
#include "poly/scene.h"
#include "poly/smf_writer.h"
#include "poly/types.h"

using namespace poly;

namespace {

// Heuristic note-on counter mirroring the scan in smf_writer_tests.cpp: the
// track data begins at byte 22 (14-byte MThd header + 8-byte MTrk chunk
// header). writeSMF emits an explicit 0x9n status for every note-on and 0x8n
// for every note-off, so a status-nibble scan is a good-enough presence check.
int countNoteOns(const std::vector<uint8_t>& smf) {
    int count = 0;
    for (size_t i = 22; i + 2 < smf.size(); ++i)
        if ((smf[i] & 0xF0) == 0x90)
            ++count;
    return count;
}

bool hasSmfHeader(const std::vector<uint8_t>& smf) {
    return smf.size() >= 14u && smf[0] == 'M' && smf[1] == 'T' && smf[2] == 'h' && smf[3] == 'd';
}

uint32_t extractTempoMeta(const std::vector<uint8_t>& smf) {
    return (static_cast<uint32_t>(smf[26]) << 16) | (static_cast<uint32_t>(smf[27]) << 8) |
           static_cast<uint32_t>(smf[28]);
}

// A minimal, fully-specified scene whose sceneA has one lane that fires 3 base
// hits over an 8-step (one-bar) cycle with no mutation/probability jitter, so
// the emitted note stream is exactly predictable and macro-independent enough
// to always produce notes.
SceneState makeDeterministicScene() {
    SceneState scene{};
    scene.select = SceneSelect::A;

    GrooveState& g = scene.sceneA;
    g.activeLaneCount = 1;
    g.seed = 42;

    LaneConfig& lane = g.lanes[0];
    lane.id = 0;
    lane.midiNote = 36;
    lane.cycle = {.steps = 8, .subdivision = 8}; // 8 eighth-note steps = 1 bar
    lane.hitCount = 3;
    lane.baseVelocity = 100;
    lane.probability = 1.0f;
    lane.mutationRate = 0.0f;
    lane.emphasisProb = 1.0f;
    lane.velocitySpread = 0.0f;
    lane.ghostFloor = 0;
    lane.active = true;
    return scene;
}

} // namespace

// --- Core contract: produces notes with no live transport --------------------

TEST(OfflineRender, ProducesNotesFromStoppedTransport) {
    // No TransportContext is passed in — the export path never consults the DAW
    // transport, so a stopped/absent host still yields a populated MIDI file.
    const SceneState scene = makeDeterministicScene();
    const auto smf = renderPatternToSMF(scene, /*bars=*/2, /*tempo=*/120.0);

    ASSERT_TRUE(hasSmfHeader(smf));
    // 3 hits/bar * 2 bars = 6 base note-ons, deterministically.
    EXPECT_EQ(countNoteOns(smf), 6) << "8-step/3-hit lane over 2 bars should emit exactly 6 notes";
}

TEST(OfflineRender, ProducesNotesFromFactoryPreset) {
    // Exercise a realistic multi-lane patch through the same macro/constraint
    // resolution the live plugin uses, proving the primitive works on shipped
    // content, not just a hand-built lane.
    ASSERT_GT(kFactoryPresetCount, 0);
    SceneState scene{};
    scene.select = SceneSelect::A;
    scene.sceneA = makeFactoryPreset(0);

    const auto smf = renderPatternToSMF(scene, /*bars=*/4, /*tempo=*/120.0);
    ASSERT_TRUE(hasSmfHeader(smf));
    EXPECT_GT(countNoteOns(smf), 0) << "factory preset 0 should emit notes over 4 bars";
}

// --- Determinism -------------------------------------------------------------

TEST(OfflineRender, DeterministicAcrossCalls) {
    // Same (scene, bars, tempo, timeSig) must produce a byte-identical blob:
    // renderRange derives all phase from absolute PPQ + seed and carries no
    // cross-block state, so repeated exports never differ.
    const SceneState scene = makeDeterministicScene();
    const auto first = renderPatternToSMF(scene, 4, 128.0, 7, 8);
    const auto second = renderPatternToSMF(scene, 4, 128.0, 7, 8);
    EXPECT_EQ(first, second);
}

TEST(OfflineRender, DeterministicForFactoryPreset) {
    SceneState scene{};
    scene.select = SceneSelect::A;
    scene.sceneA = makeFactoryPreset(0);
    const auto first = renderPatternToSMF(scene, 8, 100.0);
    const auto second = renderPatternToSMF(scene, 8, 100.0);
    EXPECT_EQ(first, second);
}

TEST(OfflineRender, DifferentTempoChangesBlob) {
    // Sanity guard on the determinism test: the blob is a real function of its
    // inputs, not a constant. Tempo changes the serialized tempo meta (and the
    // block-stepping), so the blobs must differ.
    const SceneState scene = makeDeterministicScene();
    EXPECT_NE(renderPatternToSMF(scene, 2, 90.0), renderPatternToSMF(scene, 2, 140.0));
}

// --- Negative / boundary hardening (Q7) --------------------------------------
//
// renderPatternToSMF has no external dependencies (pure in-memory compute), so
// its only failure surface is out-of-range arguments. The primitive clamps
// rather than throwing so the Export chip always yields a valid SMF blob.

TEST(OfflineRender, ZeroBarsRendersOneBar) {
    // bars <= 0 is coerced to a single bar, not an empty/invalid file.
    const SceneState scene = makeDeterministicScene();
    const auto smf = renderPatternToSMF(scene, /*bars=*/0, 120.0);
    ASSERT_TRUE(hasSmfHeader(smf));
    EXPECT_EQ(countNoteOns(smf), 3) << "zero bars should render exactly one bar (3 hits)";
}

TEST(OfflineRender, NegativeBarsRendersOneBar) {
    const SceneState scene = makeDeterministicScene();
    const auto smf = renderPatternToSMF(scene, /*bars=*/-5, 120.0);
    ASSERT_TRUE(hasSmfHeader(smf));
    EXPECT_EQ(countNoteOns(smf), 3);
}

TEST(OfflineRender, ExcessiveBarsClampToMaxAndTerminate) {
    // Pathological bar counts are capped at kMaxRenderBars so memory/time stay
    // bounded and the block loop terminates. Equivalence to the clamped value
    // proves the cap, not merely that it returns.
    const SceneState scene = makeDeterministicScene();
    const auto huge = renderPatternToSMF(scene, /*bars=*/1'000'000, 120.0);
    const auto capped = renderPatternToSMF(scene, kMaxRenderBars, 120.0);
    ASSERT_TRUE(hasSmfHeader(huge));
    EXPECT_EQ(huge, capped) << "bars beyond kMaxRenderBars must clamp to kMaxRenderBars";
}

TEST(OfflineRender, NonFiniteTempoClampsToMinimum) {
    // NaN/inf tempo must not divide-by-zero or produce inf PPQ steps (infinite
    // loop). It is floored to kSmfMinTempo for both the stepping math and the
    // serialized tempo meta.
    const SceneState scene = makeDeterministicScene();
    for (double bad : {std::nan(""), std::numeric_limits<double>::infinity(), 0.0, -120.0, 1.0}) {
        const auto smf = renderPatternToSMF(scene, 1, bad);
        ASSERT_TRUE(hasSmfHeader(smf));
        ASSERT_GE(smf.size(), 29u);
        const uint32_t expected = static_cast<uint32_t>(60000000.0 / kSmfMinTempo);
        EXPECT_EQ(extractTempoMeta(smf), expected) << "tempo " << bad << " should clamp to kSmfMinTempo";
    }
}

TEST(OfflineRender, DegenerateTimeSignatureFallsBackToFourFour) {
    // Non-positive numerator/denominator fall back to 4/4 rather than producing
    // NaN bar lengths. Result must equal an explicit 4/4 render.
    const SceneState scene = makeDeterministicScene();
    const auto degenerate = renderPatternToSMF(scene, 2, 120.0, /*num=*/0, /*den=*/0);
    const auto fourFour = renderPatternToSMF(scene, 2, 120.0, 4, 4);
    ASSERT_TRUE(hasSmfHeader(degenerate));
    EXPECT_EQ(degenerate, fourFour);
}

TEST(OfflineRender, EmptySceneStillProducesValidFile) {
    // A scene with no active lanes has no notes to emit, but the export must
    // still return a structurally valid SMF (header + empty track), never an
    // empty vector or a throw.
    SceneState scene{};
    scene.sceneA.activeLaneCount = 0;
    const auto smf = renderPatternToSMF(scene, 4, 120.0);
    ASSERT_TRUE(hasSmfHeader(smf));
    // End-of-track meta must still be present.
    const size_t n = smf.size();
    ASSERT_GE(n, 3u);
    EXPECT_EQ(smf[n - 3], 0xFF);
    EXPECT_EQ(smf[n - 2], 0x2F);
    EXPECT_EQ(smf[n - 1], 0x00);
}
