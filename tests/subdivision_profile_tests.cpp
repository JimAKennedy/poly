// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
//
// M003/S01 (EC08). Non-isochronous subdivision profiles. theory-brazilian's
// Rule 6 describes samba's long-short-short-long feel and then admits, in the
// guide's own voice, that swing approximates it poorly because swing displaces
// only alternate notes. Neither existing mechanism can express it: cellSizes is
// int, and microTimingMs is absolute milliseconds, so a ratio encoded there
// holds only at the tempo it was measured at.
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include "poly/engine.h"
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

namespace {

// A single-lane groove that strikes every step, so every onset is observable.
GrooveState fourSixteenthLane(std::array<float, 4> profile, float swing) {
    GrooveState state{};
    state.activeLaneCount = 1;
    auto& lane = state.lanes[0];
    lane.id = 0;
    lane.cycle = {4, 16};
    lane.hitCount = 4;
    lane.probability = 1.0f;
    lane.baseVelocity = 100;
    lane.humanizeMs = 0.0f;
    lane.swingAmount = swing;
    if (profile[0] > 0.0f) {
        lane.profileCount = 4;
        for (size_t i = 0; i < profile.size(); ++i)
            lane.subdivisionProfile[i] = profile[i];
    }
    return state;
}

std::vector<double> onsets(const GrooveState& state, double tempo) {
    Engine engine;
    NoteEventBuffer notes;
    TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 1.0; // one beat: exactly one four-sixteenth cycle
    tc.tempo = tempo;
    tc.playing = true;
    engine.renderRange(tc, state, notes, nullptr);

    std::vector<double> out;
    for (size_t i = 0; i < notes.count; ++i)
        out.push_back(notes.events[i].ppqPosition);
    std::sort(out.begin(), out.end());
    return out;
}

std::vector<double> intervals(const std::vector<double>& xs) {
    std::vector<double> d;
    for (size_t i = 1; i < xs.size(); ++i)
        d.push_back(xs[i] - xs[i - 1]);
    return d;
}

} // namespace

// EC08's stated verification: a profiled lane's onsets differ from the
// isochronous grid AND from the swung grid. The swing arm is there because
// swing is precisely the workaround theory-brazilian admits to.
TEST(SubdivisionProfile, DiffersFromBothTheEvenGridAndTheSwungGrid) {
    const auto even = onsets(fourSixteenthLane({0.0f, 0.0f, 0.0f, 0.0f}, 0.0f), 120.0);
    const auto swung = onsets(fourSixteenthLane({0.0f, 0.0f, 0.0f, 0.0f}, 0.2f), 120.0);
    const auto profiled = onsets(fourSixteenthLane({1.1f, 0.9f, 0.95f, 1.05f}, 0.0f), 120.0);

    ASSERT_EQ(even.size(), 4u);
    ASSERT_EQ(profiled.size(), 4u);
    ASSERT_EQ(swung.size(), 4u);

    bool differsFromEven = false;
    bool differsFromSwing = false;
    for (size_t i = 0; i < profiled.size(); ++i) {
        if (std::abs(profiled[i] - even[i]) > 1e-6)
            differsFromEven = true;
        if (std::abs(profiled[i] - swung[i]) > 1e-6)
            differsFromSwing = true;
    }
    EXPECT_TRUE(differsFromEven) << "a profile that reproduces the even grid is not a profile";
    EXPECT_TRUE(differsFromSwing) << "if swing already did this, Rule 6's workaround would not be an approximation";
}

// The property that distinguishes a profile from microTimingMs: it is a ratio,
// so it survives a tempo change. Absolute milliseconds do not.
TEST(SubdivisionProfile, HoldsItsRatiosAcrossTempi) {
    const auto slow = intervals(onsets(fourSixteenthLane({1.1f, 0.9f, 0.95f, 1.05f}, 0.0f), 90.0));
    const auto fast = intervals(onsets(fourSixteenthLane({1.1f, 0.9f, 0.95f, 1.05f}, 0.0f), 140.0));
    ASSERT_EQ(slow.size(), 3u);
    ASSERT_EQ(fast.size(), slow.size());
    // Assert the intervals are uneven first. Without this the case is vacuous:
    // an even grid also holds its ratios across tempi, so it would pass with
    // the profile ignored entirely -- which is how it behaved before this line
    // was added, under a probe that discarded the profile in prepareLaneContext.
    EXPECT_GT(std::abs(slow[0] - slow[1]), 1e-6) << "the profile is not producing uneven intervals";
    for (size_t i = 0; i < slow.size(); ++i)
        EXPECT_NEAR(slow[i], fast[i], 1e-9) << "interval " << i << " moved with tempo";
}

