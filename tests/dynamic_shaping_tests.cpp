// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

#include "poly/engine.h"
#include "poly/types.h"

namespace {

poly::LaneConfig makeBasicLane() {
    poly::LaneConfig cfg{};
    cfg.id = 0;
    cfg.midiNote = 36;
    cfg.cycle = {.steps = 4, .subdivision = 4};
    cfg.hitCount = 4;
    cfg.baseVelocity = 100;
    cfg.probability = 1.0f;
    cfg.velocitySpread = 0.0f;
    cfg.emphasisProb = 1.0f;
    cfg.ghostFloor = 0;
    cfg.active = true;
    return cfg;
}

std::vector<poly::NoteEvent> renderLane(const poly::LaneConfig& cfg, uint64_t seed = 42) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = seed;
    state.lanes[0] = cfg;

    poly::NoteEventBuffer buf;
    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0;
    tc.tempo = 120.0;
    tc.playing = true;

    engine.renderRange(tc, state, buf);

    std::vector<poly::NoteEvent> events;
    for (size_t i = 0; i < buf.count; ++i) {
        events.push_back(buf.events[i]);
    }
    return events;
}

} // namespace

// --- Accent Mask ---

TEST(DynamicShaping, AccentBoostApplied) {
    auto cfg = makeBasicLane();
    cfg.accents.steps[0] = 1.0f;

    auto events = renderLane(cfg);
    ASSERT_GE(events.size(), 4u);

    float baseVel = 100.0f / 127.0f;
    EXPECT_GT(events[0].velocity, baseVel + 0.1f);
    for (size_t i = 1; i < events.size(); ++i) {
        EXPECT_NEAR(events[i].velocity, baseVel, 0.01f);
    }
}

TEST(DynamicShaping, AccentNoMaskNoBoost) {
    auto cfg = makeBasicLane();

    auto events = renderLane(cfg);
    float baseVel = 100.0f / 127.0f;
    for (const auto& e : events) {
        EXPECT_NEAR(e.velocity, baseVel, 0.01f);
    }
}

TEST(DynamicShaping, AccentMultipleSteps) {
    auto cfg = makeBasicLane();
    cfg.accents.steps[0] = 1.0f;
    cfg.accents.steps[2] = 1.0f;

    auto events = renderLane(cfg);
    ASSERT_GE(events.size(), 4u);

    float baseVel = 100.0f / 127.0f;
    EXPECT_GT(events[0].velocity, baseVel + 0.1f);
    EXPECT_NEAR(events[1].velocity, baseVel, 0.01f);
    EXPECT_GT(events[2].velocity, baseVel + 0.1f);
    EXPECT_NEAR(events[3].velocity, baseVel, 0.01f);
}

TEST(DynamicShaping, GraduatedAccentVelocity) {
    auto cfg = makeBasicLane();
    cfg.accents.steps[0] = 1.0f;
    cfg.accents.steps[1] = 0.5f;
    cfg.accents.steps[2] = 0.0f;
    cfg.emphasisProb = 1.0f;

    auto events = renderLane(cfg);
    ASSERT_GE(events.size(), 3u);

    float baseVel = 100.0f / 127.0f;
    float fullBoost = events[0].velocity - baseVel;
    float halfBoost = events[1].velocity - baseVel;
    EXPECT_GT(fullBoost, 0.1f);
    EXPECT_NEAR(halfBoost, fullBoost * 0.5f, 0.01f);
    EXPECT_NEAR(events[2].velocity, baseVel, 0.01f);
}

// M073 S02: a user-toggled explicit accent must apply deterministically (identical
// across seeds because it is decoupled from the emphasisProb roll), strictly boost the
// set step above an unaccented one, and never saturate a mid-velocity lane.
TEST(DynamicShaping, ExplicitAccentDeterministicNoSaturate) {
    auto cfg = makeBasicLane();
    cfg.baseVelocity = 64;       // mid-velocity lane: headroom exists but is limited
    cfg.emphasisProb = 0.0f;     // probabilistic emphasis fully off -> pure explicit accent
    cfg.accents.steps[1] = 1.0f; // accent only step 1; steps 0,2,3 stay unaccented

    float baseVel = 64.0f / 127.0f;
    float accentedVel = -1.0f;
    const uint64_t seeds[] = {1, 7, 42, 1000, 999983};
    for (uint64_t seed : seeds) {
        auto events = renderLane(cfg, seed);
        ASSERT_GE(events.size(), 4u);
        // Step 1 is the accented one; steps 0/2/3 are unaccented references.
        float thisAccented = events[1].velocity;
        EXPECT_GT(thisAccented, events[0].velocity) << "accented step must exceed an unaccented step";
        EXPECT_NEAR(events[0].velocity, baseVel, 0.01f) << "unaccented steps stay at base";
        EXPECT_LT(thisAccented, 1.0f) << "mid-velocity lane must not saturate to 1.0";
        if (accentedVel < 0.0f)
            accentedVel = thisAccented;
        else
            EXPECT_FLOAT_EQ(thisAccented, accentedVel) << "accent boost must be seed-independent";
    }
}

