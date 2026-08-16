// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
// M053 S11 (T02) / M032 S01 (T03): Offline SMF render tests.
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
//
// M032 S01: the export now emits a Format-1 multi-track file (conductor track +
// one FF 03-named MTrk per active lane) via writeMultiTrackSMF, so these tests
// walk the Format-1 chunk structure frame-by-frame rather than assuming a single
// Format-0 track at a fixed byte offset. The named-track assertions are the
// milestone demo (open a 4-lane pattern in a DAW, see named tracks) pinned
// offline.

#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "poly/offline_render.h"
#include "poly/presets.h"
#include "poly/scene.h"
#include "poly/smf_writer.h"
#include "poly/types.h"

using namespace poly;

namespace {

uint16_t readBE16(const uint8_t* p) {
    return static_cast<uint16_t>((p[0] << 8) | p[1]);
}

uint32_t readBE32(const uint8_t* p) {
    return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8) | static_cast<uint32_t>(p[3]);
}

bool hasSmfHeader(const std::vector<uint8_t>& smf) {
    return smf.size() >= 14u && smf[0] == 'M' && smf[1] == 'T' && smf[2] == 'h' && smf[3] == 'd';
}

// The conductor track's tempo meta sits at the same absolute offset in both the
// old Format-0 layout and the new Format-1 layout: MThd(14) + MTrk header(8) +
// delta(1) + FF 51 03, so usPerQuarter is at bytes 26..28.
uint32_t extractTempoMeta(const std::vector<uint8_t>& smf) {
    return (static_cast<uint32_t>(smf[26]) << 16) | (static_cast<uint32_t>(smf[27]) << 8) |
           static_cast<uint32_t>(smf[28]);
}

// --- Format-1 frame-aware walkers (mirror smf_writer_tests.cpp) --------------

struct MTrkChunk {
    size_t bodyStart;
    uint32_t len;
};

std::vector<MTrkChunk> parseTracks(const std::vector<uint8_t>& d) {
    std::vector<MTrkChunk> out;
    size_t pos = 14; // past MThd (8 header + 6 body)
    while (pos + 8 <= d.size()) {
        EXPECT_EQ(d[pos + 0], 'M');
        EXPECT_EQ(d[pos + 1], 'T');
        EXPECT_EQ(d[pos + 2], 'r');
        EXPECT_EQ(d[pos + 3], 'k');
        uint32_t len = readBE32(&d[pos + 4]);
        out.push_back({pos + 8, len});
        pos += 8 + len;
    }
    return out;
}

// Count note-on events across every track, respecting event framing so delta or
// meta-length bytes that happen to have a 0x9n nibble never false-match.
int totalNoteOns(const std::vector<uint8_t>& d) {
    int n = 0;
    for (const auto& c : parseTracks(d)) {
        size_t p = c.bodyStart;
        const size_t end = c.bodyStart + c.len;
        while (p < end) {
            while (p < end && (d[p] & 0x80))
                ++p;
            ++p;
            if (p >= end)
                break;
            uint8_t status = d[p++];
            if (status == 0xFF) {
                ++p; // meta type
                uint32_t l = 0;
                while (p < end) {
                    uint8_t b = d[p++];
                    l = (l << 7) | (b & 0x7F);
                    if (!(b & 0x80))
                        break;
                }
                p += l;
            } else {
                uint8_t hi = status & 0xF0;
                if (hi == 0x90)
                    ++n;
                p += (hi == 0xC0 || hi == 0xD0) ? 1u : 2u;
            }
        }
    }
    return n;
}