// Normalisation, measured through the engine rather than the helper: a profile
// summing to more than its count must not lengthen the lane's cycle.
TEST(SubdivisionProfile, DoesNotLengthenTheCycle) {
    const auto even = onsets(fourSixteenthLane({0.0f, 0.0f, 0.0f, 0.0f}, 0.0f), 120.0);
    const auto big = onsets(fourSixteenthLane({2.2f, 1.8f, 1.9f, 2.1f}, 0.0f), 120.0);
    const auto small = onsets(fourSixteenthLane({1.1f, 0.9f, 0.95f, 1.05f}, 0.0f), 120.0);

    ASSERT_EQ(big.size(), even.size());
    ASSERT_EQ(small.size(), big.size());
    // Same guard as above: comparing two profiles to each other passes when
    // both are ignored, so pin that they are doing something first.
    bool movedFromEven = false;
    for (size_t i = 0; i < big.size(); ++i)
        if (std::abs(big[i] - even[i]) > 1e-6)
            movedFromEven = true;
    EXPECT_TRUE(movedFromEven) << "neither profile displaced anything from the even grid";

    for (size_t i = 0; i < big.size(); ++i)
        EXPECT_NEAR(big[i], small[i], 1e-6) << "step " << i << ": scale changed the placement";
    // The cycle still starts where an even one would: normalisation preserves
    // length, so the profile cannot push the next cycle late.
    EXPECT_NEAR(big[0], even[0], 1e-9);
    const double evenSpan = even[3] - even[0];
    const double bigSpan = big[3] - big[0];
    EXPECT_LT(std::abs(bigSpan - evenSpan), 0.75 * evenSpan) << "the profile changed the cycle's span";
}

// The lookahead bound must cover the longest *profiled* step, not the base
// step. Rendering a range in pieces must produce exactly what rendering it
// whole produces -- if maxTimingShift under-covers, a displaced onset near a
// block boundary is emitted by neither block.
TEST(SubdivisionProfile, SplitRenderingMatchesWholeRendering) {
    const auto state = fourSixteenthLane({1.1f, 0.9f, 0.95f, 1.05f}, 0.2f);

    Engine whole;
    NoteEventBuffer wholeBuf;
    TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = 1.0;
    tc.tempo = 120.0;
    tc.playing = true;
    whole.renderRange(tc, state, wholeBuf, nullptr);
    std::vector<double> expected;
    for (size_t i = 0; i < wholeBuf.count; ++i)
        expected.push_back(wholeBuf.events[i].ppqPosition);
    std::sort(expected.begin(), expected.end());
    ASSERT_FALSE(expected.empty());

    // Sweep the split point across the cycle so no single lucky boundary hides
    // an under-covered window.
    for (int k = 1; k < 20; ++k) {
        const double cut = static_cast<double>(k) / 20.0;
        Engine engine;
        std::vector<double> got;
        for (auto range : {std::pair<double, double>{0.0, cut}, std::pair<double, double>{cut, 1.0}}) {
            NoteEventBuffer buf;
            TransportContext part{};
            part.ppqStart = range.first;
            part.ppqEnd = range.second;
            part.tempo = 120.0;
            part.playing = true;
            engine.renderRange(part, state, buf, nullptr);
            for (size_t i = 0; i < buf.count; ++i)
                got.push_back(buf.events[i].ppqPosition);
        }
        std::sort(got.begin(), got.end());
        ASSERT_EQ(got.size(), expected.size()) << "split at " << cut << " changed the onset count";
        for (size_t i = 0; i < got.size(); ++i)
            EXPECT_NEAR(got[i], expected[i], 1e-9) << "split at " << cut << ", onset " << i;
    }
}
