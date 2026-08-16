// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
// M034 S01 T01: Deterministic bar-gated fill activation tests.
//
// A lane with fillEveryNBars=N (N>0) plays off-pattern fill notes on every bar
// whose absolute bar index is a multiple of N, and its normal pattern on all
// other bars. fillEveryNBars=0 with no manual pulse is byte-identical to the
// pre-fill render path. A manual-fill pulse (GrooveState::fillManualTrigger)
// forces the current render pass to be a fill bar independent of fillEveryNBars.
// The gate is derived from the absolute PPQ bar index (never accumulated), so
// activation is deterministic and reproducible.

#include <array>
#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include "poly/engine.h"
#include "poly/types.h"

namespace {

// A single 8-step eighth-note lane in 4/4: 8 steps * 0.5 PPQ = 4.0 PPQ per bar,
// so one bar == one cycle. hitCount is small so most steps are off-pattern and
// a fill bar has clearly more notes than a normal bar.
poly::GrooveState makeSingleFillLane(int fillEveryNBars) {
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 0x1234;
    state.globalDensityCeiling = 0;

    poly::LaneConfig& lane = state.lanes[0];
    lane.id = 0;
    lane.active = true;
    lane.cycle.steps = 8;
    lane.cycle.subdivision = 8;
    lane.hitCount = 2;
    lane.rotation = 0;
    lane.probability = 1.0f;
    lane.baseVelocity = 100;
    lane.midiNote = 36;
    lane.fillEveryNBars = fillEveryNBars;
    return state;
}

// Render exactly one 4/4 bar starting at barIndex and return its NoteEvents.
std::vector<poly::NoteEvent> renderBarNotes(const poly::GrooveState& state, int barIndex) {
    poly::Engine engine;
    poly::NoteEventBuffer notes;

    poly::TransportContext tc{};
    tc.ppqStart = barIndex * 4.0;
    tc.ppqEnd = tc.ppqStart + 4.0;
    tc.tempo = 120.0;
    tc.playing = true;

    engine.renderRange(tc, state, notes, nullptr);

    std::vector<poly::NoteEvent> copy;
    copy.reserve(notes.count);
    for (size_t i = 0; i < notes.count; ++i)
        copy.push_back(notes.events[i]);
    return copy;
}

int countAddEmissions(const poly::GrooveState& state, int barIndex) {
    poly::Engine engine;
    poly::NoteEventBuffer notes;
    poly::EmissionEventBuffer emissions;

    poly::TransportContext tc{};
    tc.ppqStart = barIndex * 4.0;
    tc.ppqEnd = tc.ppqStart + 4.0;
    tc.tempo = 120.0;
    tc.playing = true;

    engine.renderRange(tc, state, notes, &emissions);

    int add = 0;
    for (size_t i = 0; i < emissions.count; ++i)
        if (static_cast<poly::EmissionKind>(emissions.events[i].kind) == poly::EmissionKind::Add)
            ++add;
    return add;
}

} // namespace

// The lane's normal (non-fill) bar count: fewer notes than steps, and > 0.
TEST(Fill, NormalBarPlaysOnlyPatternSteps) {
    auto state = makeSingleFillLane(/*fillEveryNBars=*/0);
    auto notes = renderBarNotes(state, /*barIndex=*/0);
    EXPECT_GT(notes.size(), 0u);
    EXPECT_LT(notes.size(), 8u); // off-pattern steps must be silent without fill
}

// fillEveryNBars=0 with no manual pulse must be byte-identical to the pre-fill
// path: the same notes at the same positions every render, on every bar.
TEST(Fill, ZeroEveryNBarsIsByteIdentical) {
    auto state = makeSingleFillLane(/*fillEveryNBars=*/0);
    for (int bar = 0; bar < 4; ++bar) {
        auto a = renderBarNotes(state, bar);
        auto b = renderBarNotes(state, bar);
        ASSERT_EQ(a.size(), b.size());
        for (size_t i = 0; i < a.size(); ++i) {
            EXPECT_DOUBLE_EQ(a[i].ppqPosition, b[i].ppqPosition);
            EXPECT_EQ(a[i].pitch, b[i].pitch);
            EXPECT_FLOAT_EQ(a[i].velocity, b[i].velocity);
        }
        // A non-fill lane never fills any bar.
        EXPECT_LT(a.size(), 8u);
    }
}

// fillEveryNBars=N: bars whose absolute index is a multiple of N are fill bars
// (all 8 steps fire), all other bars play the normal pattern.
TEST(Fill, EveryNBarsGatesOnAbsoluteBarIndex) {
    auto fillState = makeSingleFillLane(/*fillEveryNBars=*/2);
    auto normalState = makeSingleFillLane(/*fillEveryNBars=*/0);
    const size_t patternCount = renderBarNotes(normalState, 0).size();

    // Bar 0 (0 % 2 == 0) and bar 2 (2 % 2 == 0) are fill bars: all steps fire.
    EXPECT_EQ(renderBarNotes(fillState, 0).size(), 8u);
    EXPECT_EQ(renderBarNotes(fillState, 2).size(), 8u);
    // Bar 1 and bar 3 are normal bars: identical to the no-fill pattern count.
    EXPECT_EQ(renderBarNotes(fillState, 1).size(), patternCount);
    EXPECT_EQ(renderBarNotes(fillState, 3).size(), patternCount);
}

// The off-pattern notes added on a fill bar are classified StepOutcome::Add so
// the WebUI timeline renders them as off-grid adds (no new observability path).
TEST(Fill, FillNotesAreClassifiedAdd) {
    auto normalState = makeSingleFillLane(/*fillEveryNBars=*/0);
    auto fillState = makeSingleFillLane(/*fillEveryNBars=*/2);

    const int patternCount = static_cast<int>(renderBarNotes(normalState, 0).size());
    // Fill bar 0 adds (8 - patternCount) off-pattern notes as Add emissions.
    EXPECT_EQ(countAddEmissions(fillState, 0), 8 - patternCount);
    // Normal bar 1 adds nothing.
    EXPECT_EQ(countAddEmissions(fillState, 1), 0);
}

// The manual-fill momentary trigger forces the current render pass to be a fill
// bar independent of fillEveryNBars (which is 0 here), for one pass only.
TEST(Fill, ManualTriggerForcesFillBar) {
    auto state = makeSingleFillLane(/*fillEveryNBars=*/0);
    const size_t patternCount = renderBarNotes(state, 1).size();
    EXPECT_LT(patternCount, 8u);

    state.fillManualTrigger = true;
    // Bar 1 is not a bar-gated fill bar, but the manual pulse forces it.
    EXPECT_EQ(renderBarNotes(state, 1).size(), 8u);

    // Clearing the pulse restores normal behavior (momentary, not sticky).
    state.fillManualTrigger = false;
    EXPECT_EQ(renderBarNotes(state, 1).size(), patternCount);
}
