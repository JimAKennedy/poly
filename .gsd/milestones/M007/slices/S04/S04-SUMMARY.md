---
id: S04
parent: M007
milestone: M007
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - engine/include/poly/types.h
  - engine/src/engine.cpp
  - engine/include/poly/state_io.h
  - plugin/source/plugids.h
  - plugin/source/controller.cpp
  - plugin/source/processor.cpp
  - tests/golden_tests.cpp
key_decisions:
  - Clamp timing offset to PPQ 0.0 instead of block ppqStart to preserve block-size independence
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-22T21:05:41.794Z
blocker_discovered: false
---

# S04: Lane Timing Offset

**Per-lane timing offset (-20 to +20 ms) for groove pocket feel, with state serialization and 5 golden tests**

## What Happened

Added timingOffsetMs to LaneConfig, applied in renderRange() after swing and humanize. The offset converts ms to PPQ using tempo and stacks with existing timing modifications. Key design decision: clamp to 0.0 instead of tc.ppqStart to preserve block-size independence (clamping to block start caused different output depending on how the PPQ range was sliced). State version bumped to 8. VST3 parameter registered with bipolar mapping (normalized 0.5 = 0ms). Five golden tests verify zero-identity, positive/negative shift, swing interaction, and block-size determinism.

## Verification

211/211 tests pass. RT safety check clean. clang-format applied. All 35 golden tests pass including 5 new timing offset tests.

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
