// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
//
// M002/S01 (GP03). Position-weighted stochastic decisions.
//
// Four decisions in engine.cpp are position-blind: whether to mutate a step,
// which kind of mutation, whether to fill, and whether to activate. Every step
// is equally likely to be mutated, ghosted or filled. 03-afro-cuban teaches
// that clave is a matrix and that parts ignoring it "sound wrong", and nothing
// in the engine knows the timeline lane exists.
//
// The mechanism: a weight scales a probability, and 1.0 is a no-op. The
// byte-identity guarantee falls out of that rather than being asserted
// separately.
#include <array>

#include <gtest/gtest.h>

#include "poly/types.h"

using namespace poly;

namespace {

// A source pattern striking every fourth step, so aligned and contradicting
// steps are both plentiful and unambiguous.
std::array<bool, kMaxSteps> quarterPattern() {
    std::array<bool, kMaxSteps> p{};
    for (int i = 0; i < kMaxSteps; i += 4)
        p[static_cast<size_t>(i)] = true;
    return p;
}

LaneConfig weightedLane(int sourceLane, float strength) {
    LaneConfig cfg{};
    cfg.id = 1;
    cfg.cycle = {16, 16};
    cfg.timelineSourceLane = sourceLane;
    cfg.timelineStrength = strength;
    return cfg;
}

} // namespace

// The byte-identity guarantee, stated as arithmetic rather than as a promise.
TEST(StepWeights, AreNeutralWithNoReferenceLane) {
    const LaneConfig cfg = weightedLane(-1, 0.8f);
    const auto pattern = quarterPattern();
    for (int step = 0; step < 16; ++step) {
        const StepWeights w = computeStepWeights(cfg, pattern, 16, step);
        EXPECT_FLOAT_EQ(w.add, 1.0f) << "step " << step;
        EXPECT_FLOAT_EQ(w.drop, 1.0f) << "step " << step;
        EXPECT_FLOAT_EQ(w.ghost, 1.0f) << "step " << step;
        EXPECT_FLOAT_EQ(w.fill, 1.0f) << "step " << step;
    }
}

TEST(StepWeights, PositiveStrengthAttractsAddsToTheTimeline) {
    const LaneConfig cfg = weightedLane(0, 0.8f);
    const auto pattern = quarterPattern();
    const StepWeights onTimeline = computeStepWeights(cfg, pattern, 16, 4);
    const StepWeights offTimeline = computeStepWeights(cfg, pattern, 16, 5);

    EXPECT_GT(onTimeline.add, 1.0f) << "a step the timeline strikes must be favoured";
    EXPECT_LT(offTimeline.add, 1.0f) << "a step it does not strike must be disfavoured";
}

// The arm that proves the SIGN is read. Without it, "adds prefer aligned steps"
// would pass for an implementation that only ever reads the magnitude.
TEST(StepWeights, NegativeStrengthInvertsTheRelationship) {
    const LaneConfig cfg = weightedLane(0, -0.8f);
    const auto pattern = quarterPattern();
    const StepWeights onTimeline = computeStepWeights(cfg, pattern, 16, 4);
    const StepWeights offTimeline = computeStepWeights(cfg, pattern, 16, 5);

    EXPECT_LT(onTimeline.add, 1.0f) << "avoidance must disfavour the timeline's own steps";
    EXPECT_GT(offTimeline.add, 1.0f);
}

// Protecting what attraction favours is the row's stated behaviour: a hit on a
// high-weight step should be less likely to be dropped.
TEST(StepWeights, DropMovesOppositeToAdd) {
    const LaneConfig cfg = weightedLane(0, 0.8f);
    const auto pattern = quarterPattern();
    const StepWeights onTimeline = computeStepWeights(cfg, pattern, 16, 4);

    EXPECT_GT(onTimeline.add, 1.0f);
    EXPECT_LT(onTimeline.drop, 1.0f) << "a favoured step must be protected from drops";
}

// A reference that cannot be honoured is refused, not guessed at.
//
// The responsibility is split deliberately. computeStepWeights receives an
// ALREADY-RESOLVED pattern, so it can only refuse what it can see: a negative
// index or a self-reference. Whether lane N exists at all, and whether it
// points back, is the resolver's question -- referenceLaneUsable answers it,
// mirroring the guard kotekan already uses.
TEST(StepWeights, RefusesWhatItCanSee) {
    const auto pattern = quarterPattern();
    for (int bad : {1, -2}) { // self-reference, negative
        LaneConfig cfg = weightedLane(bad, 0.8f);
        const StepWeights w = computeStepWeights(cfg, pattern, 16, 4);
        EXPECT_FLOAT_EQ(w.add, 1.0f) << "reference " << bad << " must yield neutral weights";
    }
}

TEST(ReferenceLane, RefusesOutOfRangeSelfAndMutualReferences) {
    constexpr int kActive = 4;
    // Usable: lane 2 referenced from lane 1, and lane 2 points at nothing.
    EXPECT_TRUE(referenceLaneUsable(2, 1, kActive, -1));

    EXPECT_FALSE(referenceLaneUsable(-1, 1, kActive, -1)) << "negative index";
    EXPECT_FALSE(referenceLaneUsable(kActive, 1, kActive, -1)) << "index at the active count";
    EXPECT_FALSE(referenceLaneUsable(99, 1, kActive, -1)) << "index beyond the active count";
    EXPECT_FALSE(referenceLaneUsable(1, 1, kActive, -1)) << "self-reference";
    // The mutual case: lane 1 names lane 2, and lane 2 names lane 1 back.
    EXPECT_FALSE(referenceLaneUsable(2, 1, kActive, 1)) << "mutual reference must be refused";
}

// A step outside the cycle is not weighted rather than read out of bounds.
TEST(StepWeights, RefusesAStepOutsideTheCycle) {
    const LaneConfig cfg = weightedLane(0, 0.8f);
    const auto pattern = quarterPattern();
    for (int step : {-1, 16, 999})
        EXPECT_FLOAT_EQ(computeStepWeights(cfg, pattern, 16, step).add, 1.0f) << "step " << step;
}
