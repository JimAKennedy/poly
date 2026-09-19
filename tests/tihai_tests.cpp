// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
//
// M002/S03 (GP05). The tihai: a phrase repeated three times, calculated to land
// its final onset exactly on the target — sam, in Hindustani and Carnatic
// practice (Nelson 2008; Clayton 2000). 06-indian-classical's "Tihai: Landing
// on Sam" section asks the reader to solve 3P + 2g = remaining by hand.
#include <gtest/gtest.h>

#include "poly/tihai.h"

using namespace poly;

// The worked example the guide itself uses: three repetitions of a five-beat
// phrase with half-beat gaps fill one sixteen-beat tintal cycle.
TEST(Tihai, SolvesTheGuidesWorkedExample) {
    const TihaiSolution s = solveTihai(5.0, 16.0);
    ASSERT_TRUE(s.solvable);
    EXPECT_DOUBLE_EQ(s.gap, 0.5);
}

// The property, for arbitrary inputs: the third repetition's final onset lands
// exactly on the target.
TEST(Tihai, TheThirdRepetitionLandsOnTheTarget) {
    for (double phrase : {2.0, 3.5, 5.0, 7.25}) {
        for (double remaining : {12.0, 16.0, 24.0, 32.0}) {
            const TihaiSolution s = solveTihai(phrase, remaining);
            if (!s.solvable)
                continue;
            // Start, gap, phrase, gap, phrase, phrase -- the last onset is at
            // 3P + 2g from the start, which must be the target.
            EXPECT_DOUBLE_EQ(3.0 * phrase + 2.0 * s.gap, remaining)
                << "phrase " << phrase << ", remaining " << remaining;
        }
    }
}

// An unsolvable case is reported, not silently rounded. A phrase too long for
// the space has no tihai, and returning one anyway would place the third
// repetition past the target -- landing after sam, which is the one thing the
// figure must never do.
TEST(Tihai, ReportsUnsolvableRatherThanOverrunningTheTarget) {
    const TihaiSolution tooLong = solveTihai(6.0, 16.0); // 3*6 = 18 > 16
    EXPECT_FALSE(tooLong.solvable) << "gap would be " << tooLong.gap;

    const TihaiSolution exact = solveTihai(16.0 / 3.0, 16.0);
    EXPECT_TRUE(exact.solvable) << "three repetitions with no gap is a valid tihai";
    EXPECT_NEAR(exact.gap, 0.0, 1e-12);
}

TEST(Tihai, RejectsNonsenseInput) {
    EXPECT_FALSE(solveTihai(0.0, 16.0).solvable) << "a zero-length phrase is not a phrase";
    EXPECT_FALSE(solveTihai(-2.0, 16.0).solvable);
    EXPECT_FALSE(solveTihai(4.0, 0.0).solvable) << "no space to land in";
}
