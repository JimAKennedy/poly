---
id: T02
parent: S02
milestone: M007
key_files:
  - engine/src/engine.cpp
key_decisions:
  - Used 3 mutation types (drop/ghost/add) weighted 40/30/30 for musical variety
  - RNG channels 8/9 avoid collision with existing channels 0-5
  - cycleIndex * kMaxSteps + cycleStep as hash key gives unique per-cycle-step values
duration: 
verification_result: passed
completed_at: 2026-06-22T19:48:23.329Z
blocker_discovered: false
---

# T02: Implemented deterministic per-cycle pattern mutation in renderRange

**Implemented deterministic per-cycle pattern mutation in renderRange**

## What Happened

After computing cycleStep and isPatternStep, added mutation logic gated on mutationRate > 0 and !isAnchor. Uses deterministicRand with cycleIndex-based keys (channels 8/9) for mutation chance and type selection. Three mutation types: Drop (40% — removes existing hit), Ghost (30% — reduces velocity to ghost floor), Add (30% — inserts hit where there was a rest). Mutations are deterministic given same (seed, laneId, cycleIndex, stepInCycle). Anchor steps are never mutated. Ghost mutation overrides velocity after all normal velocity processing.

## Verification

Build + all tests pass; mutationRate=0 produces byte-identical output to baseline (verified by GoldenMutation.ZeroRateMatchesBaseline test)

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build && ctest --test-dir build` | 0 | 201/201 tests pass | 9500ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/src/engine.cpp`
