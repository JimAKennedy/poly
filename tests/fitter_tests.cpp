// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
//
// M035 S01 T01: reverse-Euclid fitter — recover (steps, subdivision, hitCount,
// rotation) from a lane's onset PPQ positions. Test suite name is FitEuclidean
// so `ctest -R FitEuclidean` selects exactly these cases.
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include "poly/engine.h"
#include "poly/euclidean.h"
#include "poly/fitter.h"
#include "poly/types.h"

namespace {

// Generate the onset PPQ positions a pure Euclidean rhythm E(k,n) at the given
// subdivision and rotation would produce over exactly one cycle, with the cycle
// starting at PPQ 0 (the same convention the fitter assumes).
std::vector<double> euclideanOnsets(int k, int n, int rotation, int subdivision) {
    std::array<bool, poly::kMaxSteps> pattern{};
    poly::euclidean(k, n, rotation, pattern);
    const double g = 4.0 / static_cast<double>(subdivision);
    std::vector<double> onsets;
    for (int i = 0; i < n; ++i) {
        if (pattern[static_cast<size_t>(i)])
            onsets.push_back(static_cast<double>(i) * g);
    }
    return onsets;
}

// PPQ length of one full cycle of n steps at the given subdivision.
double loopLength(int n, int subdivision) {
    return static_cast<double>(n) * (4.0 / static_cast<double>(subdivision));
}

// Round-trip a known Euclidean triple through the fitter and assert exact
// recovery. All spellings used here are rotationally aperiodic (gcd(k,n)==1),
// so the rotation is recovered uniquely.
void expectRoundTrip(int k, int n, int rotation, int subdivision) {
    const auto onsets = euclideanOnsets(k, n, rotation, subdivision);
    const poly::FitResult fit = poly::fitEuclidean(onsets, loopLength(n, subdivision));

    EXPECT_TRUE(fit.valid);
    EXPECT_EQ(fit.steps, n) << "k=" << k << " n=" << n << " r=" << rotation << " sub=" << subdivision;
    EXPECT_EQ(fit.subdivision, subdivision) << "k=" << k << " n=" << n << " r=" << rotation;
    EXPECT_EQ(fit.hitCount, k) << "k=" << k << " n=" << n << " r=" << rotation;
    EXPECT_EQ(fit.rotation, rotation) << "k=" << k << " n=" << n << " r=" << rotation;
    EXPECT_EQ(fit.patternMismatch, 0);
    EXPECT_DOUBLE_EQ(fit.onsetError, 0.0);
}

} // namespace

// --- Round-trip: canonical spellings at multiple rotations ---

TEST(FitEuclidean, Tresillo_3_8_AllRotations) {
    for (int r = 0; r < 8; ++r)
        expectRoundTrip(3, 8, r, 8);
}

TEST(FitEuclidean, Cinquillo_5_8_AllRotations) {
    for (int r = 0; r < 8; ++r)
        expectRoundTrip(5, 8, r, 8);
}

TEST(FitEuclidean, Five_16_AllRotations) {
    for (int r = 0; r < 16; ++r)
        expectRoundTrip(5, 16, r, 16);
}

TEST(FitEuclidean, Seven_12_AllRotations) {
    for (int r = 0; r < 12; ++r)
        expectRoundTrip(7, 12, r, 12);
}

// --- Degenerate / negative input (Q7) ---

TEST(FitEuclidean, EmptyInputReturnsInvalidDefault) {
    const poly::FitResult fit = poly::fitEuclidean({}, 4.0);
    EXPECT_FALSE(fit.valid);
    // Safe, documented default — usable as a plain fallback lane, never a crash.
    EXPECT_GE(fit.steps, 1);
    EXPECT_GE(fit.hitCount, 1);
    EXPECT_EQ(fit.fixedPatternLength, 0);
}

TEST(FitEuclidean, SingleOnsetReturnsInvalidDefault) {
    const poly::FitResult fit = poly::fitEuclidean({1.5}, 4.0);
    EXPECT_FALSE(fit.valid);
    EXPECT_GE(fit.steps, 1);
}

TEST(FitEuclidean, NonPositiveLoopLengthReturnsInvalid) {
    const poly::FitResult fit = poly::fitEuclidean({0.0, 1.5, 3.0}, 0.0);
    EXPECT_FALSE(fit.valid);
}

