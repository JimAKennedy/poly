#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "poly/engine.h"
#include "poly/macro.h"
#include "poly/types.h"

namespace {

poly::GrooveState makeTestState() {
    poly::GrooveState state{};
    state.activeLaneCount = 4;
    state.seed = 42;

    auto& kick = state.lanes[0];
    kick.id = 0;
    kick.role = poly::Role::AnchorPulse;
    kick.midiNote = 36;
    kick.cycle = {.steps = 4, .subdivision = 4};
    kick.hitCount = 4;
    kick.baseVelocity = 110;
    kick.probability = 1.0f;

    auto& snare = state.lanes[1];
    snare.id = 1;
    snare.role = poly::Role::Backbeat;
    snare.midiNote = 38;
    snare.cycle = {.steps = 4, .subdivision = 4};
    snare.hitCount = 2;
    snare.baseVelocity = 100;
    snare.probability = 1.0f;

    auto& hh = state.lanes[2];
    hh.id = 2;
    hh.role = poly::Role::Shimmer;
    hh.midiNote = 42;
    hh.cycle = {.steps = 8, .subdivision = 8};
    hh.hitCount = 8;
    hh.baseVelocity = 80;
    hh.probability = 0.9f;
    hh.velocitySpread = 0.1f;

    auto& ghost = state.lanes[3];
    ghost.id = 3;
    ghost.role = poly::Role::Ghost;
    ghost.midiNote = 45;
    ghost.cycle = {.steps = 5, .subdivision = 16};
    ghost.hitCount = 3;
    ghost.baseVelocity = 50;
    ghost.probability = 0.7f;
    ghost.ghostFloor = 25;
    ghost.velocitySpread = 0.15f;

    return state;
}

struct EventRecord {
    double ppq;
    int pitch;
    float velocity;
    double duration;

    bool operator<(const EventRecord& o) const {
        if (ppq != o.ppq)
            return ppq < o.ppq;
        if (pitch != o.pitch)
            return pitch < o.pitch;
        return velocity < o.velocity;
    }

    bool operator==(const EventRecord& o) const {
        return ppq == o.ppq && pitch == o.pitch && velocity == o.velocity && duration == o.duration;
    }
};

std::vector<EventRecord> renderSorted(poly::Engine& engine, const poly::GrooveState& state, double ppqStart,
                                      double ppqEnd, double blockPpq) {
    std::vector<EventRecord> all;
    poly::NoteEventBuffer buf;
    double ppq = ppqStart;

    while (ppq < ppqEnd) {
        double end = std::min(ppq + blockPpq, ppqEnd);
        poly::TransportContext tc{};
        tc.ppqStart = ppq;
        tc.ppqEnd = end;
        tc.tempo = 120.0;
        tc.playing = true;

        engine.renderRange(tc, state, buf);

        for (size_t i = 0; i < buf.count; ++i) {
            all.push_back(
                {buf.events[i].ppqPosition, buf.events[i].pitch, buf.events[i].velocity, buf.events[i].duration});
        }
        ppq = end;
    }

    std::sort(all.begin(), all.end());
    return all;
}

std::string serialize(const std::vector<EventRecord>& events) {
    std::ostringstream ss;
    ss << std::fixed;
    for (const auto& e : events) {
        ss << e.ppq << " " << e.pitch << " " << e.velocity << " " << e.duration << "\n";
    }
    return ss.str();
}

} // namespace

// --- Test 1: Same patch+seed reproduces identical output ---
// region:same-patch-same-seed
TEST(GoldenDeterminism, SamePatchSameSeed) {
    poly::Engine engine;
    auto state = makeTestState();

    auto run1 = renderSorted(engine, state, 0.0, 16.0, 0.5);
    auto run2 = renderSorted(engine, state, 0.0, 16.0, 0.5);

    ASSERT_EQ(run1.size(), run2.size());
    EXPECT_EQ(serialize(run1), serialize(run2));
}
// endregion:same-patch-same-seed

// --- Test 2: Different block sizes produce identical events ---
TEST(GoldenDeterminism, BlockSizeIndependence) {
    poly::Engine engine;
    auto state = makeTestState();

    auto small = renderSorted(engine, state, 0.0, 16.0, 0.05);
    auto medium = renderSorted(engine, state, 0.0, 16.0, 0.5);
    auto large = renderSorted(engine, state, 0.0, 16.0, 2.0);

    EXPECT_EQ(serialize(small), serialize(medium)) << "0.05 vs 0.5 PPQ blocks differ";
    EXPECT_EQ(serialize(small), serialize(large)) << "0.05 vs 2.0 PPQ blocks differ";
}

// --- Test 3: Loop restart produces identical sub-range ---
TEST(GoldenDeterminism, LoopRestart) {
    poly::Engine engine;
    auto state = makeTestState();

    // Render bars 0-4 straight through
    auto straight = renderSorted(engine, state, 0.0, 16.0, 0.5);

    // Extract just bars 0-2 from the straight-through
    std::vector<EventRecord> firstTwo;
    for (const auto& e : straight) {
        if (e.ppq < 8.0)
            firstTwo.push_back(e);
    }

    // Render bars 0-2 as if starting fresh (simulating loop restart)
    auto looped = renderSorted(engine, state, 0.0, 8.0, 0.5);

    EXPECT_EQ(serialize(firstTwo), serialize(looped))
        << "Loop restart of bars 0-2 differs from straight-through bars 0-2";
}

// --- Test 4: Position jump then continue matches straight-through ---
TEST(GoldenDeterminism, PositionJump) {
    poly::Engine engine;
    auto state = makeTestState();

    // Straight through bars 2-4
    auto straight = renderSorted(engine, state, 8.0, 16.0, 0.5);

    // Jump: render bars 0-1, then jump to bar 2 and continue
    auto ignored = renderSorted(engine, state, 0.0, 4.0, 0.5);
    auto jumped = renderSorted(engine, state, 8.0, 16.0, 0.5);

    EXPECT_EQ(serialize(straight), serialize(jumped)) << "Position jump then continue differs from straight-through";
}

