#include <gtest/gtest.h>

#include "poly/constraint.h"
#include "poly/engine.h"
#include "poly/euclidean.h"
#include "poly/macro.h"
#include "poly/state_io.h"

namespace {

poly::GrooveState makeBaseState() {
    poly::GrooveState state{};
    state.activeLaneCount = 2;
    state.seed = 42;

    auto& lane0 = state.lanes[0];
    lane0.id = 0;
    lane0.role = poly::Role::AnchorPulse;
    lane0.cycle = {.steps = 8, .subdivision = 8};
    lane0.hitCount = 4;
    lane0.rotation = 0;
    lane0.probability = 1.0f;
    lane0.baseVelocity = 100;
    lane0.emphasisProb = 0.7f;

    auto& lane1 = state.lanes[1];
    lane1.id = 1;
    lane1.role = poly::Role::Backbeat;
    lane1.cycle = {.steps = 8, .subdivision = 8};
    lane1.hitCount = 4;
    lane1.rotation = 0;
    lane1.probability = 0.8f;
    lane1.baseVelocity = 90;
    lane1.emphasisProb = 0.6f;

    return state;
}

poly::TransportContext makeTransport(double ppqStart, double ppqEnd) {
    poly::TransportContext tc{};
    tc.ppqStart = ppqStart;
    tc.ppqEnd = ppqEnd;
    tc.tempo = 120.0;
    tc.sampleRate = 44100.0;
    tc.blockSize = 512;
    tc.playing = true;
    return tc;
}

// --- Anchor Steps ---

TEST(Constraint, AnchorStepsAlwaysFireWithZeroProbability) {
    auto state = makeBaseState();
    auto& lane = state.lanes[0];
    lane.probability = 0.0f;
    lane.constraints.anchorSteps.steps[0] = true;
    lane.constraints.anchorSteps.steps[4] = true;

    poly::Engine engine;
    poly::NoteEventBuffer buf;
    auto tc = makeTransport(0.0, 4.0);
    engine.renderRange(tc, state, buf);

    int lane0Hits = 0;
    for (size_t i = 0; i < buf.count; ++i) {
        if (buf.events[i].pitch == lane.midiNote)
            ++lane0Hits;
    }
    EXPECT_GE(lane0Hits, 2) << "Anchor steps 0 and 4 should fire even with probability=0";
}

TEST(Constraint, NonAnchorStepsRespectProbabilityZero) {
    auto state = makeBaseState();
    state.activeLaneCount = 1;
    auto& lane = state.lanes[0];
    lane.probability = 0.0f;
    lane.hitCount = 8;

    poly::Engine engine;
    poly::NoteEventBuffer buf;
    auto tc = makeTransport(0.0, 4.0);
    engine.renderRange(tc, state, buf);

    EXPECT_EQ(buf.count, 0u) << "No anchors, probability=0 means no notes";
}

TEST(Constraint, AnchorStepsBypassActivationSuppression) {
    auto state = makeBaseState();
    state.activeLaneCount = 1;
    auto& lane = state.lanes[0];
    lane.hitCount = 8;
    lane.probability = 1.0f;
    lane.constraints.anchorSteps.steps[0] = true;

    lane.envelopeCount = 1;
    lane.envelopes[0].active = true;
    lane.envelopes[0].envelope.target = poly::EnvTarget::ActivationWeight;
    lane.envelopes[0].envelope.depth = 1.0f;
    lane.envelopes[0].envelope.periodBars = 1.0f;
    lane.envelopes[0].envelope.shape = poly::Shape::Ramp;
    lane.envelopes[0].envelope.phaseOffset = 0.5f;

    poly::Engine engine;
    poly::NoteEventBuffer buf;
    auto tc = makeTransport(0.0, 4.0);
    engine.renderRange(tc, state, buf);

    bool anchorFired = false;
    for (size_t i = 0; i < buf.count; ++i) {
        double stepPpq = 0.5;
        if (std::abs(buf.events[i].ppqPosition) < stepPpq * 0.1)
            anchorFired = true;
    }
    EXPECT_TRUE(anchorFired) << "Anchor step 0 should fire despite activation suppression";
}

// --- Backbeat Protection ---

TEST(Constraint, BackbeatProtectPreservesEmphasis) {
    auto state = makeBaseState();
    auto& lane = state.lanes[0];
    lane.emphasisProb = 0.8f;
    lane.baseVelocity = 110;
    lane.constraints.backbeatProtect = true;

    state.macros.syncopation = 1.0f;
    state.macros.tension = 1.0f;

    auto macroResolved = poly::resolveMacros(state);
    EXPECT_NE(macroResolved.lanes[0].emphasisProb, 0.8f) << "Macros should have changed emphasisProb";

    auto constrained = poly::resolveConstraints(state, macroResolved);
    EXPECT_FLOAT_EQ(constrained.lanes[0].emphasisProb, 0.8f) << "BackbeatProtect should restore original emphasisProb";
    EXPECT_EQ(constrained.lanes[0].baseVelocity, 110) << "BackbeatProtect should restore original baseVelocity";
}

TEST(Constraint, NoBackbeatProtectAllowsMacroChanges) {
    auto state = makeBaseState();
    auto& lane = state.lanes[0];
    lane.emphasisProb = 0.8f;
    lane.constraints.backbeatProtect = false;

    state.macros.syncopation = 1.0f;

    auto macroResolved = poly::resolveMacros(state);
    auto constrained = poly::resolveConstraints(state, macroResolved);
    EXPECT_FLOAT_EQ(constrained.lanes[0].emphasisProb, macroResolved.lanes[0].emphasisProb)
        << "Without backbeatProtect, emphasis should remain macro-resolved";
}

// --- Density Guardrails ---

TEST(Constraint, DensityMinClampsHitCount) {
    auto state = makeBaseState();
    auto& lane = state.lanes[0];
    lane.hitCount = 4;
    lane.constraints.densityMin = 3;

    state.macros.density = 0.0f;
    auto macroResolved = poly::resolveMacros(state);
    EXPECT_LT(macroResolved.lanes[0].hitCount, 3) << "Density=0 macro should reduce hitCount below 3";

    auto constrained = poly::resolveConstraints(state, macroResolved);
    EXPECT_GE(constrained.lanes[0].hitCount, 3) << "densityMin should clamp hitCount to at least 3";
}

TEST(Constraint, DensityMaxClampsHitCount) {
    auto state = makeBaseState();
    auto& lane = state.lanes[0];
    lane.hitCount = 4;
    lane.constraints.densityMax = 5;

    state.macros.density = 1.0f;
    auto macroResolved = poly::resolveMacros(state);
    EXPECT_GT(macroResolved.lanes[0].hitCount, 5) << "Density=1 macro should push hitCount above 5";

    auto constrained = poly::resolveConstraints(state, macroResolved);
    EXPECT_LE(constrained.lanes[0].hitCount, 5) << "densityMax should clamp hitCount to at most 5";
}

TEST(Constraint, DensityGuardrailsNoop) {
    auto state = makeBaseState();
    auto& lane = state.lanes[0];
    lane.hitCount = 4;
    lane.constraints.densityMin = 0;
    lane.constraints.densityMax = poly::kMaxSteps;

    auto macroResolved = poly::resolveMacros(state);
    auto constrained = poly::resolveConstraints(state, macroResolved);
    EXPECT_EQ(constrained.lanes[0].hitCount, macroResolved.lanes[0].hitCount)
        << "Default density bounds should not alter hitCount";
}

// --- Global Density Ceiling ---

TEST(Constraint, GlobalDensityCeilingReducesTotal) {
    auto state = makeBaseState();
    state.lanes[0].hitCount = 8;
    state.lanes[1].hitCount = 8;
    state.globalDensityCeiling = 10;

    auto constrained = poly::resolveConstraints(state, state);
    int total = constrained.lanes[0].hitCount + constrained.lanes[1].hitCount;
    EXPECT_LE(total, 10) << "Global ceiling should cap total hits";
}

TEST(Constraint, GlobalDensityCeilingRespectsPerLaneMin) {
    auto state = makeBaseState();
    state.lanes[0].hitCount = 8;
    state.lanes[0].constraints.densityMin = 6;
    state.lanes[1].hitCount = 8;
    state.lanes[1].constraints.densityMin = 6;
    state.globalDensityCeiling = 8;

    auto constrained = poly::resolveConstraints(state, state);
    EXPECT_GE(constrained.lanes[0].hitCount, 6);
    EXPECT_GE(constrained.lanes[1].hitCount, 6);
}

TEST(Constraint, GlobalDensityCeilingZeroMeansUnlimited) {
    auto state = makeBaseState();
    state.lanes[0].hitCount = 8;
    state.lanes[1].hitCount = 8;
    state.globalDensityCeiling = 0;

    auto constrained = poly::resolveConstraints(state, state);
    EXPECT_EQ(constrained.lanes[0].hitCount, 8);
    EXPECT_EQ(constrained.lanes[1].hitCount, 8);
}

// --- Serialization Round-trip ---

TEST(Constraint, SerializationRoundTrip) {
    poly::GrooveState original{};
    original.activeLaneCount = 1;
    original.seed = 99;
    original.lanes[0].id = 0;
    original.lanes[0].cycle = {.steps = 8, .subdivision = 8};
    original.lanes[0].hitCount = 4;
    original.lanes[0].constraints.anchorSteps.steps[0] = true;
    original.lanes[0].constraints.anchorSteps.steps[3] = true;
    original.lanes[0].constraints.backbeatProtect = true;
    original.lanes[0].constraints.densityMin = 2;
    original.lanes[0].constraints.densityMax = 6;
    original.globalDensityCeiling = 12;

    std::vector<uint8_t> buffer;
    auto write = [&buffer](const void* data, size_t size) -> bool {
        auto* bytes = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), bytes, bytes + size);
        return true;
    };

    ASSERT_TRUE(poly::writeGrooveState(write, original));

    poly::GrooveState loaded{};
    size_t readPos = 0;
    auto read = [&buffer, &readPos](void* data, size_t size) -> bool {
        if (readPos + size > buffer.size())
            return false;
        std::memcpy(data, buffer.data() + readPos, size);
        readPos += size;
        return true;
    };

    ASSERT_TRUE(poly::readGrooveState(read, loaded));

    EXPECT_TRUE(loaded.lanes[0].constraints.anchorSteps.steps[0]);
    EXPECT_TRUE(loaded.lanes[0].constraints.anchorSteps.steps[3]);
    EXPECT_FALSE(loaded.lanes[0].constraints.anchorSteps.steps[1]);
    EXPECT_TRUE(loaded.lanes[0].constraints.backbeatProtect);
    EXPECT_EQ(loaded.lanes[0].constraints.densityMin, 2);
    EXPECT_EQ(loaded.lanes[0].constraints.densityMax, 6);
    EXPECT_EQ(loaded.globalDensityCeiling, 12);
}

