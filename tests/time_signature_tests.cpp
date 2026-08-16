// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
// M051 S02 T04: regression tests for host time signature threading.
//
// These lock the four behaviors the S02 vision cares about:
//   1. Lane cycles remain meter-independent (a steps=7/subdivision=8 lane
//      cycles every 7/8 = 3.5 PPQ regardless of host meter).
//   2. Scene chain advances on host bar boundaries (3/4 = 3 PPQ per bar,
//      not 4).
//   3. Envelope period follows host meter (periodBars=2 at 3/4 = 6 PPQ,
//      not 8).
//   4. MIDI capture window follows host meter (extractLastBars(4) at 3/4
//      grabs the last 12 PPQ, not 16).

#include <gtest/gtest.h>

#include "poly/engine.h"
#include "poly/envelope.h"
#include "poly/midi_capture.h"
#include "poly/scene.h"
#include "poly/types.h"

namespace {

// -----------------------------------------------------------------------
// 1. Lane cycles are meter-independent.
//
// Runs the engine over the same lane config once at 4/4 and once at 7/8.
// Note-on positions relative to block start must be byte-identical: the
// only path that would leak host meter into lane math is stepPpq, and
// stepPpq is fixed at 4.0/subdivision by design.
// -----------------------------------------------------------------------
TEST(TimeSignature, LaneCyclesAreMeterIndependent) {
    poly::GrooveState state{};
    state.activeLaneCount = 1;
    auto& lane = state.lanes[0];
    lane.active = true;
    lane.id = 0;
    lane.midiNote = 36;
    lane.cycle = {.steps = 7, .subdivision = 8};
    lane.hitCount = 3;
    lane.rotation = 0;
    lane.probability = 1.0f;

    poly::TransportContext tc44{};
    tc44.ppqStart = 0.0;
    tc44.ppqEnd = 8.0;
    tc44.tempo = 120.0;
    tc44.playing = true;
    tc44.timeSigNumerator = 4;
    tc44.timeSigDenominator = 4;

    poly::TransportContext tc78 = tc44;
    tc78.timeSigNumerator = 7;
    tc78.timeSigDenominator = 8;

    poly::Engine e44;
    poly::Engine e78;
    poly::NoteEventBuffer buf44{};
    poly::NoteEventBuffer buf78{};

    e44.renderRange(tc44, state, buf44);
    e78.renderRange(tc78, state, buf78);

    ASSERT_EQ(buf44.count, buf78.count);
    for (size_t i = 0; i < buf44.count; ++i) {
        EXPECT_DOUBLE_EQ(buf44.events[i].ppqPosition, buf78.events[i].ppqPosition)
            << "Lane cycle should be meter-independent at event " << i;
        EXPECT_EQ(buf44.events[i].pitch, buf78.events[i].pitch);
    }
}

// -----------------------------------------------------------------------
// 2. Scene chain bar boundaries follow host meter.
//
// A chain entry with bars=2 crosses a boundary at PPQ 8.0 in 4/4 (2 bars
// × 4 PPQ) but at PPQ 6.0 in 3/4 (2 bars × 3 PPQ). Assert that reading
// the same ppqPosition sees a different chain state under different
// meters.
// -----------------------------------------------------------------------
TEST(TimeSignature, SceneChainBoundaryShiftsWithMeter) {
    poly::SceneChainConfig config{};
    config.entryCount = 2;
    config.entries[0] = {.scene = poly::SceneSelect::A, .bars = 2};
    config.entries[1] = {.scene = poly::SceneSelect::B, .bars = 2};
    config.mode = poly::ChainMode::Loop;

    // At 4/4: bar 0 starts at PPQ 0, bar 2 starts at PPQ 8.
    poly::SceneChainState state44;
    EXPECT_EQ(state44.update(config, 0.0, 4.0), poly::SceneSelect::A);
    EXPECT_EQ(state44.update(config, 4.0, 4.0), poly::SceneSelect::A); // still bar 1
    EXPECT_EQ(state44.update(config, 8.0, 4.0), poly::SceneSelect::B); // bar 2 = crossover

    // At 3/4: bar 0 starts at PPQ 0, bar 2 starts at PPQ 6.
    poly::SceneChainState state34;
    EXPECT_EQ(state34.update(config, 0.0, 3.0), poly::SceneSelect::A);
    EXPECT_EQ(state34.update(config, 3.0, 3.0), poly::SceneSelect::A); // still bar 1
    EXPECT_EQ(state34.update(config, 6.0, 3.0), poly::SceneSelect::B); // bar 2 at 6 PPQ, not 8

    // At PPQ 8 in 3/4 the chain has advanced further than the 4/4 case
    // saw at PPQ 8 — same absolute time, different bar count under a
    // different meter.
    poly::SceneChainState state34b;
    EXPECT_EQ(state34b.update(config, 0.0, 3.0), poly::SceneSelect::A);
    EXPECT_EQ(state34b.update(config, 8.0, 3.0), poly::SceneSelect::B); // bar 2.67 → still in B
}

// -----------------------------------------------------------------------
// 3. Envelope period follows host meter.
//
// periodBars=2 means "2 bars per LFO cycle." At 4/4 that's 8 PPQ; the
// half-cycle point (phase=0.5) sits at PPQ 4. At 3/4 the same
// configuration gives 6 PPQ per cycle, and phase=0.5 sits at PPQ 3.
// -----------------------------------------------------------------------
TEST(TimeSignature, EnvelopePeriodFollowsHostMeter) {
    constexpr double kEps = 1e-9;
    constexpr float periodBars = 2.0f;

    // 4/4: 2 bars = 8 PPQ, half-cycle at PPQ 4.
    EXPECT_NEAR(poly::computeEnvelopePhase(0.0, periodBars, 0.0f, 4.0), 0.0, kEps);
    EXPECT_NEAR(poly::computeEnvelopePhase(4.0, periodBars, 0.0f, 4.0), 0.5, kEps);
    EXPECT_NEAR(poly::computeEnvelopePhase(8.0, periodBars, 0.0f, 4.0), 0.0, kEps);

    // 3/4: 2 bars = 6 PPQ, half-cycle at PPQ 3.
    EXPECT_NEAR(poly::computeEnvelopePhase(0.0, periodBars, 0.0f, 3.0), 0.0, kEps);
    EXPECT_NEAR(poly::computeEnvelopePhase(3.0, periodBars, 0.0f, 3.0), 0.5, kEps);
    EXPECT_NEAR(poly::computeEnvelopePhase(6.0, periodBars, 0.0f, 3.0), 0.0, kEps);

    // 7/8: 2 bars = 7 PPQ, half-cycle at PPQ 3.5.
    EXPECT_NEAR(poly::computeEnvelopePhase(3.5, periodBars, 0.0f, 3.5), 0.5, kEps);

    // Default arg (no ppqPerBar passed) matches 4/4 — locks the back-compat
    // contract that engine tests and hosts without a published time sig
    // still see the historical behavior.
    EXPECT_NEAR(poly::computeEnvelopePhase(4.0, periodBars, 0.0f), 0.5, kEps);
}

// -----------------------------------------------------------------------
// 4. MIDI capture window follows host meter.
//
// A user asking for "the last 4 bars" of capture should get a 16 PPQ
// window at 4/4 but a 12 PPQ window at 3/4. Push events at PPQ 0..20
// and assert the meter-dependent slice.
// -----------------------------------------------------------------------
TEST(TimeSignature, CaptureWindowFollowsHostMeter) {
    poly::MidiCaptureBuffer buf;
    for (int i = 0; i <= 20; ++i) {
        poly::NoteEvent ev{};
        ev.ppqPosition = static_cast<double>(i);
        ev.pitch = static_cast<int16_t>(60 + i);
        ev.velocity = 100.0f;
        buf.push(ev);
    }
    // newestPpq is now 20.0.

    poly::NoteEvent out[64];

    // 4/4 — 4 bars = 16 PPQ. Window: [20 - 16, 20 + 1) = [4, 21).
    // Events at PPQ 4..20 (inclusive lower, exclusive upper) → 17 events.
    size_t n44 = buf.extractLastBars(4, out, 64, 4.0);
    EXPECT_EQ(n44, 17u);
    EXPECT_DOUBLE_EQ(out[0].ppqPosition, 4.0);
    EXPECT_DOUBLE_EQ(out[n44 - 1].ppqPosition, 20.0);

    // 3/4 — 4 bars = 12 PPQ. Window: [20 - 12, 20 + 1) = [8, 21).
    // Events at PPQ 8..20 → 13 events. The meter-driven narrowing (17 → 13)
    // is the invariant this test locks.
    size_t n34 = buf.extractLastBars(4, out, 64, 3.0);
    EXPECT_EQ(n34, 13u);
    EXPECT_DOUBLE_EQ(out[0].ppqPosition, 8.0);
    EXPECT_DOUBLE_EQ(out[n34 - 1].ppqPosition, 20.0);
    EXPECT_LT(n34, n44); // 3/4 window is strictly narrower than 4/4 for the same "N bars"

    // Default arg matches 4/4 — back-compat contract.
    size_t nDefault = buf.extractLastBars(4, out, 64);
    EXPECT_EQ(nDefault, 17u);
}

// -----------------------------------------------------------------------
// TransportContext::ppqPerBar() sanity — the helper the plugin threads
// through all real call sites. Golden values for the meters the plugin's
// sanitizer allows.
// -----------------------------------------------------------------------
TEST(TimeSignature, PpqPerBarHelper) {
    poly::TransportContext tc{};

    tc.timeSigNumerator = 4;
    tc.timeSigDenominator = 4;
    EXPECT_DOUBLE_EQ(tc.ppqPerBar(), 4.0);

    tc.timeSigNumerator = 3;
    tc.timeSigDenominator = 4;
    EXPECT_DOUBLE_EQ(tc.ppqPerBar(), 3.0);

    tc.timeSigNumerator = 7;
    tc.timeSigDenominator = 8;
    EXPECT_DOUBLE_EQ(tc.ppqPerBar(), 3.5);

    tc.timeSigNumerator = 9;
    tc.timeSigDenominator = 8;
    EXPECT_DOUBLE_EQ(tc.ppqPerBar(), 4.5);

    tc.timeSigNumerator = 5;
    tc.timeSigDenominator = 16;
    EXPECT_DOUBLE_EQ(tc.ppqPerBar(), 1.25);
}

} // namespace