// --- Best-fit for non-pure-Euclidean input ---

TEST(FitEuclidean, OffGridTimingRecoversSkeletonWithResidualError) {
    // Tresillo E(3,8), but the middle hit is nudged 0.05 PPQ off its grid slot.
    // The Euclidean skeleton (steps=8, hits=3, rot=0) still matches occupancy
    // exactly; the perturbation surfaces only as a non-zero onset residual.
    auto onsets = euclideanOnsets(3, 8, 0, 8); // {0.0, 1.5, 3.0}
    onsets[1] += 0.05;                         // 1.55

    const poly::FitResult fit = poly::fitEuclidean(onsets, loopLength(8, 8));

    EXPECT_TRUE(fit.valid);
    EXPECT_EQ(fit.steps, 8);
    EXPECT_EQ(fit.hitCount, 3);
    EXPECT_EQ(fit.rotation, 0);
    EXPECT_EQ(fit.patternMismatch, 0);
    EXPECT_GT(fit.onsetError, 0.0);
}

TEST(FitEuclidean, ExtraHitStillReturnsValidBestFit) {
    // Add an off-pattern onset (a hit the Euclidean skeleton cannot host). The
    // fitter must still return a valid best-fit rather than fail — the residual
    // capture (S01 T02) records what the skeleton misses.
    auto onsets = euclideanOnsets(3, 8, 0, 8); // {0.0, 1.5, 3.0}
    onsets.push_back(0.5);                     // extra on-grid hit at step 1

    const poly::FitResult fit = poly::fitEuclidean(onsets, loopLength(8, 8));

    EXPECT_TRUE(fit.valid);
    EXPECT_GE(fit.steps, 2);
    EXPECT_GE(fit.hitCount, 3);
}

// --- Residual capture: timeline occupancy + micro-timing (T02) ---

TEST(FitEuclidean, PureEuclideanPopulatesTimelineOccupancy) {
    // A pure Euclidean rhythm's timeline occupancy must equal its skeleton, and
    // an exactly-on-grid rhythm carries no micro-timing residual.
    std::array<bool, poly::kMaxSteps> skeleton{};
    poly::euclidean(5, 16, 3, skeleton);
    const auto onsets = euclideanOnsets(5, 16, 3, 16);

    const poly::FitResult fit = poly::fitEuclidean(onsets, loopLength(16, 16));

    ASSERT_TRUE(fit.valid);
    EXPECT_EQ(fit.fixedPatternLength, 16);
    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(fit.fixedPattern[static_cast<size_t>(i)], skeleton[static_cast<size_t>(i)]) << "step " << i;
        EXPECT_FLOAT_EQ(fit.microTimingMs[static_cast<size_t>(i)], 0.0f) << "step " << i;
    }
}

TEST(FitEuclidean, OffGridTimingPopulatesMicroTiming) {
    // Tresillo E(3,8) onsets {0.0, 1.5, 3.0} at step {0,3,6}; nudge the middle
    // hit +0.02 PPQ late. At 120 BPM one quarter note (1 PPQ) is 500 ms, so the
    // residual is +0.02 * 500 = +10 ms on step 3; the on-grid steps stay 0.
    auto onsets = euclideanOnsets(3, 8, 0, 8);
    onsets[1] += 0.02;

    const poly::FitResult fit = poly::fitEuclidean(onsets, loopLength(8, 8), 120.0);

    ASSERT_TRUE(fit.valid);
    EXPECT_EQ(fit.steps, 8);
    EXPECT_EQ(fit.fixedPatternLength, 8);
    EXPECT_NEAR(fit.microTimingMs[3], 10.0f, 1e-3f);
    EXPECT_FLOAT_EQ(fit.microTimingMs[0], 0.0f);
    EXPECT_FLOAT_EQ(fit.microTimingMs[6], 0.0f);
}

TEST(FitEuclidean, MicroTimingScalesWithTempoAndSign) {
    // The same +0.02 PPQ deviation is twice as many ms at half the tempo; an
    // early onset yields a negative offset.
    auto onsets = euclideanOnsets(3, 8, 0, 8);
    onsets[1] -= 0.02; // early → negative

    const poly::FitResult fit = poly::fitEuclidean(onsets, loopLength(8, 8), 60.0);

    ASSERT_TRUE(fit.valid);
    // 0.02 PPQ * (60000/60) = 20 ms, negated for the early nudge.
    EXPECT_NEAR(fit.microTimingMs[3], -20.0f, 1e-3f);
}

