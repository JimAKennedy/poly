#include "poly/engine.h"
#include "poly/envelope.h"
#include "poly/types.h"
#include <gtest/gtest.h>

#include <cmath>
#include <vector>

namespace {

constexpr float kEps = 1e-5f;
constexpr double kDEps = 1e-9;

// --- Shape evaluation tests ---

TEST(EnvelopeShape, SineKeyPoints) {
    EXPECT_NEAR(poly::evaluateShape(poly::Shape::Sine, 0.0f), 0.5f, kEps);
    EXPECT_NEAR(poly::evaluateShape(poly::Shape::Sine, 0.25f), 1.0f, kEps);
    EXPECT_NEAR(poly::evaluateShape(poly::Shape::Sine, 0.5f), 0.5f, kEps);
    EXPECT_NEAR(poly::evaluateShape(poly::Shape::Sine, 0.75f), 0.0f, kEps);
}

TEST(EnvelopeShape, RampKeyPoints) {
    EXPECT_NEAR(poly::evaluateShape(poly::Shape::Ramp, 0.0f), 0.0f, kEps);
    EXPECT_NEAR(poly::evaluateShape(poly::Shape::Ramp, 0.25f), 0.25f, kEps);
    EXPECT_NEAR(poly::evaluateShape(poly::Shape::Ramp, 0.5f), 0.5f, kEps);
    EXPECT_NEAR(poly::evaluateShape(poly::Shape::Ramp, 0.75f), 0.75f, kEps);
}

TEST(EnvelopeShape, TriangleKeyPoints) {
    EXPECT_NEAR(poly::evaluateShape(poly::Shape::Triangle, 0.0f), 0.0f, kEps);
    EXPECT_NEAR(poly::evaluateShape(poly::Shape::Triangle, 0.25f), 0.5f, kEps);
    EXPECT_NEAR(poly::evaluateShape(poly::Shape::Triangle, 0.5f), 1.0f, kEps);
    EXPECT_NEAR(poly::evaluateShape(poly::Shape::Triangle, 0.75f), 0.5f, kEps);
}

TEST(EnvelopeShape, PlaceholderShapes) {
    EXPECT_NEAR(poly::evaluateShape(poly::Shape::Curve, 0.0f), 0.5f, kEps);
    EXPECT_NEAR(poly::evaluateShape(poly::Shape::Curve, 0.5f), 0.5f, kEps);
    EXPECT_NEAR(poly::evaluateShape(poly::Shape::StepList, 0.0f), 0.5f, kEps);
    EXPECT_NEAR(poly::evaluateShape(poly::Shape::StepList, 0.75f), 0.5f, kEps);
}

// --- Phase calculation tests ---

TEST(EnvelopePhase, BasicPeriod) {
    // 4-bar period = 16 PPQ. At PPQ 0, phase = 0
    EXPECT_NEAR(poly::computeEnvelopePhase(0.0, 4.0f, 0.0f), 0.0, kDEps);
    // At PPQ 8 (half of 16), phase = 0.5
    EXPECT_NEAR(poly::computeEnvelopePhase(8.0, 4.0f, 0.0f), 0.5, kDEps);
    // At PPQ 16 (full cycle), phase wraps to 0
    EXPECT_NEAR(poly::computeEnvelopePhase(16.0, 4.0f, 0.0f), 0.0, kDEps);
}

TEST(EnvelopePhase, PhaseOffset) {
    // Offset of 0.25 shifts phase forward by quarter cycle
    EXPECT_NEAR(poly::computeEnvelopePhase(0.0, 4.0f, 0.25f), 0.25, kDEps);
    // PPQ 4 is quarter of 16-PPQ period, + 0.25 offset = 0.5
    EXPECT_NEAR(poly::computeEnvelopePhase(4.0, 4.0f, 0.25f), 0.5, kDEps);
}

TEST(EnvelopePhase, NonDividingPeriod) {
    // 3-bar period = 12 PPQ
    EXPECT_NEAR(poly::computeEnvelopePhase(0.0, 3.0f, 0.0f), 0.0, kDEps);
    EXPECT_NEAR(poly::computeEnvelopePhase(6.0, 3.0f, 0.0f), 0.5, kDEps);
    EXPECT_NEAR(poly::computeEnvelopePhase(12.0, 3.0f, 0.0f), 0.0, kDEps);
}

TEST(EnvelopePhase, LargePpqWraps) {
    // At PPQ 100, with 4-bar period (16 PPQ): 100/16 = 6.25, fmod = 0.25
    EXPECT_NEAR(poly::computeEnvelopePhase(100.0, 4.0f, 0.0f), 0.25, kDEps);
}

// --- Velocity modulation integration tests ---

poly::LaneConfig makeEnvelopeLane() {
    poly::LaneConfig cfg{};
    cfg.id = 0;
    cfg.midiNote = 36;
    cfg.cycle = {.steps = 4, .subdivision = 4};
    cfg.hitCount = 4;
    cfg.baseVelocity = 100;
    cfg.probability = 1.0f;
    cfg.velocitySpread = 0.0f;
    cfg.emphasisProb = 0.0f;
    cfg.ghostFloor = 0;
    cfg.active = true;
    return cfg;
}

std::vector<poly::NoteEvent> renderOneLane(const poly::LaneConfig& cfg,
                                            double ppqStart, double ppqEnd,
                                            uint64_t seed = 42,
                                            int globalEnvCount = 0,
                                            const poly::Envelope* globalEnvs = nullptr) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.lanes[0] = cfg;
    state.seed = seed;
    if (globalEnvs) {
        for (int i = 0; i < globalEnvCount; ++i)
            state.globalEnvelopes[i] = globalEnvs[i];
        state.globalEnvelopeCount = globalEnvCount;
    }

