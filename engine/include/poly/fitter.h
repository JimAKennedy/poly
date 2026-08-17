// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#pragma once

#include <array>
#include <vector>

#include "poly/types.h"

namespace poly {

// --- Euclidean parameter fitting (reverse Euclid, M035 S01) ---
//
// Offline import path: given the note-on onset positions extracted from one
// lane of a dropped MIDI loop, recover the Euclidean lane parameters (cycle
// length in steps, subdivision, hit count, rotation) whose maximally-even
// pattern best reproduces those onsets, chosen by least-squares onset-error
// minimization over a bounded search space.
//
// This module is deliberately NON-RT: it runs on the offline import path (file
// drop), never inside renderRange()/process(), so std::vector and other
// allocating operations are permitted. It has zero VST3/audio-thread
// dependencies (engine isolation, CLAUDE.md) and reuses the engine's own
// poly::euclidean() Bjorklund generator (M068 D016) as the candidate-pattern
// oracle, so a fitted rotation lines up with the phase the engine will play.

// region:fit-result
struct FitResult {
    // False for degenerate input (fewer than two onsets) — the caller should
    // fall back to a default lane rather than apply these fields.
    bool valid = false;

    // Fields sufficient to configure a LaneConfig: cycle.steps,
    // cycle.subdivision, hitCount, rotation.
    int steps = 4;
    int subdivision = 4; // 1=whole, 2=half, 4=quarter, 8=eighth, 16=sixteenth
    int hitCount = 1;
    int rotation = 0;

    // Total least-squares residual of the chosen fit, in PPQ^2. Zero for a pure
    // Euclidean rhythm sampled exactly on its grid; grows with off-grid timing
    // and extra/missing hits. Used to rank candidates and, in S01 T02, to derive
    // the residual timeline + micro-timing.
    double onsetError = 0.0;

    // Number of steps where the fitted Euclidean skeleton disagrees with the
    // actual per-step occupancy of the input. Zero means the onsets ARE a pure
    // Euclidean pattern. Non-zero drives the residual capture in S01 T02.
    int patternMismatch = 0;

    // Residual capture (S01 T02): what the maximally-even Euclidean skeleton
    // cannot express, recovered from the winning grid so re-rendering
    // approximates the source rather than the nearest textbook rhythm.
    //
    // fixedPattern / fixedPatternLength: the ACTUAL per-step occupancy of the
    // fitted grid (length = steps), suitable to drive a timeline-mode lane.
    // Unlike E(hitCount, steps, rotation), this preserves extra or missing hits
    // the skeleton has no room for. Empty (length 0) for degenerate input.
    //
    // microTimingMs: per-step signed offset, in ms at the source tempo, of the
    // real onset from its quantized grid slot (positive = late). Clamped to the
    // engine's [-20, +20] ms range (LaneConfig::microTimingMs); a residual that
    // would exceed that is a sign the grid is coarse, not a timing nuance. Zero
    // for on-grid steps and unoccupied steps.
    std::array<bool, kMaxSteps> fixedPattern{};
    int fixedPatternLength = 0;
    std::array<float, kMaxSteps> microTimingMs{};
};
// endregion:fit-result

// Fit Euclidean lane parameters to a set of onset PPQ positions.
//
// onsetsPpq  : note-on positions in PPQ (quarter-note units), measured relative
//              to the start of the loop (loop start = 0). Order-independent; the
//              fitter does not require them pre-sorted.
// loopLengthPpq : total length of the source loop in PPQ. This resolves the
//              cycle-length ambiguity that onsets alone cannot (trailing rests
//              carry no onset), and is known at import time from the dropped
//              file / selection length.
// tempoBpm   : source tempo, used only to express micro-timing residuals in ms
//              (S01 T02). Defaults to 120 BPM.
//
// Determinism: a given (onsetsPpq, loopLengthPpq, tempoBpm) always yields the
// same FitResult — no RNG, no nondeterminism beyond IEEE arithmetic — so
// downstream UAT is reproducible.
[[nodiscard]] FitResult fitEuclidean(const std::vector<double>& onsetsPpq, double loopLengthPpq,
                                     double tempoBpm = 120.0);

} // namespace poly
