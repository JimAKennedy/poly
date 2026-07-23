#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

#include "poly/engine.h"
#include "poly/types.h"

// Regression test for the M049 S02 / E2 finding from the 2026-07-16 product review.
//
// Bug: `buildLanePattern` builds a fixedPattern of length
//   patLen = fixedPatternLength > 0 ? fixedPatternLength : cycle.steps
// (engine.cpp:84), but the render loop wraps the cycle at `cfg.cycle.steps`
// (computeDriftedCycleStep engine.cpp:348, classifyStep :144, cyclePpqLen :285).
// A lane with `timeline=true`, `fixedPatternLength=10`, `cycle.steps=16`,
// all 10 pattern slots ON therefore plays a 16-step cycle with 10 events
// followed by 6 silent steps. That contradicts the field's own comment
// ("0 = use cycle.steps; >0 = explicit length"), the plugin's
// TimelineStepEditorView (which shows exactly patLen editable slots), and
// the WebUI test invariant `pattern.length === steps` in
// webui/tests/startup.spec.mjs.
//
// Fix: when `timeline && fixedPatternLength > 0`, treat fixedPatternLength as
// the authoritative cycle length for wrap, step-in-cycle, and cyclePpqLen.

namespace {

poly::LaneConfig makeTimelineLane(int cycleSteps, int fixedLen) {
    poly::LaneConfig cfg{};
    cfg.id = 0;
    cfg.midiNote = 36;
    cfg.cycle = {.steps = cycleSteps, .subdivision = 16};
    cfg.hitCount = 0;
    cfg.baseVelocity = 100;
    cfg.probability = 1.0f;
    cfg.velocitySpread = 0.0f;
    cfg.emphasisProb = 0.0f;
    cfg.ghostFloor = 0;
    cfg.active = true;
    cfg.timeline = true;
    cfg.fixedPatternLength = fixedLen;
    const int fillLen = fixedLen > 0 ? fixedLen : cycleSteps;
    for (int i = 0; i < fillLen; ++i)
        cfg.fixedPattern[i] = true;
    return cfg;
}

std::vector<poly::NoteEvent> renderOneBar(const poly::LaneConfig& cfg) {
    poly::Engine engine;
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;
    state.lanes[0] = cfg;

    poly::NoteEventBuffer buf;
    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 4.0; // one 4/4 bar
    tc.tempo = 120.0;
    tc.sampleRate = 48000.0;
    tc.blockSize = 512;
    tc.playing = true;

    engine.renderRange(tc, state, buf);

    std::vector<poly::NoteEvent> events;
    for (size_t i = 0; i < buf.count; ++i)
        events.push_back(buf.events[i]);
    return events;
}

double stepPpq(const poly::LaneConfig& cfg) {
    // Same math as engine::stepPpq: 4 PPQ per bar / subdivisions per bar.
    // With cycle.subdivision = 16 → 0.25 PPQ per step.
    return 4.0 / static_cast<double>(cfg.cycle.subdivision);
}

} // namespace

// --- The bug: fixedPatternLength=10, cycle.steps=16 ---

TEST(FixedPatternWrap, PatternLengthOverridesCycleStepsForWrap) {
    // 10-step pattern, 16-step cycle. All 10 slots ON. One bar = 16 steps
    // at 1/16 subdivision.
    //
    // On unfixed engine: buildLanePattern fills slots 0..9 with true, 10..15
    // stay false (silent tail). Render loop wraps at cycle.steps=16 so we
    // see 10 events (positions 0..9), then 6 silent steps, then wrap.
    //
    // On fixed engine: cycle wraps at patLen=10 so we see events at
    // positions 0, 1, 2, ..., 9, 10 (wrap to slot 0), 11 (slot 1), ...
    // 16 total events in one bar.
    auto cfg = makeTimelineLane(/*cycleSteps=*/16, /*fixedLen=*/10);
    auto events = renderOneBar(cfg);

    // Unfixed: 10 events. Fixed: 16 events. Assert fixed.
    EXPECT_EQ(events.size(), 16u) << "fixedPatternLength must govern cycle wrap in timeline mode";

    // The 11th event (index 10) must land at PPQ position 10 * stepPpq — the
    // wrap point. On the unfixed engine, this event doesn't exist.
    ASSERT_GE(events.size(), 11u);
    const double s = stepPpq(cfg);
    EXPECT_NEAR(events[10].ppqPosition, 10.0 * s, 1e-6)
        << "11th event should land at step 10 (the pattern-length wrap boundary)";
}

// --- Control: fixedPatternLength == cycle.steps (all 4 factory presets) ---

TEST(FixedPatternWrap, FactoryPresetShapeUnchanged) {
    // fixedPatternLength == cycle.steps: pre-fix and post-fix behavior are
    // identical. This is the only shape any shipped factory preset uses.
    auto cfg = makeTimelineLane(/*cycleSteps=*/12, /*fixedLen=*/12);
    auto events = renderOneBar(cfg);
    // One bar = 4 PPQ / (0.25 PPQ per step) = 16 steps of playback,
    // wrapping the 12-step cycle: 12 + 4 = 16 events.
    EXPECT_EQ(events.size(), 16u);
    const double s = stepPpq(cfg);
    // Event 12 (index 12) is the first wrap — it should land at step 12
    // in absolute time (same as before the fix).
    ASSERT_GE(events.size(), 13u);
    EXPECT_NEAR(events[12].ppqPosition, 12.0 * s, 1e-6);
}

// --- Control: fixedPatternLength == 0 (default, no override) ---

TEST(FixedPatternWrap, DefaultUsesCycleSteps) {
    // fixedPatternLength == 0: fall back to cycle.steps, same as always.
    auto cfg = makeTimelineLane(/*cycleSteps=*/8, /*fixedLen=*/0);
    auto events = renderOneBar(cfg);
    // 8-step cycle at 1/16 sub, one bar = 16 steps → 2 full cycles = 16 events.
    EXPECT_EQ(events.size(), 16u);
    const double s = stepPpq(cfg);
    ASSERT_GE(events.size(), 9u);
    EXPECT_NEAR(events[8].ppqPosition, 8.0 * s, 1e-6);
}
