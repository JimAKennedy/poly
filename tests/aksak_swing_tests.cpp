// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
//
// M003/S02 (EC09). Cell-aware swing for additive (aksak) meters. Swing keys off
// (cycleStep % 2), which on a 2+2+3 lane running one step per unit displaces
// steps 1, 3 and 5 -- alternate notes across the bar, ignoring where the cells
// fall. The guide describes the feel as belonging to the cell: the long cell's
// internal division differs from the short cells'.
#include <array>

#include <gtest/gtest.h>

#include "poly/types.h"

using namespace poly;

namespace {

LaneConfig aksakLane() {
    LaneConfig cfg{};
    cfg.cycle = {7, 8};
    cfg.swingCellCount = 3;
    cfg.swingCellSizes = {2, 2, 3};
    return cfg;
}

} // namespace

// A 9/8 aksak lane running one step per unit: seven steps grouped 2+2+3.
TEST(AksakSwing, MapsStepsToCellsAndPositions) {
    const auto cfg = aksakLane();
    const std::array<int, 7> expectedCell = {0, 0, 1, 1, 2, 2, 2};
    const std::array<int, 7> expectedPos = {0, 1, 0, 1, 0, 1, 2};

    for (int step = 0; step < 7; ++step) {
        const auto info = swingCellFor(cfg, step, 7);
        ASSERT_TRUE(info.valid) << "step " << step << " must land in a cell";
        EXPECT_EQ(info.cell, expectedCell[static_cast<size_t>(step)]) << "step " << step;
        EXPECT_EQ(info.positionInCell, expectedPos[static_cast<size_t>(step)]) << "step " << step;
    }
}

// No grouping is the default, and must stay a no-op: every existing swung lane
// keeps keying off (cycleStep % 2).
TEST(AksakSwing, ReportsNoGroupingWhenUnset) {
    LaneConfig cfg{};
    cfg.cycle = {7, 8};
    for (int step = 0; step < 7; ++step)
        EXPECT_FALSE(swingCellFor(cfg, step, 7).valid) << "step " << step;
}

// Sizes that do not account for every step are a configuration error, not a
// licence to read past the end of the array.
TEST(AksakSwing, RejectsSizesThatDoNotSumToTheStepCount) {
    LaneConfig cfg = aksakLane();
    cfg.swingCellSizes = {2, 2, 2}; // sums to 6, not 7
    for (int step = 0; step < 7; ++step)
        EXPECT_FALSE(swingCellFor(cfg, step, 7).valid) << "step " << step;
}

// A step outside the cycle must not be mapped -- the caller's index is not
// trusted to be in range.
TEST(AksakSwing, RejectsAStepOutsideTheCycle) {
    const auto cfg = aksakLane();
    EXPECT_FALSE(swingCellFor(cfg, 7, 7).valid);
    EXPECT_FALSE(swingCellFor(cfg, -1, 7).valid);
}
