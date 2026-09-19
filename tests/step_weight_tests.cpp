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
#include <cmath>

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
#include "poly/macro.h"
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

// --- M002/S02 (GP04): ghosts cluster where funk puts them ---
//
// Ghost notes come from a flat per-step mutation roll: any mutated step has an
// equal chance of becoming a ghost, independent of where it sits in the meter.
// Funk ghosting is grammatical -- ghosts concentrate on the weak subdivisions
// around the backbeat and fill TOWARD the next accent (Danielsen 2006; Stewart
// 2000). Chapter 11 works around this with a dedicated high-hit-count ghost
// lane, which costs a lane and cannot respond to where the accents are.

namespace {

// A lane whose pattern accents steps 4 and 12 -- a backbeat -- so the steps
// before and after an accent are unambiguous.
LaneConfig ghostLane(float grammar) {
    LaneConfig cfg{};
    cfg.id = 1;
    cfg.cycle = {16, 16};
    cfg.ghostGrammar = grammar;
    cfg.accents.steps[4] = 1.0f; // a backbeat, so approach and departure are unambiguous
    cfg.accents.steps[12] = 1.0f;
    return cfg;
}

std::array<bool, kMaxSteps> backbeatPattern() {
    std::array<bool, kMaxSteps> p{};
    p[4] = true;
    p[12] = true;
    return p;
}

} // namespace

TEST(GhostGrammar, IsNeutralWhenUnset) {
    const LaneConfig cfg = ghostLane(0.0f);
    for (int step = 0; step < 16; ++step)
        EXPECT_FLOAT_EQ(computeGhostWeight(cfg, 16, step), 1.0f) << "step " << step;
}

// The gradient, and both halves of it. A weight that merely favoured every weak
// step would satisfy a one-sided test; "fills toward the accent" means the step
// BEFORE an accent is favoured over the one after.
TEST(GhostGrammar, FavoursTheApproachToAnAccentOverTheDeparture) {
    const LaneConfig cfg = ghostLane(1.0f);

    const float before = computeGhostWeight(cfg, 16, 3); // leads into the accent at 4
    const float after = computeGhostWeight(cfg, 16, 5);  // follows it

    EXPECT_GT(before, 1.0f) << "the approach must be favoured";
    EXPECT_LT(after, 1.0f) << "the step after an accent must be quieter";
    EXPECT_GT(before, after);
}

// A step far from any accent is closer to neutral than the approach is: the
// weighting is a gradient toward accents, not a blanket lift on weak steps.
TEST(GhostGrammar, IsAGradientRatherThanABlanketLift) {
    const LaneConfig cfg = ghostLane(1.0f);

    const float approach = computeGhostWeight(cfg, 16, 3);
    const float distant = computeGhostWeight(cfg, 16, 8);
    EXPECT_GT(approach, distant) << "approach " << approach << " vs distant " << distant;
}

// The row states this and it is checkable: low Complexity keeps grooves clean.
TEST(GhostGrammar, ScalesWithTheComplexityMacro) {
    GrooveState low{};
    low.activeLaneCount = 1;
    low.lanes[0] = ghostLane(1.0f);
    low.macros.complexity = 0.0f;

    GrooveState high = low;
    high.macros.complexity = 1.0f;

    const GrooveState resolvedLow = resolveMacros(low);
    const GrooveState resolvedHigh = resolveMacros(high);

    EXPECT_LT(resolvedLow.lanes[0].ghostGrammar, resolvedHigh.lanes[0].ghostGrammar)
        << "low " << resolvedLow.lanes[0].ghostGrammar << ", high " << resolvedHigh.lanes[0].ghostGrammar;
    EXPECT_LT(resolvedLow.lanes[0].ghostGrammar, 1.0f) << "low Complexity must damp the grammar";
}

