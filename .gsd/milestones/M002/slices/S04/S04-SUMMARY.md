---
id: S04
parent: M002
milestone: M002
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - engine/include/poly/macro.h
  - engine/src/macro.cpp
  - tests/macro_tests.cpp
key_decisions:
  - Pure function design — resolveMacros takes and returns GrooveState by value, no side effects
  - Macro curves use sqrt/squared shaping for musical response, not linear
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-20T22:19:47.900Z
blocker_discovered: false
---

# S04: Macro Resolution

**Coherent macro-to-parameter mapping for 6 macros affecting multiple lane parameters**

## What Happened

Implemented resolveMacros() pure function mapping complexity, density, syncopation, swing, tension, and humanize macros to lane parameters. Each macro coherently affects multiple parameters with musically meaningful curves. Integrated into the engine pipeline before renderRange — caller resolves macros on GrooveState, no heap allocation. Comprehensive unit tests verify all mappings at boundary and mid-range values.

## Verification

101/101 tests pass including macro-specific tests. Golden tests verify macro sweep produces expected output.

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