// --- Test 5: Different seed produces different output ---
TEST(GoldenDeterminism, DifferentSeedDiffers) {
    poly::Engine engine;
    auto state1 = makeTestState();
    auto state2 = makeTestState();
    state2.seed = 999;

    auto run1 = renderSorted(engine, state1, 0.0, 16.0, 0.5);
    auto run2 = renderSorted(engine, state2, 0.0, 16.0, 0.5);

    EXPECT_NE(serialize(run1), serialize(run2));
}

// --- Test 6: Tempo change doesn't affect PPQ positions ---
TEST(GoldenDeterminism, TempoIndependence) {
    poly::Engine engine;
    auto state = makeTestState();

    // Render at 120 BPM vs 90 BPM — PPQ positions should be identical
    // because the engine works in PPQ space, not real-time
    auto at120 = renderSorted(engine, state, 0.0, 16.0, 0.5);

    std::vector<EventRecord> at90;
    poly::NoteEventBuffer buf;
    double ppq = 0.0;
    while (ppq < 16.0) {
        double end = std::min(ppq + 0.5, 16.0);
        poly::TransportContext tc{};
        tc.ppqStart = ppq;
        tc.ppqEnd = end;
        tc.tempo = 90.0;
        tc.playing = true;

        engine.renderRange(tc, state, buf);
        for (size_t i = 0; i < buf.count; ++i) {
            at90.push_back(
                {buf.events[i].ppqPosition, buf.events[i].pitch, buf.events[i].velocity, buf.events[i].duration});
        }
        ppq = end;
    }
    std::sort(at90.begin(), at90.end());

    EXPECT_EQ(serialize(at120), serialize(at90)) << "Tempo change affected PPQ positions";
}

// --- Test 7: Not playing produces no events ---
TEST(GoldenDeterminism, NotPlaying) {
    poly::Engine engine;
    auto state = makeTestState();

    poly::NoteEventBuffer buf;
    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0;
    tc.tempo = 120.0;
    tc.playing = false;

    engine.renderRange(tc, state, buf);
    EXPECT_EQ(buf.count, 0u);
}

// --- Test 8: Polymetric cycle lengths create non-repeating patterns ---
TEST(GoldenDeterminism, PolymetricPhaseVariation) {
    poly::Engine engine;
    auto state = makeTestState();
    state.lanes[3].probability = 1.0f;

    // Ghost lane has 5/16 cycle. Over 4 bars (16 PPQ), the 5-step cycle
    // doesn't divide evenly, so the pattern against the 4/4 kick should
    // show phase drift.
    auto events = renderSorted(engine, state, 0.0, 16.0, 0.5);

    // Collect ghost (pitch 45) PPQ positions relative to bar start
    std::vector<double> ghostPhases;
    for (const auto& e : events) {
        if (e.pitch == 45) {
            ghostPhases.push_back(std::fmod(e.ppq, 4.0));
        }
    }

    // Over 4 bars, ghost positions within each bar should NOT all be identical
    // (the 5-step cycle misaligns with 4/4)
    std::vector<double> bar0, bar1;
    for (const auto& e : events) {
        if (e.pitch == 45 && e.ppq < 4.0)
            bar0.push_back(std::fmod(e.ppq, 4.0));
        if (e.pitch == 45 && e.ppq >= 4.0 && e.ppq < 8.0)
            bar1.push_back(e.ppq - 4.0);
    }

    EXPECT_FALSE(bar0.empty());
    EXPECT_FALSE(bar1.empty());
    // The ghost hit positions relative to the bar should differ between bars
    // because 5 steps of 1/16 note = 1.25 PPQ cycle, which doesn't align with 4 PPQ bars
    bool hasDifference = bar0.size() != bar1.size();
    if (!hasDifference && bar0.size() == bar1.size()) {
        for (size_t i = 0; i < bar0.size(); ++i) {
            if (std::abs(bar0[i] - bar1[i]) > 0.001) {
                hasDifference = true;
                break;
            }
        }
    }
    EXPECT_TRUE(hasDifference) << "Polymetric ghost lane should show phase drift across bars";
}

// --- Test 9: Dynamic shaping features maintain determinism across block sizes ---
TEST(GoldenDeterminism, DynamicShapingBlockIndependence) {
    poly::Engine engine;
    auto state = makeTestState();

    // Configure dynamic shaping on the kick lane
    state.lanes[0].accents.steps[0] = 1.0f;
    state.lanes[0].accents.steps[2] = 1.0f;
    state.lanes[0].emphasisProb = 0.7f;
    state.lanes[0].ghostFloor = 20;

    // Ghost lane already has ghostFloor=25; add accents
    state.lanes[3].accents.steps[0] = 1.0f;
    state.lanes[3].accents.steps[3] = 1.0f;
    state.lanes[3].emphasisProb = 0.5f;

    auto small = renderSorted(engine, state, 0.0, 16.0, 0.05);
    auto medium = renderSorted(engine, state, 0.0, 16.0, 0.5);
    auto large = renderSorted(engine, state, 0.0, 16.0, 2.0);

    EXPECT_EQ(serialize(small), serialize(medium)) << "Dynamic shaping: 0.05 vs 0.5 PPQ blocks differ";
    EXPECT_EQ(serialize(small), serialize(large)) << "Dynamic shaping: 0.05 vs 2.0 PPQ blocks differ";
}

