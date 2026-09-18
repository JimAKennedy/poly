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

    EXPECT_GT(distant, adjacent * 1.5) << "adjacent mean|delta| = " << adjacent << ", distant = " << distant
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

// --- Render-level: the fluctuation reaches the emitted notes ---

#include <algorithm>
#include <cstring>

#include "poly/engine.h"
#include "poly/presets.h"

namespace {

GrooveState humanizedLane(HumanizeMode mode) {
    GrooveState state{};
    state.activeLaneCount = 1;
    state.seed = 4242;
    auto& lane = state.lanes[0];
    lane.id = 0;
    lane.cycle = {16, 16};
    lane.hitCount = 16;
    lane.probability = 1.0f;
    lane.baseVelocity = 100;
    lane.humanizeMs = 12.0f;
    lane.humanizeMode = mode;
    return state;
}

std::vector<double> renderOnsets(const GrooveState& state, double from, double to) {
    Engine engine;
    NoteEventBuffer notes;
    TransportContext tc{};
    tc.ppqStart = from;
    tc.ppqEnd = to;
    tc.tempo = 120.0;
    tc.playing = true;
    engine.renderRange(tc, state, notes, nullptr);
    std::vector<double> out;
    for (size_t i = 0; i < notes.count; ++i)
        out.push_back(notes.events[i].ppqPosition);
    std::sort(out.begin(), out.end());
    return out;
}

// Displacement of each onset from its nominal grid position.
std::vector<double> displacements(const std::vector<double>& onsets, double stepPpq) {
    std::vector<double> d;
    for (size_t i = 0; i < onsets.size(); ++i)
        d.push_back(onsets[i] - static_cast<double>(i) * stepPpq);
    return d;
}

double meanAbsDelta(const std::vector<double>& xs) {
    if (xs.size() < 2)
        return 0.0;
    double t = 0.0;
    for (size_t i = 1; i < xs.size(); ++i)
        t += std::abs(xs[i] - xs[i - 1]);
    return t / static_cast<double>(xs.size() - 1);
}

} // namespace

// The claim, measured through the public render path rather than the helper:
// a correlated lane's successive displacements move less between neighbours
// than a white-noise lane's do.
TEST(CorrelatedNoise, ACorrelatedLaneDriftsWhereAWhiteNoiseLaneJitters) {
    const double stepPpq = 0.25;
    const double drift =
        meanAbsDelta(displacements(renderOnsets(humanizedLane(HumanizeMode::Correlated), 0.0, 16.0), stepPpq));
    const double jitter =
        meanAbsDelta(displacements(renderOnsets(humanizedLane(HumanizeMode::WhiteNoise), 0.0, 16.0), stepPpq));

    EXPECT_GT(jitter, drift * 1.5) << "correlated mean|delta| = " << drift << ", white-noise = " << jitter;
}

// #151 requires determinism explicitly: the offsets must survive a transport
// jump, which is what an accumulating implementation would break.
TEST(CorrelatedNoise, ALocateReproducesTheSameDisplacements) {
    const auto lane = humanizedLane(HumanizeMode::Correlated);
    const auto whole = renderOnsets(lane, 4.0, 8.0);

    // Render an earlier range first on the same engine, then the range again:
    // the second pass must be identical to rendering it cold.
    Engine engine;
    NoteEventBuffer warm;
    TransportContext first{};
    first.ppqStart = 0.0;
    first.ppqEnd = 4.0;
    first.tempo = 120.0;
    first.playing = true;
    engine.renderRange(first, lane, warm, nullptr);

    NoteEventBuffer after;
    TransportContext second{};
    second.ppqStart = 4.0;
    second.ppqEnd = 8.0;
    second.tempo = 120.0;
    second.playing = true;
    engine.renderRange(second, lane, after, nullptr);

    std::vector<double> got;
    for (size_t i = 0; i < after.count; ++i)
        got.push_back(after.events[i].ppqPosition);
    std::sort(got.begin(), got.end());

    ASSERT_EQ(got.size(), whole.size());
    for (size_t i = 0; i < got.size(); ++i)
        EXPECT_NEAR(got[i], whole[i], 1e-12) << "onset " << i << " moved after a prior render";
}

// The milestone's back-compatibility rule, for this slice's field.
TEST(CorrelatedNoise, EveryFactoryPresetIsUnmovedWithTheModeOff) {
    for (int i = 0; i < poly::kFactoryPresetCount; ++i) {
        const poly::GrooveState preset = poly::makeFactoryPreset(i);
        const char* name = poly::getFactoryPresetInfo(i).name;

        for (int lane = 0; lane < preset.activeLaneCount; ++lane)
            ASSERT_EQ(preset.lanes[static_cast<size_t>(lane)].humanizeMode, poly::HumanizeMode::WhiteNoise)
                << "preset " << i << " (" << name << ") ships Correlated; this golden asserts the default";

        poly::GrooveState reset = preset;
        for (auto& lane : reset.lanes)
            lane.humanizeMode = poly::HumanizeMode::WhiteNoise;

        const auto a = renderOnsets(preset, 0.0, 8.0);
        const auto b = renderOnsets(reset, 0.0, 8.0);
        ASSERT_EQ(a.size(), b.size()) << "preset " << i << " (" << name << ")";
        for (size_t k = 0; k < a.size(); ++k)
            EXPECT_NEAR(a[k], b[k], 1e-12) << "preset " << i << " (" << name << ") onset " << k;
    }
}
