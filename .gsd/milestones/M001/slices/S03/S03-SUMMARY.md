---
id: S03
parent: M001
milestone: M001
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - engine/include/poly/euclidean.h
  - engine/src/euclidean.cpp
  - engine/include/poly/rng.h
  - engine/src/engine.cpp
  - tests/euclidean_tests.cpp
  - tests/golden_tests.cpp
  - tests/golden/default_patch_4bars.txt
key_decisions:
  - Bresenham formula for Euclidean rhythms (equivalent to Bjorklund, simpler)
  - All phase from absolute PPQ — no accumulators
  - splitmix64-style position hash for deterministic RNG
  - Channel parameter separates probability/velocity randomness
  - Golden tests sort by PPQ+pitch (within-block lane ordering is non-deterministic by design)
patterns_established:
  - Position-based deterministic RNG pattern: deterministicRand(seed, lane, step, channel)
  - Golden test pattern: renderSorted() + serialize() for cross-scenario comparison
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-20T20:05:56.793Z
blocker_discovered: false
---

# S03: Timing Determinism Prototype

**Engine produces deterministic polymetric output proven by 18 tests covering block-size independence, loop restart, position jump, and tempo invariance**

## What Happened

Implemented the core timing engine in four tasks: (T01) Euclidean rhythm algorithm with Bresenham distribution and rotation, (T02) PPQ-based absolute-position cycle timing in renderRange(), (T03) position-seeded deterministic RNG for probability gating and velocity spread, (T04) 8-test golden determinism suite plus a checked-in reference file. The engine is a pure function of (patch, seed, transport position) with zero accumulated state. All phase computation derives from absolute PPQ — no drift under loop restarts, tempo changes, or position jumps. The polymetric 5/16 ghost lane correctly shows phase variation against the 4/4 grid.

## Verification

18/18 tests pass: 10 Euclidean algorithm tests + 8 golden determinism tests. Block-size independence verified across 0.05, 0.1, 0.2, 0.5, 1.0, 2.0 PPQ. Harness produces audible polymetric output (79 events over 4 bars).

## Requirements Advanced

None.

## Requirements Validated

None.

## New Requirements Surfaced

None.

## Requirements Invalidated or Re-scoped

None.

## Operational Readiness

None.

## Deviations

None.

## Known Limitations

None.

## Follow-ups

None.

## Files Created/Modified

None.