// --- Test 10: Dynamic shaping deterministic across loop restarts ---
TEST(GoldenDeterminism, DynamicShapingLoopRestart) {
    poly::Engine engine;
    auto state = makeTestState();

    state.lanes[0].accents.steps[0] = 1.0f;
    state.lanes[0].emphasisProb = 0.8f;
    state.lanes[3].ghostFloor = 40;

    auto straight = renderSorted(engine, state, 0.0, 16.0, 0.5);

    std::vector<EventRecord> firstTwo;
    for (const auto& e : straight) {
        if (e.ppq < 8.0)
            firstTwo.push_back(e);
    }

    auto looped = renderSorted(engine, state, 0.0, 8.0, 0.5);

    EXPECT_EQ(serialize(firstTwo), serialize(looped)) << "Dynamic shaping: loop restart differs from straight-through";
}

// --- Test 11: Envelope modulation deterministic across block sizes ---
TEST(GoldenDeterminism, EnvelopeBlockIndependence) {
    poly::Engine engine;
    auto state = makeTestState();

    // Per-lane velocity envelope on kick: 3-bar sine
    state.lanes[0].envelopes[0].envelope = {poly::EnvTarget::Velocity, 3.0f, poly::Shape::Sine, 0.8f, 0.0f};
    state.lanes[0].envelopes[0].active = true;
    state.lanes[0].envelopeCount = 1;

    // Per-lane density envelope on hi-hat: 7-bar ramp
    state.lanes[2].envelopes[0].envelope = {poly::EnvTarget::Density, 7.0f, poly::Shape::Ramp, 0.6f, 0.25f};
    state.lanes[2].envelopes[0].active = true;
    state.lanes[2].envelopeCount = 1;

    // Global velocity envelope: 5-bar triangle
    state.globalEnvelopes[0] = {poly::EnvTarget::Velocity, 5.0f, poly::Shape::Triangle, 0.5f, 0.1f};
    state.globalEnvelopeCount = 1;

    // 8 bars = 32 PPQ — enough for non-dividing periods to show emergence
    auto small = renderSorted(engine, state, 0.0, 32.0, 0.05);
    auto medium = renderSorted(engine, state, 0.0, 32.0, 0.5);
    auto large = renderSorted(engine, state, 0.0, 32.0, 2.0);

    EXPECT_EQ(serialize(small), serialize(medium)) << "Envelope: 0.05 vs 0.5 PPQ blocks differ";
    EXPECT_EQ(serialize(small), serialize(large)) << "Envelope: 0.05 vs 2.0 PPQ blocks differ";
}

// --- Test 12: Swing + humanize deterministic across block sizes ---
TEST(GoldenDeterminism, SwingHumanizeBlockIndependence) {
    poly::Engine engine;
    auto state = makeTestState();

    state.lanes[2].swingAmount = 0.5f;
    state.lanes[2].humanizeMs = 5.0f;
    state.lanes[0].noteDuration = 0.75f;

    auto small = renderSorted(engine, state, 0.0, 16.0, 0.05);
    auto medium = renderSorted(engine, state, 0.0, 16.0, 0.5);
    auto large = renderSorted(engine, state, 0.0, 16.0, 2.0);

    EXPECT_EQ(serialize(small), serialize(medium)) << "Swing/humanize: 0.05 vs 0.5 PPQ blocks differ";
    EXPECT_EQ(serialize(small), serialize(large)) << "Swing/humanize: 0.05 vs 2.0 PPQ blocks differ";
}

// --- Test 13: Envelope modulation deterministic across loop restarts ---
TEST(GoldenDeterminism, EnvelopeLoopRestart) {
    poly::Engine engine;
    auto state = makeTestState();

    state.lanes[0].envelopes[0].envelope = {poly::EnvTarget::Velocity, 3.0f, poly::Shape::Sine, 0.8f, 0.0f};
    state.lanes[0].envelopes[0].active = true;
    state.lanes[0].envelopeCount = 1;

    state.globalEnvelopes[0] = {poly::EnvTarget::Velocity, 5.0f, poly::Shape::Triangle, 0.5f, 0.1f};
    state.globalEnvelopeCount = 1;

    auto straight = renderSorted(engine, state, 0.0, 32.0, 0.5);

    std::vector<EventRecord> firstHalf;
    for (const auto& e : straight) {
        if (e.ppq < 16.0)
            firstHalf.push_back(e);
    }

    auto looped = renderSorted(engine, state, 0.0, 16.0, 0.5);

    EXPECT_EQ(serialize(firstHalf), serialize(looped)) << "Envelope: loop restart differs from straight-through";
}

// --- Test 14: Macro-resolved state deterministic across block sizes ---
TEST(GoldenDeterminism, MacroResolvedBlockIndependence) {
    poly::Engine engine;
    auto state = makeTestState();

    state.macros.complexity = 0.7f;
    state.macros.density = 0.6f;
    state.macros.syncopation = 0.3f;
    state.macros.swing = 0.2f;
    state.macros.tension = 0.4f;
    state.macros.humanize = 0.3f;

    auto resolved = poly::resolveMacros(state);

    auto small = renderSorted(engine, resolved, 0.0, 16.0, 0.05);
    auto medium = renderSorted(engine, resolved, 0.0, 16.0, 0.5);
    auto large = renderSorted(engine, resolved, 0.0, 16.0, 2.0);

    EXPECT_EQ(serialize(small), serialize(medium)) << "Macro-resolved: 0.05 vs 0.5 PPQ blocks differ";
    EXPECT_EQ(serialize(small), serialize(large)) << "Macro-resolved: 0.05 vs 2.0 PPQ blocks differ";
}

