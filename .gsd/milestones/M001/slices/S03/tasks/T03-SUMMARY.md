---
id: T03
parent: S03
milestone: M001
key_files:
  - engine/include/poly/rng.h
  - engine/src/engine.cpp
key_decisions:
  - splitmix64-style hash for position-based RNG
  - Channel parameter separates probability from velocity randomness
  - No state between blocks — pure function of (seed, lane, step, channel)
duration: 
verification_result: passed
completed_at: 2026-06-20T20:04:13.674Z
blocker_discovered: false
---

# T03: Implemented position-based deterministic RNG for probability gating and velocity spread

**Implemented position-based deterministic RNG for probability gating and velocity spread**

## What Happened

Added deterministicRand() in rng.h using splitmix64-style hashing keyed on (seed, laneId, absStep, channel). Channel 0 drives probability gating, channel 1 drives velocity spread. Integrated into renderRange() — hits are filtered when roll >= probability, velocity varies by +/- spread. Verified block-size independence: sorted event sets identical across 0.5, 0.1, and 0.05 PPQ block sizes with RNG active. No accumulator state — pure function of position.

## Verification

Sorted diff across three block sizes (0.5, 0.1, 0.05 PPQ) shows identical event sets. Velocity varies per event. Probability filtering visibly drops ~10% of hi-hat hits (p=0.9).

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `poly_harness 2 120 (0.5|0.1|0.05) | sort | diff` | 0 | Identical event sets across block sizes with RNG | 80ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/include/poly/rng.h`
- `engine/src/engine.cpp`
