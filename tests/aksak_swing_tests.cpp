// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
//
// M003/S02 (EC09). Cell-aware swing for additive (aksak) meters. Swing keys off
// (cycleStep % 2), which on a 2+2+3 lane running one step per unit displaces
// steps 1, 3 and 5 -- alternate notes across the bar, ignoring where the cells
// fall. The guide describes the feel as belonging to the cell: the long cell's
// internal division differs from the short cells'.
#include <algorithm>
#include <array>
#include <vector>

#include <gtest/gtest.h>

#include "poly/engine.h"
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

namespace {

// A 9/8 lane running one step per unit and striking every one, so every onset
// is observable -- the zurna of the Balkan preset has exactly this shape.
GrooveState nineEightLane(bool grouped, float swing) {
    GrooveState state{};
    state.activeLaneCount = 1;
    auto& lane = state.lanes[0];
    lane.id = 0;
    lane.cycle = {7, 8};
    lane.hitCount = 7;
    lane.probability = 1.0f;
    lane.baseVelocity = 100;
    lane.humanizeMs = 0.0f;
    lane.swingAmount = swing;
    if (grouped) {
        lane.swingCellCount = 3;
        lane.swingCellSizes = {2, 2, 3};
    }
    return state;
}

std::vector<double> renderOnsets(const GrooveState& state) {
    Engine engine;
    NoteEventBuffer notes;
    TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 3.5; // seven eighth notes
    tc.tempo = 120.0;
    tc.playing = true;
    engine.renderRange(tc, state, notes, nullptr);
    std::vector<double> out;
    for (size_t i = 0; i < notes.count; ++i)
        out.push_back(notes.events[i].ppqPosition);
    std::sort(out.begin(), out.end());
    return out;
}

// Which steps moved relative to the same lane with no swing at all.
std::vector<int> displacedSteps(bool grouped) {
    const auto plain = renderOnsets(nineEightLane(grouped, 0.0f));
    const auto swung = renderOnsets(nineEightLane(grouped, 0.2f));
    std::vector<int> moved;
    for (size_t i = 0; i < plain.size() && i < swung.size(); ++i)
        if (std::abs(swung[i] - plain[i]) > 1e-9)
            moved.push_back(static_cast<int>(i));
    return moved;
}

} // namespace

// Ungrouped, swing keys off (cycleStep % 2): steps 1, 3 and 5 -- alternate
// notes across the bar, ignoring where the 2+2+3 cells fall.
TEST(AksakSwing, WithoutAGroupingSwingAlternatesAcrossTheBar) {
    EXPECT_EQ(displacedSteps(false), (std::vector<int>{1, 3, 5}));
}

// Grouped, swing belongs to the cell: the odd positions *within* each cell,
// which for 2+2+3 are steps 1, 3, 5 and 6.
TEST(AksakSwing, WithAGroupingSwingDisplacesWithinEachCell) {
    // Issue #157: "the final subdivision of each 2- or 3-group". Cells are
    // {0,1}, {2,3}, {4,5,6}, so the tails are steps 1, 3 and 6 -- not 5, which
    // is what bar parity displaces.
    EXPECT_EQ(displacedSteps(true), (std::vector<int>{1, 3, 6}));
}

// The slice's second definition-of-done clause, stated as a measurement: the
// three-unit cell divides internally in a way the two-unit cells do not.
TEST(AksakSwing, TheLongCellDividesDifferentlyFromTheShortCells) {
    const auto onsets = renderOnsets(nineEightLane(true, 0.2f));
    ASSERT_EQ(onsets.size(), 7u);
    // Inter-onset intervals inside cell 0 (steps 0,1) and cell 2 (steps 4,5,6).
    const double shortCellInternal = onsets[1] - onsets[0];
    const double longCellFirst = onsets[5] - onsets[4];
    const double longCellSecond = onsets[6] - onsets[5];
    // The two-cell has one internal interval, stretched by the swing on its
    // tail. The three-cell divides into two, and they are not equal to each
    // other -- the first runs at its nominal length, the second is stretched.
    EXPECT_NEAR(longCellFirst, 0.5, 1e-9) << "the long cell's first division is unswung";
    EXPECT_GT(std::abs(longCellSecond - longCellFirst), 1e-9) << "the long cell must divide unevenly inside itself";
    EXPECT_NEAR(longCellSecond, shortCellInternal, 1e-9)
        << "the swung tail of every cell is stretched by the same amount";
    EXPECT_GT(std::abs(longCellFirst - shortCellInternal), 1e-9)
        << "the long cell's opening division must differ from the short cells'";
}

// An ungrouped swung lane must be byte-identical to what it played before this
// slice, so no existing patch moves.
TEST(AksakSwing, AnUngroupedLaneIsUnchanged) {
    const auto before = renderOnsets(nineEightLane(false, 0.2f));
    ASSERT_EQ(before.size(), 7u);
    const double base = 0.5; // one eighth at 4 ppq per bar
    for (size_t i = 0; i < before.size(); ++i) {
        const double nominal = static_cast<double>(i) * base;
        const double expected = (i % 2 == 1) ? nominal + 0.2 * base / 3.0 : nominal;
        EXPECT_NEAR(before[i], expected, 1e-9) << "step " << i;
    }
}

// The rachenitsa hides the bug: on 2+2+3 the tail rule and bar parity displace
// the same steps. On 2+3+2 they do not, which is the case that proves the
// grouping is doing anything at all.
TEST(AksakSwing, TheTailRuleDivergesFromBarParityOn232) {
    GrooveState grouped = nineEightLane(true, 0.2f);
    grouped.lanes[0].swingCellSizes = {2, 3, 2};
    GrooveState plain = nineEightLane(true, 0.0f);
    plain.lanes[0].swingCellSizes = {2, 3, 2};

    const auto a = renderOnsets(plain);
    const auto b = renderOnsets(grouped);
    std::vector<int> moved;
    for (size_t i = 0; i < a.size() && i < b.size(); ++i)
        if (std::abs(b[i] - a[i]) > 1e-9)
            moved.push_back(static_cast<int>(i));
    // Cells {0,1}, {2,3,4}, {5,6}: tails are 1, 4, 6. Bar parity would say 1, 3, 5.
    EXPECT_EQ(moved, (std::vector<int>{1, 4, 6}));
}
