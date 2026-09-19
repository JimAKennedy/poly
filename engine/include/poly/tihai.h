// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#pragma once

namespace poly {

// M002 S03 (GP05). A tihai is a phrase played three times, spaced so the third
// repetition's final onset lands exactly on the target — sam. The arithmetic is
// 3P + 2g = remaining, solved for the gap.
//
// 06-indian-classical's "Tihai: Landing on Sam" asks the reader to do this by
// hand. This is the whole of it: pure arithmetic, no engine state, so it can be
// tested without a transport and reused wherever the figure is constructed.
struct TihaiSolution {
    bool solvable = false;
    double gap = 0.0;
};

inline TihaiSolution solveTihai(double phraseLength, double remaining) {
    TihaiSolution s{};
    if (phraseLength <= 0.0 || remaining <= 0.0)
        return s;

    const double gap = (remaining - 3.0 * phraseLength) / 2.0;
    // A negative gap means the three repetitions do not fit: the third would
    // land past the target. Reporting that is the point — a figure that
    // overruns sam is worse than no figure, and silently rounding would hide
    // it.
    if (gap < 0.0)
        return s;

    s.solvable = true;
    s.gap = gap;
    return s;
}

} // namespace poly
