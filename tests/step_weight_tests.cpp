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

// --- Render-level: the rolls read the weights ---

#include <algorithm>
#include <cstring>
#include <vector>

#include "poly/engine.h"
#include "poly/euclidean.h"
#include "poly/presets.h"

namespace {

// Lane 0 is the timeline, striking every fourth step. Lane 1 mutates heavily
// and weights against lane 0, so the distribution of added steps is
// observable over many seeds.
GrooveState timelinePair(float strength, float mutationRate = 0.5f) {
    GrooveState state{};
    state.activeLaneCount = 2;

    auto& timeline = state.lanes[0];
    timeline.id = 0;
    timeline.cycle = {16, 16};
    timeline.hitCount = 4;
    timeline.probability = 1.0f;
    timeline.baseVelocity = 100;

    auto& voice = state.lanes[1];
    voice.id = 1;
    voice.cycle = {16, 16};
    voice.hitCount = 4;
    voice.rotation = 1;
    voice.probability = 1.0f;
    voice.baseVelocity = 90;
    voice.mutationRate = mutationRate;
    voice.timelineSourceLane = 0;
    voice.timelineStrength = strength;
    return state;
}

// Which steps of lane 0 carry onsets, for classifying lane 1's output.
std::array<bool, kMaxSteps> timelineSteps(const GrooveState& state) {
    std::array<bool, kMaxSteps> p{};
    const auto& src = state.lanes[0];
    euclidean(src.hitCount, src.cycle.steps, src.rotation, p);
    return p;
}

// Count lane-1 onsets landing on timeline steps versus off them, across many
// seeds. One roll proves nothing about a probability; a distribution does.
struct Tally {
    int aligned = 0;
    int off = 0;
};

Tally tallyAcrossSeeds(float strength, float mutationRate = 0.5f) {
    Tally t{};
    const auto onTimeline = timelineSteps(timelinePair(strength, mutationRate));
    for (uint64_t seed = 1; seed <= 400; ++seed) {
        GrooveState state = timelinePair(strength, mutationRate);
        state.seed = seed;

        Engine engine;
        NoteEventBuffer notes;
        TransportContext tc{};
        tc.ppqStart = 0.0;
        tc.ppqEnd = 4.0;
        tc.tempo = 120.0;
        tc.playing = true;
        engine.renderRange(tc, state, notes, nullptr);

        for (size_t i = 0; i < notes.count; ++i) {
            if (notes.events[i].laneIndex != 1)
                continue;
            const int step = static_cast<int>(std::lround(notes.events[i].ppqPosition / 0.25)) % 16;
            if (onTimeline[static_cast<size_t>(step)])
                ++t.aligned;
            else
                ++t.off;
        }
    }
    return t;
}

} // namespace

// Attraction: with a positive strength, lane 1's onsets favour the timeline's
// steps more than they do with no weighting at all.
TEST(StepWeightsRender, PositiveStrengthShiftsOnsetsTowardTheTimeline) {
    const Tally neutral = tallyAcrossSeeds(0.0f);
    const Tally attracted = tallyAcrossSeeds(0.9f);

    const double neutralRatio = static_cast<double>(neutral.aligned) / (neutral.aligned + neutral.off);
    const double attractedRatio = static_cast<double>(attracted.aligned) / (attracted.aligned + attracted.off);

    EXPECT_GT(attractedRatio, neutralRatio)
        << "aligned share: neutral " << neutralRatio << ", attracted " << attractedRatio;
}

// Avoidance: the same comparison must invert with the sign. This is the arm
// that proves the sign reaches the rolls, not just the helper.
TEST(StepWeightsRender, NegativeStrengthShiftsOnsetsAwayFromTheTimeline) {
    const Tally neutral = tallyAcrossSeeds(0.0f);
    const Tally avoided = tallyAcrossSeeds(-0.9f);

    const double neutralRatio = static_cast<double>(neutral.aligned) / (neutral.aligned + neutral.off);
    const double avoidedRatio = static_cast<double>(avoided.aligned) / (avoided.aligned + avoided.off);

    EXPECT_LT(avoidedRatio, neutralRatio) << "aligned share: neutral " << neutralRatio << ", avoided " << avoidedRatio;
}

// A lane naming no timeline must render exactly as it did before M002.
TEST(StepWeightsRender, AnUnweightedLaneIsUnchanged) {
    const Tally a = tallyAcrossSeeds(0.0f);
    GrooveState none = timelinePair(0.9f);
    none.lanes[1].timelineSourceLane = -1;

    // Same tally, computed with the reference removed rather than the strength
    // zeroed: both routes must reach the pre-M002 path.
    Tally b{};
    const auto onTimeline = timelineSteps(none);
    for (uint64_t seed = 1; seed <= 400; ++seed) {
        GrooveState state = none;
        state.seed = seed;
        Engine engine;
        NoteEventBuffer notes;
        TransportContext tc{};
        tc.ppqStart = 0.0;
        tc.ppqEnd = 4.0;
        tc.tempo = 120.0;
        tc.playing = true;
        engine.renderRange(tc, state, notes, nullptr);
        for (size_t i = 0; i < notes.count; ++i) {
            if (notes.events[i].laneIndex != 1)
                continue;
            const int step = static_cast<int>(std::lround(notes.events[i].ppqPosition / 0.25)) % 16;
            if (onTimeline[static_cast<size_t>(step)])
                ++b.aligned;
            else
                ++b.off;
        }
    }
    EXPECT_EQ(a.aligned, b.aligned);
    EXPECT_EQ(a.off, b.off);
}

// The milestone's back-compatibility rule for this slice's fields.
TEST(StepWeightsRender, EveryFactoryPresetIsUnmovedWithNoTimelineSet) {
    for (int i = 0; i < poly::kFactoryPresetCount; ++i) {
        const poly::GrooveState preset = poly::makeFactoryPreset(i);
        const char* name = poly::getFactoryPresetInfo(i).name;
        for (int lane = 0; lane < preset.activeLaneCount; ++lane) {
            const auto& cfg = preset.lanes[static_cast<size_t>(lane)];
            ASSERT_EQ(cfg.timelineSourceLane, -1)
                << "preset " << i << " (" << name << ") lane " << lane << " ships a timeline reference";
            ASSERT_FLOAT_EQ(cfg.timelineStrength, 0.0f) << "preset " << i << " (" << name << ")";
        }
    }
}
