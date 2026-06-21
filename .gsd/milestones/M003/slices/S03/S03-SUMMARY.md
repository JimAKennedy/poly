---
id: S03
parent: M003
milestone: M003
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - engine/include/poly/constraint.h
  - engine/src/constraint.cpp
  - engine/include/poly/types.h
  - engine/src/engine.cpp
  - engine/include/poly/state_io.h
  - tests/constraint_tests.cpp
key_decisions:
  - Constraints applied post-macro, pre-envelope — anchors override probability gate, not envelope modulation
  - State version bumped to v4 with safe defaults for backwards compat
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-21T03:10:51.325Z
blocker_discovered: false
---

# S03: Constraint Layer

**Anchor steps, backbeat protection, and density guardrails prevent macros/envelopes from destroying groove coherence**

## What Happened

Implemented three constraint types: (1) Anchor steps that always fire regardless of probability/envelope attenuation, (2) Backbeat protection that preserves emphasis on backbeat positions even under extreme syncopation macros, (3) Density guardrails that clamp post-macro hitCount to configurable min/max bounds. Constraints applied after macro resolution, before output. State version bumped to v4 with backwards compat defaults (no anchors, no backbeat protect, density bounds 0/maxSteps). Comprehensive constraint_tests verify each constraint type at extremes plus serialization round-trip.

## Verification

All 156 tests pass including constraint_tests covering anchors with prob=0, backbeat under extreme syncopation, density clamping at extremes, and serialization round-trip. Commit 66ab642.

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
