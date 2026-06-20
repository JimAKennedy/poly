#pragma once

#include <cstdint>

namespace poly {

// Position-based deterministic hash. Same (seed, lane, step) always
// produces the same value regardless of block boundaries.
// Returns a value in [0.0, 1.0).
inline float deterministicRand(uint64_t seed, int laneId, int64_t absStep, uint32_t channel) {
    // splitmix64-style mixing of the combined key
    uint64_t h = seed;
    h ^= static_cast<uint64_t>(laneId) * 0x9E3779B97F4A7C15ULL;
    h ^= static_cast<uint64_t>(absStep) * 0x517CC1B727220A95ULL;
    h ^= static_cast<uint64_t>(channel) * 0x6C62272E07BB0142ULL;

    h ^= h >> 30;
    h *= 0xBF58476D1CE4E5B9ULL;
    h ^= h >> 27;
    h *= 0x94D049BB133111EBULL;
    h ^= h >> 31;

    return static_cast<float>(h >> 40) / static_cast<float>(1ULL << 24);
}

} // namespace poly