TEST(Constraint, SceneSerializationRoundTrip) {
    poly::SceneState original{};
    original.sceneA.activeLaneCount = 1;
    original.sceneA.lanes[0].constraints.anchorSteps.steps[2] = true;
    original.sceneA.lanes[0].constraints.backbeatProtect = true;
    original.sceneA.lanes[0].constraints.densityMin = 3;
    original.sceneA.lanes[0].constraints.densityMax = 7;
    original.sceneA.globalDensityCeiling = 20;
    original.sceneB.activeLaneCount = 1;
    original.sceneB.lanes[0].constraints.densityMin = 1;
    original.select = poly::SceneSelect::Morph;
    original.morphAmount = 0.5f;

    std::vector<uint8_t> buffer;
    auto write = [&buffer](const void* data, size_t size) -> bool {
        auto* bytes = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), bytes, bytes + size);
        return true;
    };

    ASSERT_TRUE(poly::writeSceneState(write, original));

    poly::SceneState loaded{};
    size_t readPos = 0;
    auto read = [&buffer, &readPos](void* data, size_t size) -> bool {
        if (readPos + size > buffer.size())
            return false;
        std::memcpy(data, buffer.data() + readPos, size);
        readPos += size;
        return true;
    };

    ASSERT_TRUE(poly::readSceneState(read, loaded));

    EXPECT_TRUE(loaded.sceneA.lanes[0].constraints.anchorSteps.steps[2]);
    EXPECT_TRUE(loaded.sceneA.lanes[0].constraints.backbeatProtect);
    EXPECT_EQ(loaded.sceneA.lanes[0].constraints.densityMin, 3);
    EXPECT_EQ(loaded.sceneA.lanes[0].constraints.densityMax, 7);
    EXPECT_EQ(loaded.sceneA.globalDensityCeiling, 20);
    EXPECT_EQ(loaded.sceneB.lanes[0].constraints.densityMin, 1);
    EXPECT_EQ(loaded.select, poly::SceneSelect::Morph);
    EXPECT_FLOAT_EQ(loaded.morphAmount, 0.5f);
}

} // namespace
