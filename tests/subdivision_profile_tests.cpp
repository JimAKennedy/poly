// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
//
// M003/S01 (EC08). Non-isochronous subdivision profiles. theory-brazilian's
// Rule 6 describes samba's long-short-short-long feel and then admits, in the
// guide's own voice, that swing approximates it poorly because swing displaces
// only alternate notes. Neither existing mechanism can express it: cellSizes is
// int, and microTimingMs is absolute milliseconds, so a ratio encoded there
// holds only at the tempo it was measured at.
#include <array>
#include <cmath>

#include <gtest/gtest.h>

#include "poly/types.h"

using namespace poly;

namespace {

LaneConfig profiledLane(std::array<float, 4> entries) {
    LaneConfig cfg{};
    cfg.cycle = {4, 16};
    cfg.profileCount = 4;
    for (size_t i = 0; i < entries.size(); ++i)
        cfg.subdivisionProfile[i] = entries[i];
    return cfg;
}

} // namespace

// The shape Rule 6 states: first sixteenth slightly long, middle two
// compressed, fourth slightly long again.
TEST(SubdivisionProfile, PlacesStepsInTheStatedProportions) {
    const auto info = computeAdditiveCells(profiledLane({1.1f, 0.9f, 0.95f, 1.05f}));
    ASSERT_EQ(info.count, 4);

    const double base = 4.0 / 16.0;
    // Successive differences are the step durations; they must be in the
    // profile's proportions, and the first must be longer than the second.
    const double d0 = info.cumPpq[1] - info.cumPpq[0];
    const double d1 = info.cumPpq[2] - info.cumPpq[1];
    const double d2 = info.cumPpq[3] - info.cumPpq[2];
    const double d3 = info.totalPpq - info.cumPpq[3];

    // Tolerance is 1e-6, not tighter: the profile is stored as float by design,
    // so 1.1f / 0.9f differs from the double ratio at about 6e-8.
    EXPECT_NEAR(d0 / d1, 1.1 / 0.9, 1e-6);
    EXPECT_NEAR(d2 / d3, 0.95 / 1.05, 1e-6);
    EXPECT_GT(d0, d1);
    EXPECT_GT(d3, d2);
    // Normalisation: four steps still occupy four even steps.
    EXPECT_NEAR(info.totalPpq, 4 * base, 1e-9);
}

// A profile states distribution, never length. Scaling every entry by the same
// constant must change nothing at all -- without this, a profile summing to 4.2
// would make its lane 5% longer than every other lane and the ensemble would
// drift apart.
TEST(SubdivisionProfile, IsScaleInvariant) {
    const auto a = computeAdditiveCells(profiledLane({1.1f, 0.9f, 0.95f, 1.05f}));
    const auto b = computeAdditiveCells(profiledLane({2.2f, 1.8f, 1.9f, 2.1f}));
    ASSERT_EQ(a.count, b.count);
    for (int i = 0; i < a.count; ++i)
        EXPECT_NEAR(a.cumPpq[i], b.cumPpq[i], 1e-6) << "step " << i;
    EXPECT_NEAR(a.totalPpq, b.totalPpq, 1e-6);
}

// profileCount == 0 must leave the existing additive path byte-identical: the
// 2+2+3 davul of the Balkan preset is the case that matters.
TEST(SubdivisionProfile, LeavesIntegerAksakCellsUntouched) {
    LaneConfig cfg{};
    cfg.cycle = {3, 8};
    cfg.cellCount = 3;
    cfg.cellSizes = {2, 2, 3};

    const auto info = computeAdditiveCells(cfg);
    ASSERT_EQ(info.count, 3);
    const double base = 4.0 / 8.0;
    EXPECT_NEAR(info.cumPpq[0], 0.0, 1e-12);
    EXPECT_NEAR(info.cumPpq[1], 2 * base, 1e-12);
    EXPECT_NEAR(info.cumPpq[2], 4 * base, 1e-12);
    EXPECT_NEAR(info.totalPpq, 7 * base, 1e-12);
}

// A profile takes precedence over cellSizes. Silently combining two
// non-isochronies is the thing nobody can reason about a year later.
TEST(SubdivisionProfile, TakesPrecedenceOverCellSizes) {
    LaneConfig cfg = profiledLane({1.0f, 1.0f, 1.0f, 1.0f});
    cfg.cellCount = 3;
    cfg.cellSizes = {2, 2, 3};

    const auto info = computeAdditiveCells(cfg);
    EXPECT_EQ(info.count, 4) << "profileCount must win over cellCount";
    EXPECT_NEAR(info.totalPpq, 4 * (4.0 / 16.0), 1e-9);
}

// A profile that sums to nothing is not a feel, and dividing by it would put
// infinities into the timing path.
TEST(SubdivisionProfile, RejectsANonPositiveSum) {
    LaneConfig cfg = profiledLane({0.0f, 0.0f, 0.0f, 0.0f});
    const auto info = computeAdditiveCells(cfg);
    EXPECT_EQ(info.count, 0);
    EXPECT_NEAR(info.totalPpq, 0.0, 1e-12);
}