// Read the FF 03 track-name string. The writer always places it first (delta 0).
std::string trackName(const std::vector<uint8_t>& d, const MTrkChunk& c) {
    size_t p = c.bodyStart;
    const size_t end = c.bodyStart + c.len;
    if (p + 3 > end || d[p + 0] != 0x00 || d[p + 1] != 0xFF || d[p + 2] != 0x03)
        return {};
    p += 3;
    uint32_t l = 0;
    while (p < end) {
        uint8_t b = d[p++];
        l = (l << 7) | (b & 0x7F);
        if (!(b & 0x80))
            break;
    }
    std::string s;
    for (uint32_t i = 0; i < l && p < end; ++i)
        s.push_back(static_cast<char>(d[p++]));
    return s;
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

// Configure one deterministic lane at a given GM note in-place.
void configureDeterministicLane(LaneConfig& lane, int id, int16_t midiNote) {
    lane.id = id;
    lane.midiNote = midiNote;
    lane.cycle = {.steps = 8, .subdivision = 8};
    lane.hitCount = 3;
    lane.baseVelocity = 100;
    lane.probability = 1.0f;
    lane.mutationRate = 0.0f;
    lane.emphasisProb = 1.0f;
    lane.velocitySpread = 0.0f;
    lane.ghostFloor = 0;
    lane.active = true;
}

// A two-lane deterministic scene: lane 0 = Kick (36), lane 1 = Snare (38). Both
// fire, so the Format-1 export yields conductor + Kick + Snare tracks.
SceneState makeTwoLaneScene() {
    SceneState scene{};
    scene.select = SceneSelect::A;
    GrooveState& g = scene.sceneA;
    g.activeLaneCount = 2;
    g.seed = 42;
    configureDeterministicLane(g.lanes[0], 0, 36); // Kick
    configureDeterministicLane(g.lanes[1], 1, 38); // Snare
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
    // Format-1: conductor + 1 active lane = ntrks 2.
    EXPECT_EQ(readBE16(&smf[8]), 1u);
    EXPECT_EQ(readBE16(&smf[10]), 2u);
    // 3 hits/bar * 2 bars = 6 base note-ons, deterministically.
    EXPECT_EQ(totalNoteOns(smf), 6) << "8-step/3-hit lane over 2 bars should emit exactly 6 notes";
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
    EXPECT_EQ(readBE16(&smf[8]), 1u) << "export must be Format-1";
    EXPECT_GT(totalNoteOns(smf), 0) << "factory preset 0 should emit notes over 4 bars";
}

// --- Milestone demo: named tracks, one per lane ------------------------------

TEST(OfflineRender, MultiLaneEmitsOneNamedTrackPerLane) {
    // The M032 demo: opening a multi-lane pattern in a DAW shows one named track
    // per lane. Offline, that is a Format-1 file whose lane MTrks carry FF 03
    // names derived from each lane's GM note.
    const SceneState scene = makeTwoLaneScene();
    const auto smf = renderPatternToSMF(scene, /*bars=*/1, /*tempo=*/120.0);
    ASSERT_TRUE(hasSmfHeader(smf));

    EXPECT_EQ(readBE16(&smf[8]), 1u);  // format 1
    EXPECT_EQ(readBE16(&smf[10]), 3u); // conductor + Kick + Snare

    auto tracks = parseTracks(smf);
    ASSERT_EQ(tracks.size(), 3u);
    // Conductor (track 0) has no name meta; lane tracks are named by GM note.
    EXPECT_EQ(trackName(smf, tracks[1]), "Kick");
    EXPECT_EQ(trackName(smf, tracks[2]), "Snare");
}

TEST(OfflineRender, ConductorTrackCarriesTempoAndNoNotes) {
    const SceneState scene = makeTwoLaneScene();
    const auto smf = renderPatternToSMF(scene, /*bars=*/1, /*tempo=*/120.0);
    auto tracks = parseTracks(smf);
    ASSERT_GE(tracks.size(), 1u);

    // Track 0 body begins with delta 0 + FF 51 03 tempo meta.
    const size_t b = tracks[0].bodyStart;
    ASSERT_GE(smf.size(), b + 7u);
    EXPECT_EQ(smf[b + 0], 0x00);
    EXPECT_EQ(smf[b + 1], 0xFF);
    EXPECT_EQ(smf[b + 2], 0x51);
    EXPECT_EQ(smf[b + 3], 0x03);
    EXPECT_EQ(extractTempoMeta(smf), 500000u);
}

// --- Per-lane export: laneFilter (M032 S02 T02) ------------------------------
//
// laneFilter selects which lanes reach the serializer: -1 (default) exports
// every active lane; N restricts the file to lane N's notes as a single named
// MTrk. Filtering happens after render, so a filtered export is byte-for-byte
// the all-lanes render with the other lanes' tracks removed.

TEST(OfflineRender, LaneFilterEmitsOnlyRequestedLane) {
    // The M032 demo: export a single lane. Filtering the two-lane scene to lane 1
    // yields conductor + exactly the Snare track — no Kick track present.
    const SceneState scene = makeTwoLaneScene();
    const auto smf = renderPatternToSMF(scene, /*bars=*/1, /*tempo=*/120.0, /*num=*/4, /*den=*/4, /*laneFilter=*/1);
    ASSERT_TRUE(hasSmfHeader(smf));

    EXPECT_EQ(readBE16(&smf[8]), 1u);  // format 1
    EXPECT_EQ(readBE16(&smf[10]), 2u); // conductor + Snare only

    auto tracks = parseTracks(smf);
    ASSERT_EQ(tracks.size(), 2u);
    EXPECT_EQ(trackName(smf, tracks[1]), "Snare") << "filtered lane 1 (note 38) names its track Snare";
    // 3 hits/bar over 1 bar for the single retained lane.
    EXPECT_EQ(totalNoteOns(smf), 3) << "only lane 1's 3 notes survive the filter";
}

TEST(OfflineRender, LaneFilterZeroEmitsFirstLaneOnly) {
    // Symmetry check: filtering to lane 0 keeps the Kick, drops the Snare.
    const SceneState scene = makeTwoLaneScene();
    const auto smf = renderPatternToSMF(scene, /*bars=*/1, /*tempo=*/120.0, 4, 4, /*laneFilter=*/0);
    ASSERT_TRUE(hasSmfHeader(smf));

    EXPECT_EQ(readBE16(&smf[10]), 2u); // conductor + Kick only
    auto tracks = parseTracks(smf);
    ASSERT_EQ(tracks.size(), 2u);
    EXPECT_EQ(trackName(smf, tracks[1]), "Kick");
    EXPECT_EQ(totalNoteOns(smf), 3);
}

TEST(OfflineRender, LaneFilterNegativeExportsAllLanes) {
    // -1 is the documented "all lanes" sentinel and the parameter default, so an
    // explicit -1 must be byte-identical to omitting the argument entirely.
    const SceneState scene = makeTwoLaneScene();
    const auto explicitAll = renderPatternToSMF(scene, 1, 120.0, 4, 4, /*laneFilter=*/-1);
    const auto defaultAll = renderPatternToSMF(scene, 1, 120.0);
    EXPECT_EQ(explicitAll, defaultAll);
    // And it really is the full two-lane file.
    EXPECT_EQ(readBE16(&explicitAll[10]), 3u); // conductor + Kick + Snare
}

TEST(OfflineRender, LaneFilterOutOfRangeYieldsConductorOnly) {
    // An index past the active lanes matches no events: the file is still valid,
    // just conductor-only (ntrks 1), never a throw or empty vector.
    const SceneState scene = makeTwoLaneScene();
    const auto smf = renderPatternToSMF(scene, 1, 120.0, 4, 4, /*laneFilter=*/7);
    ASSERT_TRUE(hasSmfHeader(smf));
    EXPECT_EQ(readBE16(&smf[8]), 1u);  // format 1
    EXPECT_EQ(readBE16(&smf[10]), 1u); // conductor only
    EXPECT_EQ(totalNoteOns(smf), 0);
    // End-of-track meta must still terminate the conductor track.
    const size_t n = smf.size();
    ASSERT_GE(n, 3u);
    EXPECT_EQ(smf[n - 3], 0xFF);
    EXPECT_EQ(smf[n - 2], 0x2F);
    EXPECT_EQ(smf[n - 1], 0x00);
}

TEST(OfflineRender, LaneFilterIsDeterministic) {
    // Filtering must not disturb determinism: same inputs -> byte-identical blob.
    const SceneState scene = makeTwoLaneScene();
    const auto first = renderPatternToSMF(scene, 4, 128.0, 7, 8, /*laneFilter=*/1);
    const auto second = renderPatternToSMF(scene, 4, 128.0, 7, 8, /*laneFilter=*/1);
    EXPECT_EQ(first, second);
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
    EXPECT_EQ(totalNoteOns(smf), 3) << "zero bars should render exactly one bar (3 hits)";
}

TEST(OfflineRender, NegativeBarsRendersOneBar) {
    const SceneState scene = makeDeterministicScene();
    const auto smf = renderPatternToSMF(scene, /*bars=*/-5, 120.0);
    ASSERT_TRUE(hasSmfHeader(smf));
    EXPECT_EQ(totalNoteOns(smf), 3);
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
    // serialized tempo meta (which lives in the conductor track).
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
    // still return a structurally valid SMF (Format-1 header + conductor track),
    // never an empty vector or a throw.
    SceneState scene{};
    scene.sceneA.activeLaneCount = 0;
    const auto smf = renderPatternToSMF(scene, 4, 120.0);
    ASSERT_TRUE(hasSmfHeader(smf));
    // Conductor-only: format 1, ntrks 1.
    EXPECT_EQ(readBE16(&smf[8]), 1u);
    EXPECT_EQ(readBE16(&smf[10]), 1u);
    // End-of-track meta must still be present as the final bytes.
    const size_t n = smf.size();
    ASSERT_GE(n, 3u);
    EXPECT_EQ(smf[n - 3], 0xFF);
    EXPECT_EQ(smf[n - 2], 0x2F);
    EXPECT_EQ(smf[n - 1], 0x00);
}
