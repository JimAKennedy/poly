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
    cfg.accents.steps[0] = true;

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
    cfg.accents.steps[0] = true;
    cfg.accents.steps[2] = true;

    auto events = renderLane(cfg);
    ASSERT_GE(events.size(), 4u);

    float baseVel = 100.0f / 127.0f;
    EXPECT_GT(events[0].velocity, baseVel + 0.1f);
    EXPECT_NEAR(events[1].velocity, baseVel, 0.01f);
    EXPECT_GT(events[2].velocity, baseVel + 0.1f);
    EXPECT_NEAR(events[3].velocity, baseVel, 0.01f);
}

// --- Emphasis Probability ---

TEST(DynamicShaping, EmphasisZeroSuppressesAccent) {
    auto cfg = makeBasicLane();
    for (int i = 0; i < 4; ++i)
        cfg.accents.steps[i] = true;
    cfg.emphasisProb = 0.0f;

    auto events = renderLane(cfg);
    float baseVel = 100.0f / 127.0f;
    for (const auto& e : events) {
        EXPECT_NEAR(e.velocity, baseVel, 0.01f);
    }
}

TEST(DynamicShaping, EmphasisOneAlwaysExpresses) {
    auto cfg = makeBasicLane();
    for (int i = 0; i < 4; ++i)
        cfg.accents.steps[i] = true;
    cfg.emphasisProb = 1.0f;

    auto events = renderLane(cfg);
    float baseVel = 100.0f / 127.0f;
    for (const auto& e : events) {
        EXPECT_GT(e.velocity, baseVel + 0.1f);
    }
}

TEST(DynamicShaping, EmphasisPartialMix) {
    auto cfg = makeBasicLane();
    cfg.cycle = {.steps = 16, .subdivision = 16};
    cfg.hitCount = 16;
    for (int i = 0; i < 16; ++i)
        cfg.accents.steps[i] = true;
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
    int accented = 0, plain = 0;
    for (size_t i = 0; i < buf.count; ++i) {
        if (buf.events[i].velocity > baseVel + 0.1f)
            accented++;
        else
            plain++;
    }
    EXPECT_GT(accented, 0) << "Expected some accented notes";
    EXPECT_GT(plain, 0) << "Expected some unaccented notes";
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

// --- Combined Pipeline ---

TEST(DynamicShaping, AccentPlusFloor) {
    auto cfg = makeBasicLane();
    cfg.baseVelocity = 20;
    cfg.ghostFloor = 30;
    cfg.accents.steps[0] = true;
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
    cfg.accents.steps[0] = true;
    cfg.emphasisProb = 1.0f;

    auto events = renderLane(cfg);
    for (const auto& e : events) {
        EXPECT_LE(e.velocity, 1.0f);
        EXPECT_GE(e.velocity, 0.0f);
    }
}

TEST(DynamicShaping, Deterministic) {
    auto cfg = makeBasicLane();
    cfg.accents.steps[0] = true;
    cfg.accents.steps[2] = true;
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
