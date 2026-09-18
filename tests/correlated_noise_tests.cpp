// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
//
// M001/S02 (GP02). Correlated humanize.
//
// humanizeMs displaces each hit by an independent seeded draw, which is white
// noise. Human timing fluctuation is long-range correlated (Hennig et al.
// 2011): a drummer slightly late tends to stay slightly late for a stretch,
// drifting rather than jittering. Listeners distinguish the two, and
// white-noise jitter is what makes humanized MIDI sound fake.
//
// The cases below pin CORRELATION, which is the whole point and the one thing
// an eyeball cannot check on a list of offsets.
#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include "poly/rng.h"

using namespace poly;

namespace {

std::vector<float> series(uint64_t seed, int laneId, int count, int stride = 1) {
    std::vector<float> out;
    out.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i)
        out.push_back(correlatedNoise(seed, laneId, static_cast<int64_t>(i) * stride));
    return out;
}

double meanAbsDelta(const std::vector<float>& xs) {
    if (xs.size() < 2)
        return 0.0;
    double total = 0.0;
    for (size_t i = 1; i < xs.size(); ++i)
        total += std::abs(static_cast<double>(xs[i]) - static_cast<double>(xs[i - 1]));
    return total / static_cast<double>(xs.size() - 1);
}

} // namespace

// The case that distinguishes this from the white noise it replaces. Adjacent
// samples must differ by materially less than distant ones; for white noise the
// two figures are the same, so this is the test that would fail if the helper
// were quietly pointed back at deterministicRand.
TEST(CorrelatedNoise, AdjacentSamplesAreCloserThanDistantOnes) {
    const double adjacent = meanAbsDelta(series(1234, 0, 2048, 1));
    const double distant = meanAbsDelta(series(1234, 0, 2048, 997));

    EXPECT_GT(distant, adjacent * 1.5)
        << "adjacent mean|delta| = " << adjacent << ", distant = " << distant
        << " — this is white noise, not a correlated drift";
}

// It has to scale humanizeMs the way the draw it replaces does.
TEST(CorrelatedNoise, StaysWithinUnitRange) {
    for (const float v : series(99, 3, 4096)) {
        ASSERT_GE(v, -1.0f);
        ASSERT_LE(v, 1.0f);
    }
}

// Determinism, which #151 states as a requirement: the offsets must be a pure
// function of the absolute step index, so a locate or a loop reproduces them.
TEST(CorrelatedNoise, IsAPureFunctionOfTheStepIndex) {
    for (int64_t step : {int64_t{0}, int64_t{7}, int64_t{1013}, int64_t{-42}})
        EXPECT_FLOAT_EQ(correlatedNoise(5, 2, step), correlatedNoise(5, 2, step)) << "step " << step;
}

// Two lanes on one seed must not drift together, or an ensemble breathes in
// lockstep — which is a different artefact from the one being fixed.
TEST(CorrelatedNoise, LanesWithTheSameSeedDoNotMoveTogether) {
    const auto laneA = series(7, 0, 512);
    const auto laneB = series(7, 1, 512);
    int identical = 0;
    for (size_t i = 0; i < laneA.size(); ++i)
        if (std::abs(laneA[i] - laneB[i]) < 1e-6f)
            ++identical;
    EXPECT_LT(identical, 16) << identical << " of 512 samples coincide across lanes";
}