TEST(FitEuclidean, MicroTimingClampedToEngineRange) {
    // A deviation whose ms exceeds the engine's [-20,+20] micro-timing range is
    // clamped, matching what the engine would sanitize the applied value to.
    auto onsets = euclideanOnsets(3, 8, 0, 8);
    onsets[1] += 0.05; // 0.05 * 500 = 25 ms > 20 ms cap

    const poly::FitResult fit = poly::fitEuclidean(onsets, loopLength(8, 8), 120.0);

    ASSERT_TRUE(fit.valid);
    EXPECT_FLOAT_EQ(fit.microTimingMs[3], 20.0f);
}

TEST(FitEuclidean, TimelineOccupancyRecordsActualOnsetsOnWinningGrid) {
    // The timeline `fixedPattern` must record the ACTUAL per-step occupancy of
    // whatever grid the fitter selects — including an extra hit the maximally-
    // even skeleton cannot host — so a timeline-mode re-render reproduces the
    // source. Grid-agnostic: recompute occupancy on the winning (subdivision,
    // steps) and compare, rather than assuming which grid wins (adding a fourth
    // on-grid hit can make a coarser full grid the cheaper Occam fit).
    auto onsets = euclideanOnsets(3, 8, 0, 8); // steps {0,3,6}
    onsets.push_back(0.5);                     // extra on-grid hit

    const poly::FitResult fit = poly::fitEuclidean(onsets, loopLength(8, 8));

    ASSERT_TRUE(fit.valid);
    EXPECT_EQ(fit.fixedPatternLength, fit.steps);

    const double g = 4.0 / static_cast<double>(fit.subdivision);
    std::array<bool, poly::kMaxSteps> expected{};
    for (double onset : onsets) {
        const long slot = std::lround(onset / g);
        int step = static_cast<int>(slot % fit.steps);
        if (step < 0)
            step += fit.steps;
        expected[static_cast<size_t>(step)] = true;
    }
    for (int i = 0; i < fit.steps; ++i)
        EXPECT_EQ(fit.fixedPattern[static_cast<size_t>(i)], expected[static_cast<size_t>(i)]) << "step " << i;

    // Occupancy population equals the fitted hit count.
    int occ = 0;
    for (int i = 0; i < fit.steps; ++i)
        occ += fit.fixedPattern[static_cast<size_t>(i)] ? 1 : 0;
    EXPECT_EQ(occ, fit.hitCount);
    EXPECT_GT(occ, 0);
}

TEST(FitEuclidean, DegenerateInputLeavesResidualEmpty) {
    // The invalid safe default must carry no timeline/micro-timing residual.
    const poly::FitResult fit = poly::fitEuclidean({1.5}, 4.0);
    EXPECT_FALSE(fit.valid);
    EXPECT_EQ(fit.fixedPatternLength, 0);
    for (int i = 0; i < poly::kMaxSteps; ++i) {
        EXPECT_FALSE(fit.fixedPattern[static_cast<size_t>(i)]);
        EXPECT_FLOAT_EQ(fit.microTimingMs[static_cast<size_t>(i)], 0.0f);
    }
}

// --- Determinism ---

TEST(FitEuclidean, DeterministicAcrossRepeatedCalls) {
    const auto onsets = euclideanOnsets(5, 16, 3, 16);
    const double len = loopLength(16, 16);

    const poly::FitResult a = poly::fitEuclidean(onsets, len);
    const poly::FitResult b = poly::fitEuclidean(onsets, len);

    EXPECT_EQ(a.valid, b.valid);
    EXPECT_EQ(a.steps, b.steps);
    EXPECT_EQ(a.subdivision, b.subdivision);
    EXPECT_EQ(a.hitCount, b.hitCount);
    EXPECT_EQ(a.rotation, b.rotation);
    EXPECT_DOUBLE_EQ(a.onsetError, b.onsetError);
    EXPECT_EQ(a.patternMismatch, b.patternMismatch);
    EXPECT_EQ(a.fixedPatternLength, b.fixedPatternLength);
    EXPECT_EQ(a.fixedPattern, b.fixedPattern);
    EXPECT_EQ(a.microTimingMs, b.microTimingMs);
}

