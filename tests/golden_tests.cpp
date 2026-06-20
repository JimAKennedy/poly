#include "poly/engine.h"
#include "poly/types.h"
#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

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
        if (ppq != o.ppq) return ppq < o.ppq;
        if (pitch != o.pitch) return pitch < o.pitch;
        return velocity < o.velocity;
    }

    bool operator==(const EventRecord& o) const {
        return ppq == o.ppq && pitch == o.pitch &&
               velocity == o.velocity && duration == o.duration;
    }
};

std::vector<EventRecord> renderSorted(poly::Engine& engine,
                                       const poly::GrooveState& state,
                                       double ppqStart, double ppqEnd,
                                       double blockPpq) {
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
            all.push_back({buf.events[i].ppqPosition,
                           buf.events[i].pitch,
                           buf.events[i].velocity,
                           buf.events[i].duration});
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
TEST(GoldenDeterminism, SamePatchSameSeed) {
    poly::Engine engine;
    auto state = makeTestState();

    auto run1 = renderSorted(engine, state, 0.0, 16.0, 0.5);
    auto run2 = renderSorted(engine, state, 0.0, 16.0, 0.5);

    ASSERT_EQ(run1.size(), run2.size());
    EXPECT_EQ(serialize(run1), serialize(run2));
}

// --- Test 2: Different block sizes produce identical events ---
TEST(GoldenDeterminism, BlockSizeIndependence) {
    poly::Engine engine;
    auto state = makeTestState();

    auto small  = renderSorted(engine, state, 0.0, 16.0, 0.05);
    auto medium = renderSorted(engine, state, 0.0, 16.0, 0.5);
    auto large  = renderSorted(engine, state, 0.0, 16.0, 2.0);

    EXPECT_EQ(serialize(small), serialize(medium))
        << "0.05 vs 0.5 PPQ blocks differ";
    EXPECT_EQ(serialize(small), serialize(large))
        << "0.05 vs 2.0 PPQ blocks differ";
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
        if (e.ppq < 8.0) firstTwo.push_back(e);
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
    auto jumped  = renderSorted(engine, state, 8.0, 16.0, 0.5);

    EXPECT_EQ(serialize(straight), serialize(jumped))
        << "Position jump then continue differs from straight-through";
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
            at90.push_back({buf.events[i].ppqPosition,
                            buf.events[i].pitch,
                            buf.events[i].velocity,
                            buf.events[i].duration});
        }
        ppq = end;
    }
    std::sort(at90.begin(), at90.end());

    EXPECT_EQ(serialize(at120), serialize(at90))
        << "Tempo change affected PPQ positions";
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
        if (e.pitch == 45 && e.ppq < 4.0) bar0.push_back(std::fmod(e.ppq, 4.0));
        if (e.pitch == 45 && e.ppq >= 4.0 && e.ppq < 8.0) bar1.push_back(e.ppq - 4.0);
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
