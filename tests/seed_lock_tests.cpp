// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
// M034 S03 T01: Per-lane seed lock tests.
//
// A locked lane (LaneConfig::seedLocked=true) derives every deterministicRand
// roll from its preserved LaneConfig::laneSeed via laneEffectiveSeed(), so its
// emitted pattern is byte-identical before and after a global reroll
// (GrooveState::seed change) while other, unlocked lanes re-roll. With the
// defaults (seedLocked=false, laneSeed=0) laneEffectiveSeed() returns the
// global seed unchanged, so engine output is byte-identical to the pre-lock
// RNG path (determinism stays green).

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

#include "poly/engine.h"
#include "poly/types.h"

namespace {

// A single 8-step eighth-note lane in 4/4 tuned to be RNG-sensitive:
// velocitySpread jitters per-step velocity from the seed, and mutationRate
// adds/drops steps from the seed, so a global reroll visibly changes output.
poly::LaneConfig makeSeedSensitiveLane(int id) {
    poly::LaneConfig lane{};
    lane.id = id;
    lane.active = true;
    lane.cycle.steps = 8;
    lane.cycle.subdivision = 8;
    lane.hitCount = 4;
    lane.rotation = 0;
    lane.probability = 1.0f;
    lane.baseVelocity = 100;
    lane.midiNote = static_cast<int16_t>(36 + id);
    lane.midiChannel = static_cast<int16_t>(id);
    lane.velocitySpread = 0.3f;
    lane.mutationRate = 0.4f;
    return lane;
}

// Two RNG-sensitive lanes; lane 0 is a lock candidate, lane 1 always re-rolls.
poly::GrooveState makeTwoLaneState(uint64_t seed) {
    poly::GrooveState state{};
    state.activeLaneCount = 2;
    state.seed = seed;
    state.globalDensityCeiling = 0;
    state.lanes[0] = makeSeedSensitiveLane(0);
    state.lanes[1] = makeSeedSensitiveLane(1);
    return state;
}

// Render one 4/4 bar and return the NoteEvents belonging to laneIndex.
std::vector<poly::NoteEvent> renderLaneNotes(const poly::GrooveState& state, int laneIndex, int barIndex = 0) {
    poly::Engine engine;
    poly::NoteEventBuffer notes;

    poly::TransportContext tc{};
    tc.ppqStart = barIndex * 4.0;
    tc.ppqEnd = tc.ppqStart + 4.0;
    tc.tempo = 120.0;
    tc.playing = true;

    engine.renderRange(tc, state, notes, nullptr);

    std::vector<poly::NoteEvent> out;
    for (size_t i = 0; i < notes.count; ++i) {
        if (notes.events[i].laneIndex == laneIndex)
            out.push_back(notes.events[i]);
    }
    return out;
}

// Byte-identical comparison of the audible NoteEvent payload.
bool sameNotes(const std::vector<poly::NoteEvent>& a, const std::vector<poly::NoteEvent>& b) {
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i].ppqPosition != b[i].ppqPosition || a[i].pitch != b[i].pitch || a[i].velocity != b[i].velocity ||
            a[i].duration != b[i].duration || a[i].channel != b[i].channel || a[i].laneIndex != b[i].laneIndex)
            return false;
    }
    return true;
}

constexpr uint64_t kSeedA = 0x1234ABCDULL;
constexpr uint64_t kSeedB = 0xFEED9876ULL;

// laneEffectiveSeed contract: defaults pass the global seed through; a locked
// lane substitutes its preserved laneSeed regardless of the global seed.
TEST(SeedLockTest, EffectiveSeedHelperContract) {
    poly::LaneConfig lane = makeSeedSensitiveLane(0);
    // Defaults (unlocked) → global seed unchanged.
    EXPECT_EQ(poly::laneEffectiveSeed(lane, kSeedA), kSeedA);
    EXPECT_EQ(poly::laneEffectiveSeed(lane, kSeedB), kSeedB);

    lane.seedLocked = true;
    lane.laneSeed = kSeedA;
    // Locked → preserved laneSeed, ignoring the global reroll.
    EXPECT_EQ(poly::laneEffectiveSeed(lane, kSeedA), kSeedA);
    EXPECT_EQ(poly::laneEffectiveSeed(lane, kSeedB), kSeedA);
}

