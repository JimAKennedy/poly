---
id: S03
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
  - plugin/source/processor.cpp
  - plugin/source/controller.cpp
  - plugin/source/ui/phase_alignment_view.cpp
  - tests/golden_tests.cpp
key_decisions:
  - Drift computed from floor(barPos * driftRate) — integer step shifts only, no sub-step interpolation
  - Bipolar parameter range [-4, +4] steps/bar with normalized center at 0.5
  - Drift applies to all cycleStep-dependent lookups: pattern, anchors, accents, mutation
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-22T19:55:46.134Z
blocker_discovered: false
---

# S03: Phase Drift

**Per-lane phase drift enables Reich-style phasing between lanes, derived from absolute PPQ position**

## What Happened

Implemented driftRate parameter (steps/bar) that gradually rotates the pattern lookup index based on absolute PPQ bar position. Uses floor(barPos * driftRate) to compute integer drift steps, then shifts the cycleStep in the per-step rendering loop. All downstream lookups (Euclidean pattern, anchors, accents, mutation) use the drifted step. Bipolar parameter range [-4, +4] steps/bar via normalized 0.5 center. State version bumped to 7 with backward-compatible serialization. Phase alignment view shows drift trajectories as arc trails with arrowheads. Five golden tests verify: zero-rate baseline equivalence, one-step-per-bar behavior, inter-lane phasing, transport jump correctness, and block-size independence.

## Verification

All 206 tests pass including 5 new drift golden tests. clang-format clean. RT safety check passed. Transport jump determinism verified by golden test 29.

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
