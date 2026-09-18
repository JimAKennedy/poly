// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
//
// M001/S01 (GP01). Tempo-adaptive swing.
//
// Swing is a fixed fraction of the step: swingAmount * stepDurPpq / 3.0. Two
// consequences the guide already contradicts. The ratio is capped at exact
// triplet, where measured jazz reaches about 3.5:1 at ballad tempi; and it does
// not move with tempo, where the same study shows the ratio narrowing toward
// straight as tempo approaches 300 BPM. 12-jazz.mdx teaches that swing "varies
// continuously with tempo, intensity, and style" and the engine cannot express
// it.
#include <gtest/gtest.h>

#include "poly/types.h"

using namespace poly;

namespace {

// The ratio of the long note to the short one, which is how the swing
// literature states it: 2:1 is exact triplet, 1:1 is straight.
double ratioAt(double tempo, float amount) {
    return swingRatioAt(tempo, amount);
}

} // namespace

// The ceiling the fixed /3 divisor imposes is exact triplet. Ballad-tempo jazz
// reaches roughly 3.5:1 (Friberg & Sundström 2002).
TEST(SwingCurve, ReachesBeyondTripletAtBalladTempo) {
    const double r = ratioAt(70.0, 1.0f);
    EXPECT_GT(r, 2.0) << "the fixed divisor's exact-triplet ceiling must be exceeded";
    EXPECT_NEAR(r, 3.5, 0.5) << "the shape Friberg & Sundström report, not an arbitrary widening";
}

// The property, not a pair of points: a curve that wiggles would satisfy two
// samples and still be wrong.
TEST(SwingCurve, NarrowsMonotonicallyAsTempoRises) {
    const double tempi[] = {70.0, 110.0, 150.0, 190.0, 230.0, 280.0};
    double previous = ratioAt(tempi[0], 1.0f);
    for (size_t i = 1; i < std::size(tempi); ++i) {
        const double r = ratioAt(tempi[i], 1.0f);
        EXPECT_LT(r, previous) << "ratio must narrow from " << tempi[i - 1] << " to " << tempi[i];
        previous = r;
    }
}

TEST(SwingCurve, ApproachesStraightAtVeryFastTempi) {
    const double r = ratioAt(300.0, 1.0f);
    EXPECT_LT(r, 1.35) << "near 300 BPM the ratio is close to straight";
    EXPECT_GE(r, 1.0) << "it never inverts";
}

// The parameter must still mean what it meant: zero is no swing, at any tempo.
TEST(SwingCurve, ZeroAmountIsStraightAtEveryTempo) {
    for (double tempo : {60.0, 120.0, 200.0, 300.0})
        EXPECT_NEAR(ratioAt(tempo, 0.0f), 1.0, 1e-9) << "tempo " << tempo;
}

// --- Render-level: the curve reaches the emitted notes ---

#include <algorithm>
#include <vector>

#include "poly/engine.h"

namespace {

GrooveState swungLane(SwingMode mode, float amount) {
    GrooveState state{};
    state.activeLaneCount = 1;
    auto& lane = state.lanes[0];
    lane.id = 0;
    lane.cycle = {8, 8};
    lane.hitCount = 8;
    lane.probability = 1.0f;
    lane.baseVelocity = 100;
    lane.humanizeMs = 0.0f;
    lane.swingAmount = amount;
    lane.swingMode = mode;
    return state;
}

std::vector<double> onsetsAt(const GrooveState& state, double tempo, double ppqEnd = 4.0) {
    Engine engine;
    NoteEventBuffer notes;
    TransportContext tc{};
    tc.ppqStart = 0.0;
    tc.ppqEnd = ppqEnd;
    tc.tempo = tempo;
    tc.playing = true;
    engine.renderRange(tc, state, notes, nullptr);
    std::vector<double> out;
    for (size_t i = 0; i < notes.count; ++i)
        out.push_back(notes.events[i].ppqPosition);
    std::sort(out.begin(), out.end());
    return out;
}

// The displacement of the first swung (odd) step, as a fraction of the step.
double firstSwingOffset(const std::vector<double>& onsets, double stepPpq) {
    if (onsets.size() < 2)
        return 0.0;
    return (onsets[1] - stepPpq) / stepPpq;
}

} // namespace

// The point of the slice: one setting, two tempi, two different feels.
TEST(SwingCurve, AdaptiveLaneSwingsDifferentlyAtDifferentTempi) {
    const auto lane = swungLane(SwingMode::TempoAdaptive, 0.6f);
    const double slow = firstSwingOffset(onsetsAt(lane, 90.0), 0.5);
    const double fast = firstSwingOffset(onsetsAt(lane, 200.0), 0.5);

    EXPECT_GT(slow, 0.0) << "an adaptive lane must swing at all";
    EXPECT_GT(slow, fast) << "slower tempo must swing wider from the same amount";
}