// A locked lane is byte-identical across a global reroll; an unlocked lane
// changes. This is the slice's headline behavior.
TEST(SeedLockTest, LockedLaneInvariantUnderGlobalReroll) {
    poly::GrooveState state = makeTwoLaneState(kSeedA);
    // Lock lane 0 onto the current global seed; lane 1 stays unlocked.
    state.lanes[0].seedLocked = true;
    state.lanes[0].laneSeed = kSeedA;

    std::vector<poly::NoteEvent> lane0Before = renderLaneNotes(state, 0);
    std::vector<poly::NoteEvent> lane1Before = renderLaneNotes(state, 1);
    ASSERT_FALSE(lane0Before.empty());
    ASSERT_FALSE(lane1Before.empty());

    // Global reroll.
    state.seed = kSeedB;
    std::vector<poly::NoteEvent> lane0After = renderLaneNotes(state, 0);
    std::vector<poly::NoteEvent> lane1After = renderLaneNotes(state, 1);

    // Locked lane: byte-identical output despite the reroll.
    EXPECT_TRUE(sameNotes(lane0Before, lane0After));
    // Unlocked lane: output changed (guards that the setup is seed-sensitive).
    EXPECT_FALSE(sameNotes(lane1Before, lane1After));
}

// Locking a lane onto the CURRENT global seed leaves its output unchanged at
// the moment of locking (laneEffectiveSeed returns the same value it did while
// unlocked). Only a subsequent global reroll makes the lock observable.
TEST(SeedLockTest, LockingOntoCurrentSeedIsNoOp) {
    poly::GrooveState unlocked = makeTwoLaneState(kSeedA);
    poly::GrooveState locked = makeTwoLaneState(kSeedA);
    locked.lanes[0].seedLocked = true;
    locked.lanes[0].laneSeed = kSeedA;

    EXPECT_TRUE(sameNotes(renderLaneNotes(unlocked, 0), renderLaneNotes(locked, 0)));
}

// A locked lane derived from seed B matches an unlocked lane whose global seed
// is B — the lock is a pure seed substitution, nothing more.
TEST(SeedLockTest, LockedSeedMatchesEquivalentGlobalSeed) {
    // Reference: unlocked lane rendered with global seed B.
    poly::GrooveState reference = makeTwoLaneState(kSeedB);
    std::vector<poly::NoteEvent> refLane0 = renderLaneNotes(reference, 0);

    // Locked lane onto seed B while the global seed is A.
    poly::GrooveState pinned = makeTwoLaneState(kSeedA);
    pinned.lanes[0].seedLocked = true;
    pinned.lanes[0].laneSeed = kSeedB;
    std::vector<poly::NoteEvent> pinnedLane0 = renderLaneNotes(pinned, 0);

    EXPECT_TRUE(sameNotes(refLane0, pinnedLane0));
}

// Default LaneConfig (seedLocked=false, laneSeed=0) yields deterministic,
// repeatable output driven purely by the global seed — the pre-change path.
TEST(SeedLockTest, DefaultsAreDeterministicOnGlobalSeed) {
    poly::GrooveState state = makeTwoLaneState(kSeedA);
    ASSERT_FALSE(state.lanes[0].seedLocked);
    ASSERT_EQ(state.lanes[0].laneSeed, 0u);

    // Same inputs → byte-identical output.
    EXPECT_TRUE(sameNotes(renderLaneNotes(state, 0), renderLaneNotes(state, 0)));

    // Global seed change → output changes (defaults track the global seed).
    std::vector<poly::NoteEvent> before = renderLaneNotes(state, 0);
    state.seed = kSeedB;
    EXPECT_FALSE(sameNotes(before, renderLaneNotes(state, 0)));
}

} // namespace