// --- Test 15: Extended envelope targets deterministic across block sizes ---
TEST(GoldenDeterminism, ExtendedEnvelopeTargetsBlockIndependence) {
    poly::Engine engine;
    auto state = makeTestState();

    // AccentBias on kick
    state.lanes[0].accents.steps[0] = 1.0f;
    state.lanes[0].accents.steps[2] = 1.0f;
    state.lanes[0].emphasisProb = 0.3f;
    state.lanes[0].envelopes[0].envelope = {poly::EnvTarget::AccentBias, 3.0f, poly::Shape::Sine, 0.8f, 0.0f};
    state.lanes[0].envelopes[0].active = true;
    state.lanes[0].envelopeCount = 1;

    // NoteLength on snare
    state.lanes[1].envelopes[0].envelope = {poly::EnvTarget::NoteLength, 5.0f, poly::Shape::Triangle, 0.7f, 0.15f};
    state.lanes[1].envelopes[0].active = true;
    state.lanes[1].envelopeCount = 1;

    // TimingLooseness on hi-hat
    state.lanes[2].envelopes[0].envelope = {poly::EnvTarget::TimingLooseness, 7.0f, poly::Shape::Ramp, 0.5f, 0.0f};
    state.lanes[2].envelopes[0].active = true;
    state.lanes[2].envelopeCount = 1;

    // ActivationWeight on ghost lane
    state.lanes[3].envelopes[0].envelope = {poly::EnvTarget::ActivationWeight, 4.0f, poly::Shape::Sine, 0.6f, 0.0f};
    state.lanes[3].envelopes[0].active = true;
    state.lanes[3].envelopeCount = 1;

    // Global FillLikelihood envelope
    state.globalEnvelopes[0] = {poly::EnvTarget::FillLikelihood, 8.0f, poly::Shape::Triangle, 0.3f, 0.0f};
    state.globalEnvelopeCount = 1;

    auto small = renderSorted(engine, state, 0.0, 32.0, 0.05);
    auto medium = renderSorted(engine, state, 0.0, 32.0, 0.5);
    auto large = renderSorted(engine, state, 0.0, 32.0, 2.0);

    EXPECT_EQ(serialize(small), serialize(medium)) << "Extended envelope: 0.05 vs 0.5 PPQ blocks differ";
    EXPECT_EQ(serialize(small), serialize(large)) << "Extended envelope: 0.05 vs 2.0 PPQ blocks differ";
}

// --- Test 16: phraseLength=0 (continuous) produces identical output to default ---
TEST(GoldenPhrase, ContinuousMatchesDefault) {
    poly::Engine engine;
    auto state = makeTestState();

    auto baseline = renderSorted(engine, state, 0.0, 16.0, 0.5);

    for (int i = 0; i < state.activeLaneCount; ++i) {
        state.lanes[i].phraseLength = 0.0f;
        state.lanes[i].phraseGap = 0.0f;
        state.lanes[i].phraseOffset = 0.0f;
    }

    auto withPhrase = renderSorted(engine, state, 0.0, 16.0, 0.5);
    EXPECT_EQ(serialize(baseline), serialize(withPhrase))
        << "phraseLength=0 should produce identical output to default";
}

// --- Test 17: Single lane with phraseLength=2, phraseGap=1 produces silence during gap ---
TEST(GoldenPhrase, SingleLaneGapSilence) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;

    auto& kick = state.lanes[0];
    kick.id = 0;
    kick.midiNote = 36;
    kick.cycle = {.steps = 4, .subdivision = 4};
    kick.hitCount = 4;
    kick.baseVelocity = 100;
    kick.probability = 1.0f;
    kick.phraseLength = 8.0f;
    kick.phraseGap = 4.0f;

    // Phrase cycle = 12 beats = 12 PPQ. Play 0-8 PPQ, gap 8-12, play 12-20, gap 20-24
    auto events = renderSorted(engine, state, 0.0, 24.0, 0.5);

    for (const auto& e : events) {
        double posInCycle = std::fmod(e.ppq, 12.0);
        EXPECT_LT(posInCycle, 8.0) << "Note at PPQ " << e.ppq << " falls in gap region (phrasePos=" << posInCycle
                                   << ")";
    }
    EXPECT_FALSE(events.empty()) << "Should produce some notes during play regions";
}

// --- Test 18: Two lanes with different phraseLength create offset phrase behavior ---
TEST(GoldenPhrase, OffsetPhraseBehavior) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 2;
    state.seed = 42;

    auto& lane0 = state.lanes[0];
    lane0.id = 0;
    lane0.midiNote = 36;
    lane0.cycle = {.steps = 4, .subdivision = 4};
    lane0.hitCount = 4;
    lane0.baseVelocity = 100;
    lane0.probability = 1.0f;
    lane0.phraseLength = 8.0f;
    lane0.phraseGap = 4.0f;

    auto& lane1 = state.lanes[1];
    lane1.id = 1;
    lane1.midiNote = 38;
    lane1.cycle = {.steps = 4, .subdivision = 4};
    lane1.hitCount = 4;
    lane1.baseVelocity = 100;
    lane1.probability = 1.0f;
    lane1.phraseLength = 12.0f;
    lane1.phraseGap = 4.0f;

    // Lane 0: 12-beat cycle (8+4), Lane 1: 16-beat cycle (12+4)
    // Over 24 PPQ (6 bars), their gaps should not always overlap
    auto events = renderSorted(engine, state, 0.0, 24.0, 0.5);

    bool hasLane0 = false, hasLane1 = false;
    bool bothSilentSomewhere = false;
    bool oneSilentOtherPlaying = false;

    // Check bar by bar
    for (int bar = 0; bar < 6; ++bar) {
        double barStart = bar * 4.0;
        double barEnd = barStart + 4.0;
        bool l0Active = false, l1Active = false;
        for (const auto& e : events) {
            if (e.ppq >= barStart && e.ppq < barEnd) {
                if (e.pitch == 36)
                    l0Active = true;
                if (e.pitch == 38)
                    l1Active = true;
            }
        }
        if (l0Active)
            hasLane0 = true;
        if (l1Active)
            hasLane1 = true;
        if (!l0Active && !l1Active)
            bothSilentSomewhere = true;
        if (l0Active != l1Active)
            oneSilentOtherPlaying = true;
    }

    EXPECT_TRUE(hasLane0) << "Lane 0 should produce notes";
    EXPECT_TRUE(hasLane1) << "Lane 1 should produce notes";
    EXPECT_TRUE(oneSilentOtherPlaying)
        << "Different phrase lengths should create bars where one lane rests while the other plays";
}

