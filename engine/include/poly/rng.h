// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024-2026 Jim Kennedy
#pragma once

#include <cstdint>

namespace poly {

// Position-based deterministic hash. Same (seed, lane, step) always
// produces the same value regardless of block boundaries.
// Returns a value in [0.0, 1.0).
inline float deterministicRand(uint64_t seed, int laneId, int64_t absStep, uint32_t channel) {
    uint64_t h = seed + 0x9E3779B97F4A7C15ULL;
    h ^= static_cast<uint64_t>(laneId) * 0x517CC1B727220A95ULL;
    h ^= static_cast<uint64_t>(absStep) * 0x6C62272E07BB0142ULL;
    h ^= static_cast<uint64_t>(channel) * 0xBF58476D1CE4E5B9ULL;

    h ^= h >> 30;
    h *= 0xBF58476D1CE4E5B9ULL;
    h ^= h >> 27;
    h *= 0x94D049BB133111EBULL;
    h ^= h >> 31;

    return static_cast<float>(h >> 40) / static_cast<float>(1ULL << 24);
}

// M001 S02 (GP02). A correlated (1/f-ish) fluctuation for humanize.
//
// deterministicRand below is white noise: each step's draw is independent of
// the last. Human timing fluctuation is long-range correlated (Hennig et al.
// 2011) -- a drummer slightly late tends to stay slightly late for a stretch --
// and listeners can tell the difference. White-noise jitter is precisely what
// makes "humanized" MIDI sound fake.
//
// This sums a few octaves of value noise over the absolute step index: each
// octave interpolates between hashed lattice points, and the lower octaves
// change slowly, which is what produces the drift. Returns [-1, 1] so it
// substitutes for `rand * 2 - 1` at the humanize call site.
//
// Deterministic and stateless by construction: the value depends only on
// (seed, laneId, absStep), so a locate or a loop reproduces it exactly and no
// state accumulates between renderRange calls. Pure arithmetic, no allocation
// -- it runs on the audio thread.
inline float correlatedNoise(uint64_t seed, int laneId, int64_t absStep) {
    // One octave: hash the two lattice points either side of `pos` and
    // interpolate with a smoothstep, so the result varies continuously with
    // position rather than jumping per step.
    auto octave = [&](int64_t period, uint32_t channel) -> float {
        const int64_t cell = (absStep >= 0) ? (absStep / period) : ((absStep - period + 1) / period);
        const double frac = static_cast<double>(absStep - cell * period) / static_cast<double>(period);
        const float a = deterministicRand(seed, laneId, cell, channel) * 2.0f - 1.0f;
        const float b = deterministicRand(seed, laneId, cell + 1, channel) * 2.0f - 1.0f;
        const double smooth = frac * frac * (3.0 - 2.0 * frac);
        return static_cast<float>(static_cast<double>(a) + static_cast<double>(b - a) * smooth);
    };

    // Halving amplitude over lengthening periods: the 1/f shape. Weights sum to
    // 1.0 so the result stays inside [-1, 1] without a clamp hiding a bug.
    const float value =
        0.533f * octave(16, 21) + 0.267f * octave(8, 22) + 0.133f * octave(4, 23) + 0.067f * octave(2, 24);
    return value < -1.0f ? -1.0f : (value > 1.0f ? 1.0f : value);
}

} // namespace poly