// Render-level. The four cases above all exercise computeGhostWeight directly,
// so a probe discarding the weight at the composition site killed none of them:
// the weight could be computed perfectly and thrown away. This is the case that
// proves the wiring, measured as a distribution over seeds.
TEST(GhostGrammarRender, GhostsFavourTheApproachToAnAccent) {
    auto tallyGhosts = [](float grammar) {
        int approach = 0;
        int departure = 0;
        for (uint64_t seed = 1; seed <= 400; ++seed) {
            GrooveState state{};
            state.activeLaneCount = 1;
            state.seed = seed;
            auto& lane = state.lanes[0];
            lane.id = 0;
            lane.cycle = {16, 16};
            lane.hitCount = 16; // every step lit, so any step can be ghosted
            lane.probability = 1.0f;
            lane.baseVelocity = 100;
            lane.ghostFloor = 30;
            lane.mutationRate = 0.6f;
            lane.ghostGrammar = grammar;
            lane.accents.steps[4] = 1.0f; // backbeat accents
            lane.accents.steps[12] = 1.0f;

            Engine engine;
            NoteEventBuffer notes;
            EmissionEventBuffer emissions;
            TransportContext tc{};
            tc.ppqStart = 0.0;
            tc.ppqEnd = 4.0;
            tc.tempo = 120.0;
            tc.playing = true;
            engine.renderRange(tc, state, notes, &emissions);

            for (size_t i = 0; i < emissions.count; ++i) {
                const auto& e = emissions.events[i];
                if (e.kind != static_cast<uint8_t>(EmissionKind::Ghost))
                    continue;
                const int step = static_cast<int>(e.cycleStep);
                if (step == 3 || step == 11)
                    ++approach; // leads into an accent
                else if (step == 5 || step == 13)
                    ++departure; // follows one
            }
        }
        return std::pair<int, int>{approach, departure};
    };

    const auto neutral = tallyGhosts(0.0f);
    const auto grammared = tallyGhosts(1.0f);

    const double neutralRatio = static_cast<double>(neutral.first) / std::max(1, neutral.first + neutral.second);
    const double grammarRatio = static_cast<double>(grammared.first) / std::max(1, grammared.first + grammared.second);

    EXPECT_GT(grammarRatio, neutralRatio)
        << "approach share: neutral " << neutralRatio << ", with grammar " << grammarRatio << " (approach "
        << grammared.first << " vs departure " << grammared.second << ")";
}

// --- M002/S03 (GP05): fills resolve onto the phrase boundary ---
//
// FillLikelihood is an envelope target with no knowledge of phrase position: a
// fill-add is as likely at beat 2 of bar 1 as at the end of an 8-bar phrase.
// Idiomatic fills cluster at phrase boundaries and resolve onto the downbeat
// (Nelson 2008; Clayton 2000; Riley 1994).

namespace {

LaneConfig fillLane(float shape, float phraseLength = 4.0f) {
    LaneConfig cfg{};
    cfg.id = 1;
    cfg.cycle = {16, 16};
    cfg.fillPhraseShape = shape;
    cfg.phraseLength = phraseLength;
    cfg.phraseGap = 0.0f;
    return cfg;
}

} // namespace

TEST(FillPhraseWeight, IsNeutralWhenUnset) {
    const LaneConfig cfg = fillLane(0.0f);
    for (double pos = 0.0; pos < 4.0; pos += 0.5)
        EXPECT_FLOAT_EQ(computeFillWeight(cfg, pos, 4.0), 1.0f) << "position " << pos;
}

// Across the cycle, not at two points: a weight that spiked only at the final
// step would satisfy a two-sample test and be wrong everywhere else.
TEST(FillPhraseWeight, RisesMonotonicallyTowardTheBoundary) {
    const LaneConfig cfg = fillLane(1.0f);
    float previous = computeFillWeight(cfg, 0.0, 4.0);
    for (double pos = 0.5; pos <= 3.5; pos += 0.5) {
        const float w = computeFillWeight(cfg, pos, 4.0);
        EXPECT_GT(w, previous) << "weight must rise at position " << pos;
        previous = w;
    }
}

