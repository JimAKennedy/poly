// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
// M068 S03 (T03): Round-trip saved-state migration test.
//
// S02 replaced the retired Bresenham pattern generator (step i pulses iff
// `(i*k) mod n < k`) with Bjorklund. S03 (T01) bumped kStateVersion to 16 and
// made readLaneConfig migrate every non-timeline lane of a pre-v16 state by
// `euclideanMigrationDelta`, so a project saved under the old generator plays
// byte-identically under the new one.
//
// This suite proves that end-to-end: it captures a pre-switch (v15) saved-state
// fixture, loads it through the real readSceneState path (which triggers the
// migration), and asserts the migrated state produces the exact pattern — and
// byte-identical NoteEvent output — that the pre-switch engine produced.
//
// Why the fixture is generated in-test rather than committed as a binary blob:
// the v15 and v16 wire formats are byte-identical (writeLaneConfig has no
// `>= 16` branch — the only v16 change is the read-time rotation migration), so
// serializing the current format and stamping the version int to 15 yields a
// faithful pre-switch fixture. Generating it in-test keeps the fixture
// deterministic, git-tracked, and free of the host-endianness hazards a
// committed binary blob would carry.

#include <array>
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

#include "poly/euclidean.h"
#include "poly/offline_render.h"
#include "poly/scene.h"
#include "poly/state_io.h"
#include "poly/state_io_envelope.h"
#include "poly/types.h"

using namespace poly;