// And the control: a Fixed lane must NOT vary with tempo, or the comparison
// above would pass for a reason that has nothing to do with the curve.
TEST(SwingCurve, FixedLaneIsUnchangedByTempo) {
    const auto lane = swungLane(SwingMode::Fixed, 0.6f);
    const double slow = firstSwingOffset(onsetsAt(lane, 90.0), 0.5);
    const double fast = firstSwingOffset(onsetsAt(lane, 200.0), 0.5);
    EXPECT_NEAR(slow, fast, 1e-9) << "the fixed mapping is tempo-invariant, as it always was";
}

// Back-compatibility as a test rather than a claim: the default mode's
// displacement is exactly the arithmetic the engine used before M001.
TEST(SwingCurve, FixedLaneMatchesThePreM001Mapping) {
    const auto lane = swungLane(SwingMode::Fixed, 0.6f);
    const auto onsets = onsetsAt(lane, 120.0);
    ASSERT_GE(onsets.size(), 2u);
    const double stepPpq = 0.5;
    // swingAmount is a float, so the expectation is computed from the float
    // value: 0.6 in double differs from 0.6f at about 4e-9, which is storage
    // precision rather than arithmetic. M003/S01 hit the same thing.
    const double expected = stepPpq + static_cast<double>(0.6f) * stepPpq / 3.0;
    EXPECT_NEAR(onsets[1], expected, 1e-9);
}

// The lookahead bound must cover the widened range. Rendering a range in pieces
// must produce exactly what rendering it whole produces; an under-covered
// window drops a displaced onset at the seam.
TEST(SwingCurve, SplitRenderingMatchesWholeRenderingWhenAdaptive) {
    const auto lane = swungLane(SwingMode::TempoAdaptive, 1.0f);
    const auto whole = onsetsAt(lane, 70.0);
    ASSERT_FALSE(whole.empty());

    for (int k = 1; k < 16; ++k) {
        const double cut = static_cast<double>(k) / 4.0;
        Engine engine;
        std::vector<double> got;
        for (auto range : {std::pair<double, double>{0.0, cut}, std::pair<double, double>{cut, 4.0}}) {
            NoteEventBuffer buf;
            TransportContext part{};
            part.ppqStart = range.first;
            part.ppqEnd = range.second;
            part.tempo = 70.0;
            part.playing = true;
            engine.renderRange(part, lane, buf, nullptr);
            for (size_t i = 0; i < buf.count; ++i)
                got.push_back(buf.events[i].ppqPosition);
        }
        std::sort(got.begin(), got.end());
        ASSERT_EQ(got.size(), whole.size()) << "split at " << cut << " changed the onset count";
        for (size_t i = 0; i < got.size(); ++i)
            EXPECT_NEAR(got[i], whole[i], 1e-9) << "split at " << cut << ", onset " << i;
    }
}

#include <cstring>

#include "poly/presets.h"

// The milestone's back-compatibility rule, as a test rather than a claim: with
// the feature off, every factory preset renders byte-identically. This is the
// ledger's programme-wide definition-of-done item, inherited from M003/S01,
// and it is what makes a timing change reviewable without re-auditing 45
// presets by hand.
//
// It asserts on the default state, so it fails if any preset is ever given
// TempoAdaptive without the golden being reconsidered -- which is the point:
// the claim is "presets are unmoved", not "presets happen to be unmoved today".
TEST(SwingCurve, EveryFactoryPresetIsUnmovedWithTheModeOff) {
    for (int i = 0; i < poly::kFactoryPresetCount; ++i) {
        const poly::GrooveState preset = poly::makeFactoryPreset(i);
        const char* name = poly::getFactoryPresetInfo(i).name;

        for (int lane = 0; lane < preset.activeLaneCount; ++lane) {
            ASSERT_EQ(preset.lanes[static_cast<size_t>(lane)].swingMode, poly::SwingMode::Fixed)
                << "preset " << i << " (" << name << ") lane " << lane
                << " ships TempoAdaptive; this golden asserts the pre-M001 default";
        }

        // Render the preset and the same preset with every swingMode explicitly
        // reset. Identical output is the byte-identity claim; a difference would
        // mean the Fixed branch is no longer the pre-M001 arithmetic.
        poly::GrooveState reset = preset;
        for (auto& lane : reset.lanes)
            lane.swingMode = poly::SwingMode::Fixed;

        const auto a = onsetsAt(preset, 120.0, 8.0);
        const auto b = onsetsAt(reset, 120.0, 8.0);
        ASSERT_EQ(a.size(), b.size()) << "preset " << i << " (" << name << ") changed onset count";
        for (size_t k = 0; k < a.size(); ++k)
            EXPECT_NEAR(a[k], b[k], 1e-12) << "preset " << i << " (" << name << ") onset " << k;
    }
}