// --- Test 19: Phrase with transport jump into gap region ---
TEST(GoldenPhrase, TransportJumpIntoGap) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;

    auto& kick = state.lanes[0];
    kick.id = 0;
    kick.midiNote = 36;
    kick.cycle = {.steps = 4, .subdivision = 4};
    kick.hitCount = 4;
    kick.baseVelocity = 100;
    kick.probability = 1.0f;
    kick.phraseLength = 8.0f;
    kick.phraseGap = 4.0f;

    // Gap is at PPQ 8-12. Jump directly into that range.
    auto events = renderSorted(engine, state, 8.0, 12.0, 0.5);
    EXPECT_EQ(events.size(), 0u) << "Jumping into gap region should produce no notes";

    // But jumping to PPQ 12 (start of next play region) should produce notes
    auto events2 = renderSorted(engine, state, 12.0, 16.0, 0.5);
    EXPECT_GT(events2.size(), 0u) << "Jumping to play region after gap should produce notes";
}

// --- Test 20: Phrase deterministic across block sizes ---
TEST(GoldenPhrase, BlockSizeIndependence) {
    poly::Engine engine;
    auto state = makeTestState();

    for (int i = 0; i < state.activeLaneCount; ++i) {
        state.lanes[i].phraseLength = 8.0f;
        state.lanes[i].phraseGap = 2.0f;
        state.lanes[i].phraseOffset = static_cast<float>(i) * 1.0f;
    }

    auto small = renderSorted(engine, state, 0.0, 32.0, 0.05);
    auto medium = renderSorted(engine, state, 0.0, 32.0, 0.5);
    auto large = renderSorted(engine, state, 0.0, 32.0, 2.0);

    EXPECT_EQ(serialize(small), serialize(medium)) << "Phrase: 0.05 vs 0.5 PPQ blocks differ";
    EXPECT_EQ(serialize(small), serialize(large)) << "Phrase: 0.05 vs 2.0 PPQ blocks differ";
}

// --- Test 22: mutationRate=0 produces identical output to baseline ---
TEST(GoldenMutation, ZeroRateMatchesBaseline) {
    poly::Engine engine;
    auto state = makeTestState();
    auto baseline = renderSorted(engine, state, 0.0, 16.0, 0.5);

    for (int i = 0; i < state.activeLaneCount; ++i)
        state.lanes[i].mutationRate = 0.0f;

    auto withZeroMutation = renderSorted(engine, state, 0.0, 16.0, 0.5);
    EXPECT_EQ(serialize(baseline), serialize(withZeroMutation))
        << "mutationRate=0 should produce identical output to default";
}

// --- Test 23: mutationRate>0 produces deterministic variations ---
TEST(GoldenMutation, DeterministicVariations) {
    poly::Engine engine;
    auto state = makeTestState();
    state.lanes[2].mutationRate = 0.3f;

    auto run1 = renderSorted(engine, state, 0.0, 32.0, 0.5);
    auto run2 = renderSorted(engine, state, 0.0, 32.0, 0.5);

    ASSERT_EQ(run1.size(), run2.size());
    EXPECT_EQ(serialize(run1), serialize(run2)) << "Mutation must be deterministic across runs";

    auto noMutation = makeTestState();
    auto baseline = renderSorted(engine, noMutation, 0.0, 32.0, 0.5);
    EXPECT_NE(serialize(run1), serialize(baseline)) << "mutationRate=0.3 should differ from no mutation";
}

// --- Test 24: Mutation respects anchor steps ---
TEST(GoldenMutation, RespectsAnchors) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;

    auto& kick = state.lanes[0];
    kick.id = 0;
    kick.midiNote = 36;
    kick.cycle = {.steps = 4, .subdivision = 4};
    kick.hitCount = 4;
    kick.baseVelocity = 100;
    kick.probability = 1.0f;

    kick.constraints.anchorSteps.steps[0] = 1.0f;
    kick.constraints.anchorSteps.steps[2] = 1.0f;

    auto baseline = renderSorted(engine, state, 0.0, 16.0, 0.5);

    kick.mutationRate = 1.0f;
    auto mutated = renderSorted(engine, state, 0.0, 16.0, 0.5);

    auto anchorEvents = [](const std::vector<EventRecord>& events, int steps, int sub) {
        std::vector<EventRecord> anchors;
        double sPpq = 4.0 / sub;
        for (const auto& e : events) {
            int64_t absStep = static_cast<int64_t>(std::round(e.ppq / sPpq));
            int cycleStep = static_cast<int>(((absStep % steps) + steps) % steps);
            if (cycleStep == 0 || cycleStep == 2)
                anchors.push_back(e);
        }
        return anchors;
    };

    auto baselineAnchors = anchorEvents(baseline, kick.cycle.steps, kick.cycle.subdivision);
    auto mutatedAnchors = anchorEvents(mutated, kick.cycle.steps, kick.cycle.subdivision);

    EXPECT_EQ(serialize(baselineAnchors), serialize(mutatedAnchors))
        << "Anchor steps must be identical with and without mutation";
    EXPECT_FALSE(baselineAnchors.empty()) << "Should have anchor step events";
}

// --- Test 25: Mutation deterministic across block sizes ---
TEST(GoldenMutation, BlockSizeIndependence) {
    poly::Engine engine;
    auto state = makeTestState();
    for (int i = 0; i < state.activeLaneCount; ++i)
        state.lanes[i].mutationRate = 0.4f;

    auto small = renderSorted(engine, state, 0.0, 32.0, 0.05);
    auto medium = renderSorted(engine, state, 0.0, 32.0, 0.5);
    auto large = renderSorted(engine, state, 0.0, 32.0, 2.0);

    EXPECT_EQ(serialize(small), serialize(medium)) << "Mutation: 0.05 vs 0.5 PPQ blocks differ";
    EXPECT_EQ(serialize(small), serialize(large)) << "Mutation: 0.05 vs 2.0 PPQ blocks differ";
}

