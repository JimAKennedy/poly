---
id: S06
parent: M002
milestone: M002
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - plugin/resource/poly.uidesc
  - plugin/source/ui/lane_grid_view.h
  - plugin/source/ui/lane_grid_view.cpp
  - plugin/source/ui/velocity_view.h
  - plugin/source/ui/velocity_view.cpp
  - plugin/source/controller.cpp
key_decisions:
  - Custom CView subclasses for lane grid and velocity display rather than built-in controls
  - Velocity feedback via outputParameterChanges (RT-safe) rather than allocateMessage
  - Design system alignment — all colors use jk.digital neutral grayscale tokens
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-20T22:20:10.901Z
blocker_discovered: false
---

# S06: Minimal VSTGUI Editor

**VSTGUI editor with lane grid, macro knobs, velocity display, aligned to jk.digital design system**

## What Happened

Built minimal VSTGUI editor with three sections: LaneGridView (8-lane probability grid showing active/inactive state and probability bars), macro knob row (6 CKnobs bound to macro ParamIDs via control-tags), and VelocityView (per-lane velocity bars updated via RT-safe outputParameterChanges). All colors aligned to jk.digital design system tokens — neutral grayscale palette replacing initial purple theme. Velocity output uses read-only params (400-407) written by processor only when notes fire.

## Verification

101/101 tests pass. Build clean with zero warnings from project code. VST3 bundle contains uidesc resource.

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
