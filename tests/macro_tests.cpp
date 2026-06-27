#include <cmath>

#include <gtest/gtest.h>

#include "poly/macro.h"
#include "poly/types.h"

namespace {

poly::GrooveState makeBaseState() {
    poly::GrooveState state{};
    state.activeLaneCount = 2;
    state.seed = 42;

    auto& lane0 = state.lanes[0];
    lane0.id = 0;
    lane0.cycle = {.steps = 8, .subdivision = 8};
    lane0.hitCount = 4;
    lane0.rotation = 1;
    lane0.probability = 0.8f;
    lane0.baseVelocity = 100;
    lane0.emphasisProb = 0.5f;
    lane0.velocitySpread = 0.1f;

    auto& lane1 = state.lanes[1];
    lane1.id = 1;
    lane1.cycle = {.steps = 5, .subdivision = 16};
    lane1.hitCount = 3;
    lane1.rotation = 0;
    lane1.probability = 0.9f;
    lane1.baseVelocity = 80;
    lane1.emphasisProb = 0.6f;
    lane1.velocitySpread = 0.05f;

    return state;
}

// --- Complexity ---

TEST(Macro, ComplexityZeroReducesHitsAndRotation) {
    auto state = makeBaseState();
    state.macros.complexity = 0.0f;
    auto resolved = poly::resolveMacros(state);

    EXPECT_EQ(resolved.lanes[0].hitCount, 1);
    EXPECT_EQ(resolved.lanes[0].rotation, 0);
    EXPECT_EQ(resolved.lanes[1].hitCount, 1);
}

TEST(Macro, ComplexityMidPassthrough) {
    auto state = makeBaseState();
    state.macros.complexity = 0.5f;
    auto resolved = poly::resolveMacros(state);

    EXPECT_EQ(resolved.lanes[0].hitCount, state.lanes[0].hitCount);
    EXPECT_EQ(resolved.lanes[0].rotation, state.lanes[0].rotation);
}

TEST(Macro, ComplexityOneIncreasesHitsAndRotation) {
    auto state = makeBaseState();
    state.macros.complexity = 1.0f;
    auto resolved = poly::resolveMacros(state);

    EXPECT_EQ(resolved.lanes[0].hitCount, 8);
    EXPECT_GT(resolved.lanes[0].rotation, state.lanes[0].rotation);
}

TEST(Macro, ComplexityScalesEnvelopeDepth) {
    auto state = makeBaseState();
    state.lanes[0].envelopeCount = 1;
    state.lanes[0].envelopes[0].active = true;
    state.lanes[0].envelopes[0].envelope.depth = 0.4f;

    state.macros.complexity = 0.0f;
    auto low = poly::resolveMacros(state);

    state.macros.complexity = 1.0f;
    auto high = poly::resolveMacros(state);

    EXPECT_LT(low.lanes[0].envelopes[0].envelope.depth, high.lanes[0].envelopes[0].envelope.depth);
}

// --- Density ---

TEST(Macro, DensityZeroReducesProbAndHits) {
    auto state = makeBaseState();
    state.macros.density = 0.0f;
    auto resolved = poly::resolveMacros(state);

    EXPECT_LT(resolved.lanes[0].probability, state.lanes[0].probability);
    EXPECT_LE(resolved.lanes[0].hitCount, state.lanes[0].hitCount);
}

TEST(Macro, DensityMidPassthrough) {
    auto state = makeBaseState();
    state.macros.density = 0.5f;
    auto resolved = poly::resolveMacros(state);

    EXPECT_FLOAT_EQ(resolved.lanes[0].probability, state.lanes[0].probability);
}

TEST(Macro, DensityOneMaximizes) {
    auto state = makeBaseState();
    state.macros.density = 1.0f;
    auto resolved = poly::resolveMacros(state);

    EXPECT_FLOAT_EQ(resolved.lanes[0].probability, 1.0f);
    EXPECT_EQ(resolved.lanes[0].hitCount, 8);
}

// --- Syncopation ---

TEST(Macro, SyncopationZeroNoRotationChange) {
    auto state = makeBaseState();
    state.macros.syncopation = 0.0f;
    state.macros.tension = 0.5f;
    auto resolved = poly::resolveMacros(state);

    EXPECT_EQ(resolved.lanes[0].rotation, state.lanes[0].rotation);
}

TEST(Macro, SyncopationOneShiftsRotation) {
    auto state = makeBaseState();
    state.macros.syncopation = 1.0f;
    state.macros.tension = 0.5f;
    auto resolved = poly::resolveMacros(state);

    EXPECT_NE(resolved.lanes[0].rotation, state.lanes[0].rotation);
}

TEST(Macro, SyncopationOneInvertsEmphasis) {
    auto state = makeBaseState();
    state.lanes[0].emphasisProb = 0.2f;
    state.macros.syncopation = 1.0f;
    state.macros.tension = 0.5f;
    auto resolved = poly::resolveMacros(state);

    // Syncopation inverts 0.2 → 0.8, then tension=0.5 blends toward 0.5
    // lerp(0.8, 0.5, 0.5) = 0.65
    EXPECT_NEAR(resolved.lanes[0].emphasisProb, 0.65f, 0.01f);
}

TEST(Macro, SyncopationSetsSyncopationOffset) {
    auto state = makeBaseState();
    state.macros.syncopation = 0.0f;
    auto zero = poly::resolveMacros(state);
    EXPECT_FLOAT_EQ(zero.lanes[0].syncopationOffset, 0.0f);

    state.macros.syncopation = 0.7f;
    auto mid = poly::resolveMacros(state);
    EXPECT_FLOAT_EQ(mid.lanes[0].syncopationOffset, 0.7f);

    state.macros.syncopation = 1.0f;
    auto full = poly::resolveMacros(state);
    EXPECT_FLOAT_EQ(full.lanes[0].syncopationOffset, 1.0f);
}

// --- Swing ---

TEST(Macro, SwingAddsToBase) {
    auto state = makeBaseState();
    state.lanes[0].swingAmount = 0.2f;
    state.macros.swing = 0.3f;
    auto resolved = poly::resolveMacros(state);

    EXPECT_FLOAT_EQ(resolved.lanes[0].swingAmount, 0.5f);
}

TEST(Macro, SwingClamps) {
    auto state = makeBaseState();
    state.lanes[0].swingAmount = 0.8f;
    state.macros.swing = 0.5f;
    auto resolved = poly::resolveMacros(state);

    EXPECT_FLOAT_EQ(resolved.lanes[0].swingAmount, 1.0f);
}

// --- Tension ---

TEST(Macro, TensionZeroNarrowsSpread) {
    auto state = makeBaseState();
    state.macros.tension = 0.0f;
    auto resolved = poly::resolveMacros(state);

    EXPECT_LT(resolved.lanes[0].velocitySpread, state.lanes[0].velocitySpread);
}

TEST(Macro, TensionOneWidensSpread) {
    auto state = makeBaseState();
    state.macros.tension = 1.0f;
    auto resolved = poly::resolveMacros(state);

    EXPECT_GT(resolved.lanes[0].velocitySpread, state.lanes[0].velocitySpread);
}

// --- Humanize ---

TEST(Macro, HumanizeZeroNoJitter) {
    auto state = makeBaseState();
    state.lanes[0].humanizeMs = 0.0f;
    state.macros.humanize = 0.0f;
    auto resolved = poly::resolveMacros(state);

    EXPECT_FLOAT_EQ(resolved.lanes[0].humanizeMs, 0.0f);
}

TEST(Macro, HumanizeOneAddsJitter) {
    auto state = makeBaseState();
    state.lanes[0].humanizeMs = 0.0f;
    state.macros.tension = 0.5f;
    state.macros.humanize = 1.0f;
    auto resolved = poly::resolveMacros(state);

    EXPECT_FLOAT_EQ(resolved.lanes[0].humanizeMs, 25.0f);
    EXPECT_GT(resolved.lanes[0].velocitySpread, state.lanes[0].velocitySpread);
}

// --- Composition ---

TEST(Macro, DensityAndComplexityCompose) {
    auto state = makeBaseState();
    state.macros.complexity = 1.0f;
    state.macros.density = 1.0f;
    auto resolved = poly::resolveMacros(state);

    EXPECT_EQ(resolved.lanes[0].hitCount, 8);
    EXPECT_FLOAT_EQ(resolved.lanes[0].probability, 1.0f);
}

TEST(Macro, AllMacrosDefaultPassthrough) {
    auto state = makeBaseState();
    auto resolved = poly::resolveMacros(state);

    EXPECT_EQ(resolved.lanes[0].hitCount, state.lanes[0].hitCount);
    EXPECT_FLOAT_EQ(resolved.lanes[0].probability, state.lanes[0].probability);
    EXPECT_FLOAT_EQ(resolved.lanes[0].swingAmount, state.lanes[0].swingAmount);
    EXPECT_FLOAT_EQ(resolved.lanes[0].humanizeMs, state.lanes[0].humanizeMs);
}

// --- Golden: macro resolution is deterministic ---

TEST(Macro, Deterministic) {
    auto state = makeBaseState();
    state.macros.complexity = 0.7f;
    state.macros.density = 0.3f;
    state.macros.syncopation = 0.4f;
    state.macros.swing = 0.2f;
    state.macros.tension = 0.6f;
    state.macros.humanize = 0.5f;

    auto a = poly::resolveMacros(state);
    auto b = poly::resolveMacros(state);

    for (int i = 0; i < state.activeLaneCount; ++i) {
        EXPECT_EQ(a.lanes[i].hitCount, b.lanes[i].hitCount);
        EXPECT_FLOAT_EQ(a.lanes[i].probability, b.lanes[i].probability);
        EXPECT_FLOAT_EQ(a.lanes[i].rotation, b.lanes[i].rotation);
        EXPECT_FLOAT_EQ(a.lanes[i].swingAmount, b.lanes[i].swingAmount);
        EXPECT_FLOAT_EQ(a.lanes[i].humanizeMs, b.lanes[i].humanizeMs);
        EXPECT_FLOAT_EQ(a.lanes[i].velocitySpread, b.lanes[i].velocitySpread);
        EXPECT_FLOAT_EQ(a.lanes[i].emphasisProb, b.lanes[i].emphasisProb);
    }
}

// --- Timeline immunity ---

TEST(Macro, TimelineLaneImmuneToMacros) {
    auto state = makeBaseState();
    state.lanes[0].timeline = true;
    state.macros.complexity = 1.0f;
    state.macros.density = 1.0f;
    state.macros.syncopation = 1.0f;
    state.macros.swing = 0.5f;
    state.macros.tension = 1.0f;
    state.macros.humanize = 1.0f;

    auto resolved = poly::resolveMacros(state);

    EXPECT_EQ(resolved.lanes[0].hitCount, state.lanes[0].hitCount);
    EXPECT_FLOAT_EQ(resolved.lanes[0].probability, state.lanes[0].probability);
    EXPECT_EQ(resolved.lanes[0].rotation, state.lanes[0].rotation);
    EXPECT_FLOAT_EQ(resolved.lanes[0].swingAmount, state.lanes[0].swingAmount);
    EXPECT_FLOAT_EQ(resolved.lanes[0].humanizeMs, state.lanes[0].humanizeMs);
    EXPECT_FLOAT_EQ(resolved.lanes[0].velocitySpread, state.lanes[0].velocitySpread);
    EXPECT_FLOAT_EQ(resolved.lanes[0].emphasisProb, state.lanes[0].emphasisProb);

    EXPECT_NE(resolved.lanes[1].hitCount, state.lanes[1].hitCount);
}

TEST(Macro, NonTimelineLaneStillAffected) {
    auto state = makeBaseState();
    state.lanes[0].timeline = true;
    state.macros.complexity = 0.0f;

    auto resolved = poly::resolveMacros(state);

    EXPECT_EQ(resolved.lanes[0].hitCount, state.lanes[0].hitCount);
    EXPECT_NE(resolved.lanes[1].hitCount, state.lanes[1].hitCount);
}

// --- MacroSmoother ---

TEST(MacroSmoother, SnapToTargetOnFirstAdvance) {
    poly::MacroSmoother s;
    poly::MacroValues target{};
    target.complexity = 0.8f;
    target.density = 0.3f;
    s.setTarget(target);
    s.advance(44100.0, 512);

    EXPECT_FLOAT_EQ(s.current.complexity, 0.8f);
    EXPECT_FLOAT_EQ(s.current.density, 0.3f);
}

TEST(MacroSmoother, GradualTransition) {
    poly::MacroSmoother s;
    poly::MacroValues initial{};
    initial.complexity = 0.0f;
    s.setTarget(initial);
    s.advance(44100.0, 512);

    poly::MacroValues target{};
    target.complexity = 1.0f;
    s.setTarget(target);

    s.advance(44100.0, 512);
    EXPECT_GT(s.current.complexity, 0.0f);
    EXPECT_LT(s.current.complexity, 1.0f);

    for (int i = 0; i < 500; ++i)
        s.advance(44100.0, 512);
    EXPECT_NEAR(s.current.complexity, 1.0f, 0.001f);
}

TEST(MacroSmoother, SnapToTargetBypasses) {
    poly::MacroSmoother s;
    poly::MacroValues initial{};
    initial.complexity = 0.0f;
    s.setTarget(initial);
    s.advance(44100.0, 512);

    poly::MacroValues target{};
    target.complexity = 1.0f;
    target.density = 0.7f;
    s.setTarget(target);
    s.snapToTarget();

    EXPECT_FLOAT_EQ(s.current.complexity, 1.0f);
    EXPECT_FLOAT_EQ(s.current.density, 0.7f);
}

TEST(MacroSmoother, ReinitAfterReset) {
    poly::MacroSmoother s;
    poly::MacroValues v{};
    v.complexity = 0.5f;
    s.setTarget(v);
    s.advance(44100.0, 512);

    s.initialized = false;
    v.complexity = 0.9f;
    s.setTarget(v);
    s.advance(44100.0, 512);

    EXPECT_FLOAT_EQ(s.current.complexity, 0.9f);
}

TEST(MacroSmoother, AllFieldsSmooth) {
    poly::MacroSmoother s;
    poly::MacroValues initial{};
    s.setTarget(initial);
    s.advance(44100.0, 512);

    poly::MacroValues target{};
    target.complexity = 1.0f;
    target.density = 1.0f;
    target.syncopation = 1.0f;
    target.swing = 1.0f;
    target.tension = 1.0f;
    target.humanize = 1.0f;
    s.setTarget(target);
    s.advance(44100.0, 512);

    EXPECT_GT(s.current.complexity, 0.0f);
    EXPECT_GT(s.current.density, 0.0f);
    EXPECT_GT(s.current.syncopation, 0.0f);
    EXPECT_GT(s.current.swing, 0.0f);
    EXPECT_GT(s.current.tension, 0.0f);
    EXPECT_GT(s.current.humanize, 0.0f);
    EXPECT_LT(s.current.complexity, 1.0f);
}

} // namespace