// --- Test 26: driftRate=0 produces identical output to baseline ---
TEST(GoldenDrift, ZeroRateMatchesBaseline) {
    poly::Engine engine;
    auto state = makeTestState();
    auto baseline = renderSorted(engine, state, 0.0, 16.0, 0.5);

    for (int i = 0; i < state.activeLaneCount; ++i)
        state.lanes[i].driftRate = 0.0f;

    auto withZeroDrift = renderSorted(engine, state, 0.0, 16.0, 0.5);
    EXPECT_EQ(serialize(baseline), serialize(withZeroDrift))
        << "driftRate=0 should produce identical output to default";
}

// --- Test 27: driftRate=1.0 rotates pattern by 1 step per bar ---
TEST(GoldenDrift, OneStepPerBar) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;

    auto& kick = state.lanes[0];
    kick.id = 0;
    kick.midiNote = 36;
    kick.cycle = {.steps = 8, .subdivision = 8};
    kick.hitCount = 3;
    kick.baseVelocity = 100;
    kick.probability = 1.0f;

    // Render without drift
    auto noDrift = renderSorted(engine, state, 0.0, 16.0, 0.5);

    // Render with drift=1.0 (1 step per bar)
    kick.driftRate = 1.0f;
    auto withDrift = renderSorted(engine, state, 0.0, 16.0, 0.5);

    EXPECT_NE(serialize(noDrift), serialize(withDrift))
        << "driftRate=1.0 should produce different pattern than no drift";

    // Verify determinism
    auto withDrift2 = renderSorted(engine, state, 0.0, 16.0, 0.5);
    EXPECT_EQ(serialize(withDrift), serialize(withDrift2)) << "Drift must be deterministic across runs";
}

// --- Test 28: Two lanes with different driftRates produce phasing ---
TEST(GoldenDrift, PhasingBetweenLanes) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 2;
    state.seed = 42;

    // Two identical lanes except for drift
    for (int i = 0; i < 2; ++i) {
        state.lanes[i].id = i;
        state.lanes[i].midiNote = static_cast<int16_t>(36 + i * 2);
        state.lanes[i].cycle = {.steps = 4, .subdivision = 4};
        state.lanes[i].hitCount = 3;
        state.lanes[i].baseVelocity = 100;
        state.lanes[i].probability = 1.0f;
    }

    // Lane 0: no drift, Lane 1: drift=1 step/bar
    state.lanes[0].driftRate = 0.0f;
    state.lanes[1].driftRate = 1.0f;

    auto events = renderSorted(engine, state, 0.0, 32.0, 0.5);

    // Collect per-bar relative positions for each lane
    bool phaseDiffers = false;
    for (int bar = 0; bar < 7; ++bar) {
        double barStart = bar * 4.0;
        double barEnd = barStart + 4.0;
        double nextBarStart = (bar + 1) * 4.0;
        double nextBarEnd = nextBarStart + 4.0;

        std::vector<double> l1Bar, l1Next;
        for (const auto& e : events) {
            if (e.pitch == 38 && e.ppq >= barStart && e.ppq < barEnd)
                l1Bar.push_back(e.ppq - barStart);
            if (e.pitch == 38 && e.ppq >= nextBarStart && e.ppq < nextBarEnd)
                l1Next.push_back(e.ppq - nextBarStart);
        }

        if (l1Bar.size() == l1Next.size() && !l1Bar.empty()) {
            for (size_t i = 0; i < l1Bar.size(); ++i) {
                if (std::abs(l1Bar[i] - l1Next[i]) > 0.001) {
                    phaseDiffers = true;
                    break;
                }
            }
        } else if (l1Bar.size() != l1Next.size()) {
            phaseDiffers = true;
        }
        if (phaseDiffers)
            break;
    }

    EXPECT_TRUE(phaseDiffers) << "Lane with driftRate=1.0 should show different bar-relative positions across bars";
}

// --- Test 29: Transport jump with drift produces correct rotation ---
TEST(GoldenDrift, TransportJumpCorrectRotation) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;

    auto& kick = state.lanes[0];
    kick.id = 0;
    kick.midiNote = 36;
    kick.cycle = {.steps = 8, .subdivision = 8};
    kick.hitCount = 3;
    kick.baseVelocity = 100;
    kick.probability = 1.0f;
    kick.driftRate = 0.5f;

    // Render straight through bars 0-8 (32 PPQ)
    auto straight = renderSorted(engine, state, 0.0, 32.0, 0.5);

    // Extract bars 4-8
    std::vector<EventRecord> straightTail;
    for (const auto& e : straight) {
        if (e.ppq >= 16.0)
            straightTail.push_back(e);
    }

    // Jump directly to bar 4 and render bars 4-8
    auto jumped = renderSorted(engine, state, 16.0, 32.0, 0.5);

    EXPECT_EQ(serialize(straightTail), serialize(jumped))
        << "Transport jump with drift must produce same output as straight-through";
}

// --- Test 30: Drift deterministic across block sizes ---
TEST(GoldenDrift, BlockSizeIndependence) {
    poly::Engine engine;
    auto state = makeTestState();
    for (int i = 0; i < state.activeLaneCount; ++i)
        state.lanes[i].driftRate = 0.75f;

    auto small = renderSorted(engine, state, 0.0, 32.0, 0.05);
    auto medium = renderSorted(engine, state, 0.0, 32.0, 0.5);
    auto large = renderSorted(engine, state, 0.0, 32.0, 2.0);

    EXPECT_EQ(serialize(small), serialize(medium)) << "Drift: 0.05 vs 0.5 PPQ blocks differ";
    EXPECT_EQ(serialize(small), serialize(large)) << "Drift: 0.05 vs 2.0 PPQ blocks differ";
}