// --- Emphasis Probability ---

// M073 S02: emphasisProb no longer GATES the explicit accent. With emphasisProb = 0
// the probabilistic emphasis layer is fully off, yet an explicitly set accent still
// boosts deterministically — proving the accent is decoupled from the emphasis roll.
TEST(DynamicShaping, EmphasisZeroStillExpressesExplicitAccent) {
    auto cfg = makeBasicLane();
    for (int i = 0; i < 4; ++i)
        cfg.accents.steps[i] = 1.0f;
    cfg.emphasisProb = 0.0f;

    auto events = renderLane(cfg);
    ASSERT_GE(events.size(), 4u);
    float baseVel = 100.0f / 127.0f;
    for (const auto& e : events) {
        EXPECT_GT(e.velocity, baseVel + 0.1f) << "explicit accent must boost even at emphasisProb=0";
        EXPECT_LT(e.velocity, 1.0f) << "proportional-headroom boost must not saturate";
    }
    // Deterministic: emphasis off, so every accented step gets the identical boost.
    for (const auto& e : events) {
        EXPECT_FLOAT_EQ(e.velocity, events[0].velocity);
    }
}

TEST(DynamicShaping, EmphasisOneAlwaysExpresses) {
    auto cfg = makeBasicLane();
    for (int i = 0; i < 4; ++i)
        cfg.accents.steps[i] = 1.0f;
    cfg.emphasisProb = 1.0f;

    auto events = renderLane(cfg);
    float baseVel = 100.0f / 127.0f;
    for (const auto& e : events) {
        EXPECT_GT(e.velocity, baseVel + 0.1f);
    }
}

// M073 S02: with explicit accents set and a partial emphasisProb, every set step is
// deterministically accented (>= the pure-accent floor), and the separate probabilistic
// emphasis layer adds a further nudge to a subset — so the velocities split into at
// least two distinct levels rather than one flat value.
TEST(DynamicShaping, EmphasisPartialAddsSeparateLayer) {
    auto cfg = makeBasicLane();
    cfg.cycle = {.steps = 16, .subdivision = 16};
    cfg.hitCount = 16;
    for (int i = 0; i < 16; ++i)
        cfg.accents.steps[i] = 1.0f;
    cfg.emphasisProb = 0.5f;

    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;
    state.lanes[0] = cfg;

    poly::NoteEventBuffer buf;
    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0;
    tc.tempo = 120.0;
    tc.playing = true;
    engine.renderRange(tc, state, buf);

    float baseVel = 100.0f / 127.0f;
    ASSERT_GT(buf.count, 0u);
    float minVel = buf.events[0].velocity;
    float maxVel = buf.events[0].velocity;
    for (size_t i = 0; i < buf.count; ++i) {
        // Every set step is deterministically accented regardless of the emphasis roll.
        EXPECT_GT(buf.events[i].velocity, baseVel + 0.1f) << "explicit accent must always boost";
        minVel = std::min(minVel, buf.events[i].velocity);
        maxVel = std::max(maxVel, buf.events[i].velocity);
    }
    // The probabilistic emphasis layer still fires on some steps, producing a second,
    // higher velocity level on top of the deterministic accent floor.
    EXPECT_GT(maxVel, minVel + 0.005f) << "probabilistic emphasis must add a separate nudge";
}

// --- Ghost Floor ---

TEST(DynamicShaping, GhostFloorClampsLow) {
    auto cfg = makeBasicLane();
    cfg.baseVelocity = 10;
    cfg.ghostFloor = 40;

    auto events = renderLane(cfg);
    float floor = 40.0f / 127.0f;
    for (const auto& e : events) {
        EXPECT_GE(e.velocity, floor - 0.001f);
    }
}