namespace {

// The retired rotation-0 generator, reproduced here so the test's reference is
// independent of the engine (legacyBresenham is file-local in euclidean.cpp and
// deliberately not exported). Mirrors euclidean.cpp's documented legacy formula.
std::array<bool, kMaxSteps> legacyBresenham(int k, int n) {
    std::array<bool, kMaxSteps> out{};
    if (n <= 0 || k <= 0)
        return out;
    if (n > kMaxSteps)
        n = kMaxSteps;
    if (k >= n) {
        for (int i = 0; i < n; ++i)
            out[static_cast<size_t>(i)] = true;
        return out;
    }
    for (int i = 0; i < n; ++i)
        out[static_cast<size_t>(i)] = ((i * k) % n) < k;
    return out;
}

// Right-shift a rotation-0 base pattern by `rotation`, matching euclidean()'s
// wrapping-slice rotation convention. The pre-switch engine rendered the legacy
// base shifted this same way, so `legacyBresenham(k,n) >> rotation` is exactly
// what the retired engine played for a lane with that (k, n, rotation).
std::array<bool, kMaxSteps> rotateRight(const std::array<bool, kMaxSteps>& base, int n, int rotation) {
    std::array<bool, kMaxSteps> out{};
    if (n <= 0)
        return out;
    for (int i = 0; i < n; ++i)
        out[static_cast<size_t>(i)] = base[static_cast<size_t>(((i - rotation) % n + n) % n)];
    return out;
}

// Serialize a SceneState and overwrite the leading int32 version stamp. Used to
// forge a pre-switch (v15) fixture from the current write path.
std::vector<uint8_t> serializeSceneAsVersion(const SceneState& scene, int32_t version) {
    std::vector<uint8_t> bytes;
    const bool ok = writeSceneState(
        [&bytes](const void* data, size_t size) {
            const auto* p = static_cast<const uint8_t*>(data);
            bytes.insert(bytes.end(), p, p + size);
            return true;
        },
        scene);
    EXPECT_TRUE(ok) << "writeSceneState failed";
    EXPECT_GE(bytes.size(), sizeof(int32_t));
    // The version int is the first field writeSceneState emits.
    for (size_t b = 0; b < sizeof(int32_t); ++b)
        bytes[b] = static_cast<uint8_t>((version >> (8 * b)) & 0xFF);
    return bytes;
}

// Read a serialized blob back through the production readSceneState path (which
// applies the migration for pre-v16 versions and sanitizes).
SceneState deserializeScene(const std::vector<uint8_t>& bytes) {
    SceneState scene{};
    size_t pos = 0;
    const bool ok = readSceneState(
        [&bytes, &pos](void* data, size_t size) {
            if (pos + size > bytes.size())
                return false;
            std::copy(bytes.begin() + static_cast<std::ptrdiff_t>(pos),
                      bytes.begin() + static_cast<std::ptrdiff_t>(pos + size), static_cast<uint8_t*>(data));
            pos += size;
            return true;
        },
        scene);
    EXPECT_TRUE(ok) << "readSceneState failed";
    return scene;
}

// A minimal single-lane scene mirroring offline_render_tests' deterministic
// scene: probability 1, no mutation/humanize/spread, neutral macros — so the
// emitted NoteEvent stream is a pure function of the lane's step pattern.
SceneState makeSingleEuclideanLane(int k, int n, int rotation) {
    SceneState scene{};
    scene.select = SceneSelect::A;
    GrooveState& g = scene.sceneA;
    g.activeLaneCount = 1;
    g.seed = 42;

    LaneConfig& lane = g.lanes[0];
    lane.id = 0;
    lane.midiNote = 36;
    lane.cycle = {.steps = n, .subdivision = 8};
    lane.hitCount = k;
    lane.rotation = rotation;
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

// --- Headline: pre-switch fixture yields byte-identical NoteEvent output ------

// A v15 fixture with a non-rotation-invariant lane (delta != 0, so the migration
// actually moves the rotation) must, after loading, render a NoteEvent stream
// byte-identical to what the retired Bresenham engine produced for that lane.
// The reference is injected independently via a timeline lane carrying the
// legacy pattern verbatim, so the comparison does not go through the same
// generator it is validating.
TEST(StateMigration, PreSwitchFixtureProducesByteIdenticalOutput) {
    constexpr int k = 5, n = 8, r0 = 1; // delta(5,8) == 2 (verified), so migration is exercised

    // Forge and load the pre-switch fixture.
    const SceneState authored = makeSingleEuclideanLane(k, n, r0);
    const auto fixture = serializeSceneAsVersion(authored, /*version=*/kBjorklundGeneratorStateVersion - 1);
    const SceneState migrated = deserializeScene(fixture);

    // The migration must have moved the rotation by a non-zero delta.
    const int delta = euclideanMigrationDelta(k, n);
    ASSERT_NE(delta, 0) << "chosen case must exercise a real rotation shift";
    EXPECT_EQ(migrated.sceneA.lanes[0].rotation, (r0 + delta) % n);

    // Reference = the legacy pattern the pre-switch engine played, injected as a
    // timeline lane (generator-independent) with all other fields identical.
    const std::array<bool, kMaxSteps> legacy = rotateRight(legacyBresenham(k, n), n, r0);
    SceneState reference = makeSingleEuclideanLane(k, n, r0);
    LaneConfig& refLane = reference.sceneA.lanes[0];
    refLane.timeline = true;
    refLane.fixedPatternLength = n;
    refLane.fixedPattern = legacy;
    // Route the reference through the same read+sanitize path (v16, no migration)
    // so any normalization applies symmetrically to both scenes.
    const SceneState referenceLoaded =
        deserializeScene(serializeSceneAsVersion(reference, kBjorklundGeneratorStateVersion));

    const auto migratedSmf = renderPatternToSMF(migrated, /*bars=*/4, /*tempo=*/120.0);
    const auto referenceSmf = renderPatternToSMF(referenceLoaded, /*bars=*/4, /*tempo=*/120.0);

    ASSERT_FALSE(migratedSmf.empty());
    EXPECT_EQ(migratedSmf, referenceSmf) << "migrated fixture must render byte-identically to the pre-switch engine";

    // Guard against a vacuous pass: the UNmigrated (v16) reading of the same
    // bytes — i.e. Bjorklund at the raw saved rotation — must differ, proving the
    // migration is what produces the match.
    const auto rawV16 = serializeSceneAsVersion(authored, kBjorklundGeneratorStateVersion);
    const SceneState unmigrated = deserializeScene(rawV16);
    const auto unmigratedSmf = renderPatternToSMF(unmigrated, 4, 120.0);
    EXPECT_NE(unmigratedSmf, referenceSmf) << "without migration the new generator must NOT match the legacy pattern";
}

// --- Robust sweep: every migrated lane reproduces the pre-switch pattern ------

// Exhaustively pins the migration's core guarantee at the pattern level for
// every (k, n, rotation) a lane can carry: after loading a v15 fixture, the new
// generator at the migrated rotation reproduces the exact legacy pattern the
// retired engine drew, byte-for-byte. Independent reference (inline Bresenham).
TEST(StateMigration, EveryLaneMigratesToPreSwitchPattern) {
    for (int n = 2; n <= 16; ++n) {
        for (int k = 1; k < n; ++k) {
            for (int r0 = 0; r0 < n; ++r0) {
                const SceneState authored = makeSingleEuclideanLane(k, n, r0);
                const auto fixture = serializeSceneAsVersion(authored, kBjorklundGeneratorStateVersion - 1);
                const SceneState migrated = deserializeScene(fixture);

                std::array<bool, kMaxSteps> enginePattern{};
                euclidean(k, n, migrated.sceneA.lanes[0].rotation, enginePattern);
                const std::array<bool, kMaxSteps> legacy = rotateRight(legacyBresenham(k, n), n, r0);

                for (int s = 0; s < n; ++s) {
                    ASSERT_EQ(enginePattern[static_cast<size_t>(s)], legacy[static_cast<size_t>(s)])
                        << "mismatch at k=" << k << " n=" << n << " r0=" << r0 << " step=" << s;
                }
            }
        }
    }
}

// --- Negative / boundary surface (Q7) -----------------------------------------

// Timeline lanes carry an explicit fixedPattern that is immune to the generator
// swap, so the migration must leave their rotation untouched even in a v15 state.
TEST(StateMigration, TimelineLaneRotationNotMigrated) {
    SceneState authored = makeSingleEuclideanLane(/*k=*/5, /*n=*/8, /*rotation=*/1);
    LaneConfig& lane = authored.sceneA.lanes[0];
    lane.timeline = true;
    lane.fixedPatternLength = 8;
    lane.fixedPattern = {true, false, true, false, false, true, false, true};

    const auto fixture = serializeSceneAsVersion(authored, kBjorklundGeneratorStateVersion - 1);
    const SceneState migrated = deserializeScene(fixture);

    EXPECT_EQ(migrated.sceneA.lanes[0].rotation, 1) << "timeline lane rotation must not be migrated";
}

// A current-version (v16) state carries a rotation already authored against
// Bjorklund; the version branch must NOT re-apply the delta.
TEST(StateMigration, CurrentVersionStateNotMigrated) {
    const SceneState authored = makeSingleEuclideanLane(/*k=*/5, /*n=*/8, /*rotation=*/1);
    const auto blob = serializeSceneAsVersion(authored, kBjorklundGeneratorStateVersion);
    const SceneState loaded = deserializeScene(blob);

    EXPECT_EQ(loaded.sceneA.lanes[0].rotation, 1) << "v16 state must load its rotation verbatim";
}

// A saturated lane (k >= n) fires every step regardless of rotation, so the
// migration delta is 0 and the rotation is left unchanged.
TEST(StateMigration, SaturatedLaneRotationUnchanged) {
    const int n = 6, r0 = 3;
    ASSERT_EQ(euclideanMigrationDelta(/*k=*/n, n), 0);
    const SceneState authored = makeSingleEuclideanLane(/*k=*/n, n, r0);
    const auto fixture = serializeSceneAsVersion(authored, kBjorklundGeneratorStateVersion - 1);
    const SceneState migrated = deserializeScene(fixture);

    EXPECT_EQ(migrated.sceneA.lanes[0].rotation, r0) << "saturated lane rotation must be delta-0 (unchanged)";
}