    poly::TransportContext tc{};
    tc.ppqStart = ppqStart;
    tc.ppqEnd = ppqEnd;
    tc.tempo = 120.0;
    tc.playing = true;

    poly::NoteEventBuffer buf;
    engine.renderRange(tc, state, buf);

    std::vector<poly::NoteEvent> result;
    for (size_t i = 0; i < buf.count; ++i)
        result.push_back(buf.events[i]);
    return result;
}

TEST(EnvelopeIntegration, NoEnvelopeUnchanged) {
    auto cfg = makeEnvelopeLane();
    auto events = renderOneLane(cfg, 0.0, 4.0);
    ASSERT_EQ(events.size(), 4u);
    float baseVel = 100.0f / 127.0f;
    for (const auto& e : events)
        EXPECT_NEAR(e.velocity, baseVel, kEps);
}

TEST(EnvelopeIntegration, SineVelocityModulation) {
    auto cfg = makeEnvelopeLane();
    cfg.envelopes[0].envelope = {poly::EnvTarget::Velocity, 4.0f,
                                  poly::Shape::Sine, 1.0f, 0.0f};
    cfg.envelopes[0].active = true;
    cfg.envelopeCount = 1;

    // 4-bar period = 16 PPQ. Sine at phase 0 = 0.5
    // velMod = 1 - 1*(1-0.5) = 0.5
    auto events = renderOneLane(cfg, 0.0, 4.0);
    ASSERT_EQ(events.size(), 4u);

    float baseVel = 100.0f / 127.0f;
    // First note at PPQ 0: sine phase=0, value=0.5, velMod=0.5
    EXPECT_NEAR(events[0].velocity, baseVel * 0.5f, kEps);

    // Note at PPQ 1: phase=1/16=0.0625, sine(0.0625*2pi)=sin(pi/8)≈0.383
    // value = 0.5*(1+0.383) = 0.691, velMod = 1-1*(1-0.691) = 0.691
    EXPECT_GT(events[1].velocity, events[0].velocity);
}

TEST(EnvelopeIntegration, ZeroDepthNoEffect) {
    auto cfg = makeEnvelopeLane();
    cfg.envelopes[0].envelope = {poly::EnvTarget::Velocity, 4.0f,
                                  poly::Shape::Sine, 0.0f, 0.0f};
    cfg.envelopes[0].active = true;
    cfg.envelopeCount = 1;

    auto events = renderOneLane(cfg, 0.0, 4.0);
    ASSERT_EQ(events.size(), 4u);
    float baseVel = 100.0f / 127.0f;
    for (const auto& e : events)
        EXPECT_NEAR(e.velocity, baseVel, kEps);
}

