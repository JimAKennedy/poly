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