TEST(DynamicShaping, GhostFloorZeroNoEffect) {
    auto cfg = makeBasicLane();
    cfg.baseVelocity = 10;
    cfg.ghostFloor = 0;

    auto events = renderLane(cfg);
    float baseVel = 10.0f / 127.0f;
    for (const auto& e : events) {
        EXPECT_NEAR(e.velocity, baseVel, 0.01f);
    }
}

TEST(DynamicShaping, GhostFloorNoReduceHigh) {
    auto cfg = makeBasicLane();
    cfg.baseVelocity = 100;
    cfg.ghostFloor = 40;

    auto events = renderLane(cfg);
    float baseVel = 100.0f / 127.0f;
    for (const auto& e : events) {
        EXPECT_NEAR(e.velocity, baseVel, 0.01f);
    }
}

// --- Velocity Zero Mutes Lane (M073 S01) ---

TEST(DynamicShaping, BaseVelocityZeroMutesLane) {
    auto cfg = makeBasicLane();
    cfg.baseVelocity = 0;
    cfg.ghostFloor = 40; // ghost-floor clamp must NOT resurrect the muted lane

    auto events = renderLane(cfg);
    EXPECT_TRUE(events.empty()) << "A lane with baseVelocity 0 must emit zero NoteEvents";
}

TEST(DynamicShaping, ZeroVelocityLaneDoesNotSilenceSibling) {
    poly::LaneConfig muted = makeBasicLane();
    muted.id = 0;
    muted.baseVelocity = 0;
    muted.ghostFloor = 40;

    poly::LaneConfig audible = makeBasicLane();
    audible.id = 1;
    audible.midiNote = 38;
    audible.baseVelocity = 100;

    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 2;
    state.seed = 42;
    state.lanes[0] = muted;
    state.lanes[1] = audible;

    poly::NoteEventBuffer buf;
    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0;
    tc.tempo = 120.0;
    tc.playing = true;
    engine.renderRange(tc, state, buf);

    int mutedCount = 0, audibleCount = 0;
    for (size_t i = 0; i < buf.count; ++i) {
        if (buf.events[i].laneIndex == 0)
            mutedCount++;
        else if (buf.events[i].laneIndex == 1)
            audibleCount++;
    }
    EXPECT_EQ(mutedCount, 0) << "Muted lane (baseVelocity 0) must emit nothing";
    EXPECT_GT(audibleCount, 0) << "Nonzero sibling lane must still emit its notes";
}

// --- Combined Pipeline ---

TEST(DynamicShaping, AccentPlusFloor) {
    auto cfg = makeBasicLane();
    cfg.baseVelocity = 20;
    cfg.ghostFloor = 30;
    cfg.accents.steps[0] = 1.0f;
    cfg.emphasisProb = 1.0f;

    auto events = renderLane(cfg);
    ASSERT_GE(events.size(), 2u);

    float floor = 30.0f / 127.0f;
    EXPECT_GT(events[0].velocity, floor);
    for (size_t i = 1; i < events.size(); ++i) {
        EXPECT_NEAR(events[i].velocity, floor, 0.01f);
    }
}

TEST(DynamicShaping, VelocityClampedToRange) {
    auto cfg = makeBasicLane();
    cfg.baseVelocity = 127;
    cfg.velocitySpread = 0.2f;
    cfg.accents.steps[0] = 1.0f;
    cfg.emphasisProb = 1.0f;

    auto events = renderLane(cfg);
    for (const auto& e : events) {
        EXPECT_LE(e.velocity, 1.0f);
        EXPECT_GE(e.velocity, 0.0f);
    }
}

TEST(DynamicShaping, Deterministic) {
    auto cfg = makeBasicLane();
    cfg.accents.steps[0] = 1.0f;
    cfg.accents.steps[2] = 1.0f;
    cfg.emphasisProb = 0.7f;
    cfg.ghostFloor = 20;
    cfg.velocitySpread = 0.1f;

    auto run1 = renderLane(cfg);
    auto run2 = renderLane(cfg);

    ASSERT_EQ(run1.size(), run2.size());
    for (size_t i = 0; i < run1.size(); ++i) {
        EXPECT_EQ(run1[i].velocity, run2[i].velocity);
    }
}