// --- Test 21: Phrase with phraseOffset shifts the gap position ---
TEST(GoldenPhrase, OffsetShiftsGap) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;

    auto& kick = state.lanes[0];
    kick.id = 0;
    kick.midiNote = 36;
    kick.cycle = {.steps = 4, .subdivision = 4};
    kick.hitCount = 4;
    kick.baseVelocity = 100;
    kick.probability = 1.0f;
    kick.phraseLength = 8.0f;
    kick.phraseGap = 4.0f;
    kick.phraseOffset = 4.0f;

    // With offset=4 beats (4 PPQ), the phrase cycle shifts: play starts at PPQ 4
    // Gap originally at PPQ 8-12 now shifted to PPQ 12-16
    auto events = renderSorted(engine, state, 0.0, 24.0, 0.5);

    auto noOffset = state;
    noOffset.lanes[0].phraseOffset = 0.0f;
    auto eventsNoOffset = renderSorted(engine, noOffset, 0.0, 24.0, 0.5);

    EXPECT_NE(serialize(events), serialize(eventsNoOffset)) << "phraseOffset should shift the gap position";
}

// --- Test 31: timingOffsetMs=0 produces identical output to baseline ---
TEST(GoldenTimingOffset, ZeroMatchesBaseline) {
    poly::Engine engine;
    auto state = makeTestState();
    auto baseline = renderSorted(engine, state, 0.0, 16.0, 0.5);

    for (int i = 0; i < state.activeLaneCount; ++i)
        state.lanes[i].timingOffsetMs = 0.0f;

    auto withZeroOffset = renderSorted(engine, state, 0.0, 16.0, 0.5);
    EXPECT_EQ(serialize(baseline), serialize(withZeroOffset))
        << "timingOffsetMs=0 should produce identical output to default";
}

// --- Test 32: Positive offset shifts events later ---
TEST(GoldenTimingOffset, PositiveShiftsLater) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;

    auto& kick = state.lanes[0];
    kick.id = 0;
    kick.midiNote = 36;
    kick.cycle = {.steps = 4, .subdivision = 4};
    kick.hitCount = 4;
    kick.baseVelocity = 100;
    kick.probability = 1.0f;

    auto baseline = renderSorted(engine, state, 0.0, 16.0, 0.5);

    kick.timingOffsetMs = 5.0f;
    auto shifted = renderSorted(engine, state, 0.0, 16.0, 0.5);

    ASSERT_EQ(baseline.size(), shifted.size()) << "Same number of events with positive offset";
    for (size_t i = 0; i < baseline.size(); ++i) {
        EXPECT_GT(shifted[i].ppq, baseline[i].ppq) << "Event " << i << " should be shifted later with positive offset";
    }
}

// --- Test 33: Negative offset shifts events earlier ---
TEST(GoldenTimingOffset, NegativeShiftsEarlier) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;

    auto& kick = state.lanes[0];
    kick.id = 0;
    kick.midiNote = 36;
    kick.cycle = {.steps = 4, .subdivision = 4};
    kick.hitCount = 4;
    kick.baseVelocity = 100;
    kick.probability = 1.0f;

    auto baseline = renderSorted(engine, state, 0.0, 15.5, 0.5);

    kick.timingOffsetMs = -5.0f;
    auto shifted = renderSorted(engine, state, 0.0, 15.5, 0.5);

    ASSERT_EQ(baseline.size(), shifted.size()) << "Same number of events with negative offset";
    // Skip event 0 (PPQ 0.0) since negative offset clamps to 0.0
    for (size_t i = 1; i < baseline.size(); ++i) {
        EXPECT_LT(shifted[i].ppq, baseline[i].ppq)
            << "Event " << i << " should be shifted earlier with negative offset";
    }
}

// --- Test 34: Timing offset interacts correctly with swing ---
TEST(GoldenTimingOffset, InteractsWithSwing) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;

    auto& hh = state.lanes[0];
    hh.id = 0;
    hh.midiNote = 42;
    hh.cycle = {.steps = 8, .subdivision = 8};
    hh.hitCount = 8;
    hh.baseVelocity = 80;
    hh.probability = 1.0f;
    hh.swingAmount = 0.5f;

    auto swingOnly = renderSorted(engine, state, 0.0, 16.0, 0.5);

    hh.timingOffsetMs = 3.0f;
    auto swingPlusOffset = renderSorted(engine, state, 0.0, 16.0, 0.5);

    ASSERT_EQ(swingOnly.size(), swingPlusOffset.size());
    EXPECT_NE(serialize(swingOnly), serialize(swingPlusOffset))
        << "Adding timing offset to swung pattern should change positions";

    for (size_t i = 0; i < swingOnly.size(); ++i) {
        EXPECT_GT(swingPlusOffset[i].ppq, swingOnly[i].ppq)
            << "Event " << i << " should be shifted later with positive offset on top of swing";
    }
}

// --- Test 36: kotekanSourceLane=-1 produces identical output to baseline ---
TEST(GoldenKotekan, NoSourceMatchesBaseline) {
    poly::Engine engine;
    auto state = makeTestState();
    auto baseline = renderSorted(engine, state, 0.0, 16.0, 0.5);

    for (int i = 0; i < state.activeLaneCount; ++i)
        state.lanes[i].kotekanSourceLane = -1;

    auto withNoKotekan = renderSorted(engine, state, 0.0, 16.0, 0.5);
    EXPECT_EQ(serialize(baseline), serialize(withNoKotekan))
        << "kotekanSourceLane=-1 should produce identical output to default";
}

