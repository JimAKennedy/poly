---
id: S05
parent: M008
milestone: M008
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - plugin/source/ui/cross_rhythm_view.h
  - plugin/source/ui/cross_rhythm_view.cpp
  - plugin/resource/poly.uidesc
  - plugin/source/controller.cpp
key_decisions: []
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-23T18:33:22.235Z
blocker_discovered: false
---

# S05: Cross-rhythm Visualization

**CrossRhythmView shows lane cycle boundaries on a linear timeline with convergence markers**

## What Happened

Created a new VSTGUI CView subclass that renders a linear timeline visualization of all active lanes' rhythmic cycles. Each lane gets a horizontal row with tick markers at step/cell boundaries (supporting both equal and additive cells). Convergence points where 2+ lanes' cycle boundaries coincide are highlighted with gold diamond markers and vertical highlight lines. Display span auto-scales using LCM of cycle lengths (capped at 8 bars). Registered in uidesc, controller createCustomView, and both plugin and test CMakeLists. Interaction smoke test verifies view presence in frame hierarchy.

## Verification

cmake --build build && ctest --test-dir build — 237/237 tests pass. ViewTreeTest.CustomViewsInTree confirms CrossRhythmView in view tree. clang-format clean. RT safety clean.

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
