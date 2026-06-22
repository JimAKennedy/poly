---
id: S02
parent: M007
milestone: M007
provides:
  - mutationRate per lane
  - deterministic per-cycle pattern variation
requires:
  []
affects:
  []
key_files:
  - engine/include/poly/types.h
  - engine/src/engine.cpp
  - engine/include/poly/state_io.h
  - plugin/source/plugids.h
  - plugin/source/processor.cpp
  - plugin/source/controller.cpp
  - tests/golden_tests.cpp
key_decisions:
  - 3 mutation types weighted 40/30/30 (drop/ghost/add) for musical balance
  - RNG channels 8/9 with cycleIndex*kMaxSteps+cycleStep key for per-cycle determinism
  - State version bumped to 6; backward compatible with version 5 presets
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-22T19:48:56.174Z
blocker_discovered: false
---

# S02: Pattern Mutation

**Per-lane mutation rate introduces controlled, deterministic per-cycle variations (drops, ghosts, additions) to Euclidean patterns**

## What Happened

Implemented the full pattern mutation system across 4 tasks: added mutationRate field to LaneConfig, implemented 3-type mutation logic (drop 40%, ghost 30%, add 30%) in renderRange using deterministic RNG keyed on cycleIndex, added state serialization at version 6 with full parameter wiring (processor + controller), and wrote 4 golden tests covering determinism, anchor protection, backward compatibility, and block-size independence. Mutation respects anchor steps (never mutated) and is fully deterministic given the same (seed, laneId, cycleIndex).

## Verification

201/201 tests pass including 4 new GoldenMutation tests. clang-format clean. RT safety check passes. All pre-push checks green.

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
