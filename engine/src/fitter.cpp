// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#include "poly/fitter.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

#include "poly/euclidean.h"

namespace poly {

namespace {

// region:candidate-subdivisions
// Bounded search over musically sensible subdivisions (notes-per-bar). A step
// spans 4/subdivision PPQ, so this covers whole notes through 32nd-note grids
// plus the common triplet grids (3, 6, 12, 24) needed for tresillo / cinquillo
// spellings. Ordered ascending so, on an exact tie in fit cost, the coarsest
// (simplest) grid wins — a rhythm that fits E(3,8) must not be reported as the
// finer E(6,16) alias.
constexpr std::array<int, 10> kCandidateSubdivisions = {1, 2, 3, 4, 6, 8, 12, 16, 24, 32};
// endregion:candidate-subdivisions

// Cost added per step of Euclidean-skeleton/occupancy mismatch, scaled to the
// grid so it is commensurate with the squared onset-quantization error (one
// full grid slot of timing error ~= g*g). Keeps a clean on-grid Euclidean fit
// strictly cheaper than a mis-rotated or wrong-hit-count alternative.
double mismatchPenalty(double gridPpq) {
    return gridPpq * gridPpq;
}

// Micro-timing residuals are stored in the same field the engine reads
// (LaneConfig::microTimingMs), which the engine sanitizes to [-20, +20] ms
// before use. Clamp here so the fitted value the caller applies is the value
// the engine will actually play — a residual larger than the grid's own
// half-step is a sign the grid is coarse, not a timing nuance worth preserving.
constexpr double kMicroTimingClampMs = 20.0;

// PPQ is one quarter note; the engine converts a per-step ms offset to a PPQ
// shift via ms * tempo / 60000 (engine.cpp). This is the exact inverse: a grid
// deviation of d PPQ at `tempoBpm` becomes d * 60000 / tempoBpm ms, so applying
// the fitted micro-timing reproduces the original off-grid onset.
double ppqToMs(double deltaPpq, double tempoBpm) {
    return deltaPpq * (60000.0 / tempoBpm);
}

} // namespace

// region:fit-euclidean
FitResult fitEuclidean(const std::vector<double>& onsetsPpq, double loopLengthPpq, double tempoBpm) {
    FitResult result;

    // Degenerate input: 0 or 1 onset carries no rhythm to fit. Return an
    // explicitly-invalid safe default (a plain 4-on-the-floor-ish lane) so the
    // caller can fall back without branching on array contents.
    if (onsetsPpq.size() < 2 || loopLengthPpq <= 0.0) {
        return result; // valid == false, sensible defaults from FitResult
    }

    // Micro-timing is expressed in ms via the source tempo; a non-positive
    // tempo would divide by zero, so fall back to the documented default.
    if (tempoBpm <= 0.0)
        tempoBpm = 120.0;

    double bestCost = std::numeric_limits<double>::infinity();
    bool found = false;

    for (int subdivision : kCandidateSubdivisions) {
        const double g = 4.0 / static_cast<double>(subdivision);
        const double nRaw = loopLengthPpq / g;
        const int n = static_cast<int>(std::lround(nRaw));

        // Reject grids that cannot host a real cycle or that the loop length
        // does not (approximately) divide — a subdivision whose step size does
        // not tile the loop is not the grid this rhythm was written on.
        if (n < 2 || n > kMaxSteps)
            continue;
        if (std::abs(loopLengthPpq - static_cast<double>(n) * g) > 0.5 * g)
            continue;

        // Quantize each onset onto the grid, accumulating squared timing error
        // and marking per-step occupancy (wrapping into [0, n) so onsets in
        // later repeats of the loop still land on their step). Also accumulate
        // the signed sub-grid deviation per step so the residual micro-timing
        // (T02) can be recovered for the winning grid; when several onsets fall
        // on the same step (loop repeats), their deviations are averaged.
        std::array<bool, kMaxSteps> occupied{};
        std::array<double, kMaxSteps> deltaSumPpq{};
        std::array<int, kMaxSteps> deltaCount{};
        double quantErr = 0.0;
        for (double onset : onsetsPpq) {
            const long slot = std::lround(onset / g);
            const double snapped = static_cast<double>(slot) * g;
            const double d = onset - snapped;
            quantErr += d * d;
            int step = static_cast<int>(slot % n);
            if (step < 0)
                step += n;
            occupied[static_cast<size_t>(step)] = true;
            deltaSumPpq[static_cast<size_t>(step)] += d;
            deltaCount[static_cast<size_t>(step)] += 1;
        }

        int k = 0;
        for (int i = 0; i < n; ++i)
            k += occupied[static_cast<size_t>(i)] ? 1 : 0;
        if (k < 1 || k > n)
            continue;

        // Reuse the engine's Bjorklund oracle: find the rotation whose E(k,n,r)
        // best matches the observed occupancy (fewest disagreeing steps). Ties
        // resolve to the smallest rotation.
        int bestMismatch = std::numeric_limits<int>::max();
        int bestRotation = 0;
        std::array<bool, kMaxSteps> pattern{};
        for (int r = 0; r < n; ++r) {
            euclidean(k, n, r, pattern);
            int mismatch = 0;
            for (int i = 0; i < n; ++i) {
                if (pattern[static_cast<size_t>(i)] != occupied[static_cast<size_t>(i)])
                    ++mismatch;
            }
            if (mismatch < bestMismatch) {
                bestMismatch = mismatch;
                bestRotation = r;
                if (mismatch == 0)
                    break;
            }
        }

        const double cost = quantErr + mismatchPenalty(g) * static_cast<double>(bestMismatch);
        if (cost < bestCost) {
            bestCost = cost;
            found = true;
            result.valid = true;
            result.steps = n;
            result.subdivision = subdivision;
            result.hitCount = k;
            result.rotation = bestRotation;
            result.onsetError = quantErr;
            result.patternMismatch = bestMismatch;

            // Normalized nearest-Euclidean goodness-of-fit, derived purely from
            // the structural occupancy mismatch (NOT the timing residual). The
            // maximum number of steps a size-k Euclidean skeleton can disagree
            // with any occupancy on the same k-of-n grid is 2*min(k, n-k) (each
            // displaced hit costs one false-negative on its Euclidean slot and
            // one false-positive on the slot it moved to). Dividing by that
            // bound puts a fully-displaced pattern at 0 and a pure Euclidean
            // pattern (mismatch 0) at 100. A fully-occupied grid (k == n) has no
            // empty step for the skeleton to disagree on, so it is trivially a
            // pure Euclidean fit — special-cased to 100 to avoid 0/0.
            const int room = std::min(k, n - k);
            result.fitPercent =
                (room == 0) ? 100.0
                            : 100.0 * (1.0 - static_cast<double>(bestMismatch) / (2.0 * static_cast<double>(room)));

            // Residual capture (T02): the actual per-step occupancy of the
            // winning grid becomes a timeline `fixedPattern` — this preserves
            // extra/missing hits the maximally-even skeleton cannot host, so
            // re-rendering approximates the source rather than the nearest
            // textbook rhythm. Per-step micro-timing records how far each real
            // onset sat from its quantized grid slot, in ms.
            result.fixedPattern = occupied;
            result.fixedPatternLength = n;
            result.microTimingMs = std::array<float, kMaxSteps>{};
            for (int i = 0; i < n; ++i) {
                const size_t si = static_cast<size_t>(i);
                if (deltaCount[si] == 0)
                    continue;
                const double avgPpq = deltaSumPpq[si] / static_cast<double>(deltaCount[si]);
                const double ms = std::clamp(ppqToMs(avgPpq, tempoBpm), -kMicroTimingClampMs, kMicroTimingClampMs);
                result.microTimingMs[si] = static_cast<float>(ms);
            }
        }
    }

    // No candidate grid tiled the loop (e.g. an irrational loop length). Leave
    // the invalid safe default rather than reporting a bogus fit.
    if (!found)
        return FitResult{};

    return result;
}
// endregion:fit-euclidean

} // namespace poly