// The shape parameter controls how sharply, compared mid-cycle where the two
// shapes differ most.
TEST(FillPhraseWeight, ShapeControlsHowSharplyFillsConcentrate) {
    const float gentle = computeFillWeight(fillLane(0.3f), 2.0, 4.0);
    const float sharp = computeFillWeight(fillLane(1.0f), 2.0, 4.0);
    EXPECT_LT(gentle, sharp) << "gentle " << gentle << ", sharp " << sharp;
}

// The row says the boundary for an ungated lane is the composite convergence
// point, not silence. A lane with no phrase length must still get a meaningful
// weight rather than a division by zero.
TEST(FillPhraseWeight, HandlesAnUngatedLaneWithoutDividingByZero) {
    LaneConfig cfg = fillLane(1.0f, 0.0f);
    for (double pos : {0.0, 1.0, 3.9}) {
        const float w = computeFillWeight(cfg, pos, 0.0);
        EXPECT_TRUE(std::isfinite(w)) << "position " << pos << " produced " << w;
        EXPECT_FLOAT_EQ(w, 1.0f) << "with no cycle to resolve onto, the weight is neutral";
    }
}

// Render-level. S02 taught that unit cases on the helper cannot see the wiring:
// a weight computed perfectly and discarded leaves them all green. This case
// measures where fills actually land, across seeds.
TEST(FillPhraseWeightRender, FillsConcentrateTowardThePhraseBoundary) {
    auto tallyFills = [](float shape) {
        int early = 0;
        int late = 0;
        for (uint64_t seed = 1; seed <= 300; ++seed) {
            GrooveState state{};
            state.activeLaneCount = 1;
            state.seed = seed;
            auto& lane = state.lanes[0];
            lane.id = 0;
            lane.cycle = {16, 16};
            lane.hitCount = 4;
            lane.probability = 1.0f;
            lane.baseVelocity = 100;
            lane.phraseLength = 4.0f; // a four-beat phrase, fully open
            lane.phraseGap = 0.0f;
            lane.fillPhraseShape = shape;
            // A FillLikelihood envelope drives the PROBABILISTIC fill path.
            // fillEveryNBars would not work here: `if (isFillBar) return Add;`
            // fires before the roll, so every step becomes an unconditional
            // Add and the weighted probability is never consulted -- the first
            // version of this test set it and measured exactly 1800 early
            // against 1800 late, with and without a shape.
            lane.envelopeCount = 1;
            lane.envelopes[0].active = true;
            lane.envelopes[0].envelope.target = EnvTarget::FillLikelihood;
            lane.envelopes[0].envelope.shape = Shape::Sine;
            lane.envelopes[0].envelope.periodBars = 1.0f;
            lane.envelopes[0].envelope.depth = 0.5f;

            Engine engine;
            NoteEventBuffer notes;
            EmissionEventBuffer emissions;
            TransportContext tc{};
            tc.ppqStart = 0.0;
            tc.ppqEnd = 4.0;
            tc.tempo = 120.0;
            tc.playing = true;
            engine.renderRange(tc, state, notes, &emissions);

            for (size_t i = 0; i < emissions.count; ++i) {
                const auto& e = emissions.events[i];
                if (e.kind != static_cast<uint8_t>(EmissionKind::Add))
                    continue;
                if (e.ppqPosition < 2.0)
                    ++early;
                else
                    ++late;
            }
        }
        return std::pair<int, int>{early, late};
    };

    const auto neutral = tallyFills(0.0f);
    const auto shaped = tallyFills(1.0f);

    const double neutralLate = static_cast<double>(neutral.second) / std::max(1, neutral.first + neutral.second);
    const double shapedLate = static_cast<double>(shaped.second) / std::max(1, shaped.first + shaped.second);

    EXPECT_GT(shapedLate, neutralLate) << "late share: neutral " << neutralLate << ", shaped " << shapedLate
                                       << " (early " << shaped.first << " vs late " << shaped.second << ")";
}