// --- Test 37: Kotekan pair produces exact complement ---
TEST(GoldenKotekan, PairProducesComplement) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 2;
    state.seed = 42;

    auto& laneA = state.lanes[0];
    laneA.id = 0;
    laneA.midiNote = 36;
    laneA.cycle = {.steps = 8, .subdivision = 8};
    laneA.hitCount = 3;
    laneA.baseVelocity = 100;
    laneA.probability = 1.0f;

    auto& laneB = state.lanes[1];
    laneB.id = 1;
    laneB.midiNote = 38;
    laneB.cycle = {.steps = 8, .subdivision = 8};
    laneB.hitCount = 3;
    laneB.baseVelocity = 80;
    laneB.probability = 1.0f;
    laneB.kotekanSourceLane = 0;

    auto events = renderSorted(engine, state, 0.0, 4.0, 0.5);

    // In one bar (4 PPQ = 8 eighth-note steps), source has 3 hits,
    // complement should have 5 hits. Together they fill all 8 positions.
    int countA = 0, countB = 0;
    for (const auto& e : events) {
        if (e.pitch == 36)
            countA++;
        if (e.pitch == 38)
            countB++;
    }

    EXPECT_EQ(countA, 3) << "Source lane should have 3 hits per cycle";
    EXPECT_EQ(countB, 5) << "Complement lane should have 5 hits (8 - 3)";
    EXPECT_EQ(countA + countB, 8) << "Source + complement should fill all 8 step positions";
}

// --- Test 38: Kotekan complement with different velocities ---
TEST(GoldenKotekan, DifferentVelocities) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 2;
    state.seed = 42;

    auto& laneA = state.lanes[0];
    laneA.id = 0;
    laneA.midiNote = 36;
    laneA.cycle = {.steps = 8, .subdivision = 8};
    laneA.hitCount = 3;
    laneA.baseVelocity = 127;
    laneA.probability = 1.0f;

    auto& laneB = state.lanes[1];
    laneB.id = 1;
    laneB.midiNote = 38;
    laneB.cycle = {.steps = 8, .subdivision = 8};
    laneB.hitCount = 3;
    laneB.baseVelocity = 50;
    laneB.probability = 1.0f;
    laneB.kotekanSourceLane = 0;

    auto events = renderSorted(engine, state, 0.0, 4.0, 0.5);

    for (const auto& e : events) {
        if (e.pitch == 36)
            EXPECT_NEAR(e.velocity, 1.0f, 0.15f) << "Source lane should use its own high velocity";
        if (e.pitch == 38)
            EXPECT_NEAR(e.velocity, 50.0f / 127.0f, 0.15f) << "Complement lane should use its own low velocity";
    }
}

// --- Test 39: Circular kotekan reference degrades to independent ---
TEST(GoldenKotekan, CircularReferenceFallback) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 2;
    state.seed = 42;

    for (int i = 0; i < 2; ++i) {
        state.lanes[i].id = i;
        state.lanes[i].midiNote = static_cast<int16_t>(36 + i * 2);
        state.lanes[i].cycle = {.steps = 8, .subdivision = 8};
        state.lanes[i].hitCount = 3;
        state.lanes[i].baseVelocity = 100;
        state.lanes[i].probability = 1.0f;
    }

    state.lanes[0].kotekanSourceLane = 1;
    state.lanes[1].kotekanSourceLane = 0;

    auto events = renderSorted(engine, state, 0.0, 4.0, 0.5);

    int countA = 0, countB = 0;
    for (const auto& e : events) {
        if (e.pitch == 36)
            countA++;
        if (e.pitch == 38)
            countB++;
    }

    EXPECT_EQ(countA, 3) << "Circular ref lane A should fall back to own Euclidean pattern (3 hits)";
    EXPECT_EQ(countB, 3) << "Circular ref lane B should fall back to own Euclidean pattern (3 hits)";
}

// --- Test 40: Kotekan deterministic across block sizes ---
TEST(GoldenKotekan, BlockSizeIndependence) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 2;
    state.seed = 42;

    for (int i = 0; i < 2; ++i) {
        state.lanes[i].id = i;
        state.lanes[i].midiNote = static_cast<int16_t>(36 + i * 2);
        state.lanes[i].cycle = {.steps = 8, .subdivision = 8};
        state.lanes[i].hitCount = 3;
        state.lanes[i].baseVelocity = 100;
        state.lanes[i].probability = 1.0f;
    }
    state.lanes[1].kotekanSourceLane = 0;

    auto small = renderSorted(engine, state, 0.0, 16.0, 0.05);
    auto medium = renderSorted(engine, state, 0.0, 16.0, 0.5);
    auto large = renderSorted(engine, state, 0.0, 16.0, 2.0);

    EXPECT_EQ(serialize(small), serialize(medium)) << "Kotekan: 0.05 vs 0.5 PPQ blocks differ";
    EXPECT_EQ(serialize(small), serialize(large)) << "Kotekan: 0.05 vs 2.0 PPQ blocks differ";
}

// --- Test 35: Timing offset deterministic across block sizes ---
TEST(GoldenTimingOffset, BlockSizeIndependence) {
    poly::Engine engine;
    auto state = makeTestState();
    state.lanes[0].timingOffsetMs = 5.0f;
    state.lanes[1].timingOffsetMs = -3.0f;
    state.lanes[2].timingOffsetMs = 2.0f;

    auto small = renderSorted(engine, state, 0.0, 16.0, 0.05);
    auto medium = renderSorted(engine, state, 0.0, 16.0, 0.5);
    auto large = renderSorted(engine, state, 0.0, 16.0, 2.0);

    EXPECT_EQ(serialize(small), serialize(medium)) << "Timing offset: 0.05 vs 0.5 PPQ blocks differ";
    EXPECT_EQ(serialize(small), serialize(large)) << "Timing offset: 0.05 vs 2.0 PPQ blocks differ";
}