TEST(EnvelopeIntegration, InactiveEnvelopeIgnored) {
    auto cfg = makeEnvelopeLane();
    cfg.envelopes[0].envelope = {poly::EnvTarget::Velocity, 4.0f,
                                  poly::Shape::Sine, 1.0f, 0.0f};
    cfg.envelopes[0].active = false;
    cfg.envelopeCount = 1;

    auto events = renderOneLane(cfg, 0.0, 4.0);
    ASSERT_EQ(events.size(), 4u);
    float baseVel = 100.0f / 127.0f;
    for (const auto& e : events)
        EXPECT_NEAR(e.velocity, baseVel, kEps);
}

TEST(EnvelopeIntegration, DensityModulation) {
    auto cfg = makeEnvelopeLane();
    cfg.probability = 0.5f;
    cfg.envelopes[0].envelope = {poly::EnvTarget::Density, 4.0f,
                                  poly::Shape::Ramp, 1.0f, 0.0f};
    cfg.envelopes[0].active = true;
    cfg.envelopeCount = 1;

    // Ramp at PPQ 0: phase=0, value=0, probMod=1*(0*2-1)=-1
    // effectiveProb = 0.5 + (-1) = 0 -> clamped to 0 -> no notes early
    // Ramp at PPQ 3: phase=3/16=0.1875, value=0.1875, probMod=1*(0.375-1)=-0.625
    // effectiveProb = 0.5 + (-0.625) = 0 -> still 0
    // So with ramp starting low, fewer notes at start of cycle
    auto eventsRamp = renderOneLane(cfg, 0.0, 16.0);

    // Compare with no envelope
    cfg.envelopeCount = 0;
    auto eventsNoEnv = renderOneLane(cfg, 0.0, 16.0);

    // Density modulation should change note count
    EXPECT_NE(eventsRamp.size(), eventsNoEnv.size());
}

TEST(EnvelopeIntegration, GlobalEnvelopeApplied) {
    auto cfg = makeEnvelopeLane();

    poly::Envelope globalEnv{poly::EnvTarget::Velocity, 4.0f,
                              poly::Shape::Triangle, 1.0f, 0.0f};

    auto events = renderOneLane(cfg, 0.0, 4.0, 42, 1, &globalEnv);
    ASSERT_EQ(events.size(), 4u);

    // Triangle at phase 0 = 0, velMod = 1-1*(1-0) = 0
    // But velocity is clamped to [0,1], so first note should be ~0
    EXPECT_NEAR(events[0].velocity, 0.0f, kEps);

    // Later notes should have higher velocity as triangle rises
    EXPECT_GT(events[1].velocity, events[0].velocity);
}

TEST(EnvelopeIntegration, MultipleEnvelopesMultiplicative) {
    auto cfg = makeEnvelopeLane();

    // Two velocity envelopes, both sine but different periods
    cfg.envelopes[0].envelope = {poly::EnvTarget::Velocity, 4.0f,
                                  poly::Shape::Sine, 1.0f, 0.0f};
    cfg.envelopes[0].active = true;
    cfg.envelopes[1].envelope = {poly::EnvTarget::Velocity, 8.0f,
                                  poly::Shape::Sine, 1.0f, 0.0f};
    cfg.envelopes[1].active = true;
    cfg.envelopeCount = 2;

    auto eventsDual = renderOneLane(cfg, 0.0, 4.0);
    ASSERT_EQ(eventsDual.size(), 4u);

    // Single envelope for comparison
    cfg.envelopeCount = 1;
    auto eventsSingle = renderOneLane(cfg, 0.0, 4.0);
    ASSERT_EQ(eventsSingle.size(), 4u);

    // Dual should have lower velocity than single (multiplicative)
    for (size_t i = 0; i < 4; ++i)
        EXPECT_LE(eventsDual[i].velocity, eventsSingle[i].velocity + kEps);
}

TEST(EnvelopeIntegration, Deterministic) {
    auto cfg = makeEnvelopeLane();
    cfg.envelopes[0].envelope = {poly::EnvTarget::Velocity, 3.0f,
                                  poly::Shape::Sine, 0.8f, 0.1f};
    cfg.envelopes[0].active = true;
    cfg.envelopeCount = 1;

    auto events1 = renderOneLane(cfg, 0.0, 16.0);
    auto events2 = renderOneLane(cfg, 0.0, 16.0);
    ASSERT_EQ(events1.size(), events2.size());
    for (size_t i = 0; i < events1.size(); ++i) {
        EXPECT_EQ(events1[i].ppqPosition, events2[i].ppqPosition);
        EXPECT_EQ(events1[i].velocity, events2[i].velocity);
    }
}

} // namespace