// --- T03: round-trip render proof through the real engine ---
//
// T01/T02 assert the returned struct fields directly. This section closes the
// loop end-to-end: it applies a FitResult to a LaneConfig, renders that lane
// through the real poly::Engine (Euclidean mode — hitCount/steps/rotation, NOT
// the timeline fixedPattern), and asserts the rendered note-on PPQ positions
// match the original source onsets. That proves the fit is *usable* — the
// engine's own poly::euclidean() phase lines up with the fitted rotation
// (D016), so the recovered parameters actually replay the source rhythm.

namespace {

// Configure a single-lane GrooveState from a FitResult in Euclidean mode and
// render exactly one cycle, returning the sorted note-on PPQ positions. All
// position-perturbing lane features (swing, humanize, mutation, drift,
// micro-timing, syncopation) are left at their zero defaults so the rendered
// onsets are the pure Euclidean grid the fit describes.
std::vector<double> renderFitOnsets(const poly::FitResult& fit, double loopLengthPpq) {
    poly::LaneConfig cfg{};
    cfg.id = 0;
    cfg.midiNote = 36;
    cfg.cycle = {.steps = fit.steps, .subdivision = fit.subdivision};
    cfg.hitCount = fit.hitCount;
    cfg.rotation = fit.rotation;
    cfg.probability = 1.0f; // every fitted hit fires — no probabilistic culling
    cfg.baseVelocity = 100;
    cfg.velocitySpread = 0.0f;
    cfg.emphasisProb = 0.0f;
    cfg.ghostFloor = 0;
    cfg.timeline = false; // Euclidean mode: prove rotation phase, not timeline replay
    cfg.active = true;

    poly::GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 42;
    state.lanes[0] = cfg;

    poly::TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = loopLengthPpq; // exactly one cycle
    tc.tempo = 120.0;
    tc.sampleRate = 48000.0;
    tc.blockSize = 512;
    tc.playing = true;

    poly::Engine engine;
    poly::NoteEventBuffer buf;
    engine.renderRange(tc, state, buf);

    std::vector<double> onsets;
    for (size_t i = 0; i < buf.count; ++i)
        onsets.push_back(buf.events[i].ppqPosition);
    std::sort(onsets.begin(), onsets.end());
    return onsets;
}

// Fit a known Euclidean triple, render the fit back through the engine, and
// assert the rendered onsets reproduce the source onsets exactly (within IEEE
// tolerance). Uses aperiodic spellings so rotation is unambiguous.
void expectRenderRoundTrip(int k, int n, int rotation, int subdivision) {
    const auto source = euclideanOnsets(k, n, rotation, subdivision);
    const double len = loopLength(n, subdivision);
    const poly::FitResult fit = poly::fitEuclidean(source, len);
    ASSERT_TRUE(fit.valid) << "k=" << k << " n=" << n << " r=" << rotation;

    const auto rendered = renderFitOnsets(fit, len);

    auto expected = source;
    std::sort(expected.begin(), expected.end());
    ASSERT_EQ(rendered.size(), expected.size())
        << "rendered onset count must match source; k=" << k << " n=" << n << " r=" << rotation;
    for (size_t i = 0; i < expected.size(); ++i)
        EXPECT_NEAR(rendered[i], expected[i], 1e-6)
            << "onset " << i << " mismatch; k=" << k << " n=" << n << " r=" << rotation << " sub=" << subdivision;
}

} // namespace

TEST(FitEuclidean, RenderRoundTripTresillo_3_8) {
    for (int r = 0; r < 8; ++r)
        expectRenderRoundTrip(3, 8, r, 8);
}

TEST(FitEuclidean, RenderRoundTripCinquillo_5_8) {
    for (int r = 0; r < 8; ++r)
        expectRenderRoundTrip(5, 8, r, 8);
}

TEST(FitEuclidean, RenderRoundTripFive_16) {
    for (int r = 0; r < 16; ++r)
        expectRenderRoundTrip(5, 16, r, 16);
}

TEST(FitEuclidean, RenderRoundTripSeven_12) {
    for (int r = 0; r < 12; ++r)
        expectRenderRoundTrip(7, 12, r, 12);
}
