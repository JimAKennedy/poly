---
id: S03
parent: M008
milestone: M008
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
  - plugin/source/ui/micro_timing_editor_view.h
  - plugin/source/ui/micro_timing_editor_view.cpp
key_decisions: []
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-23T02:29:07.783Z
blocker_discovered: false
---

# S03: Extended Micro-timing Maps

**Per-step micro-timing offsets for groove templates with drag-to-adjust editor**

## What Happened

Implemented per-step micro-timing maps: each step position within a cycle can have an independent timing offset (-20 to +20ms). Offsets compose additively with swing and humanize. Works with both equal and additive cells. Micro-timing Editor view provides drag-to-adjust bar chart with double-click reset. State serialized in version 10. 6 new tests (237 total).

## Verification

237/237 tests pass, build clean, clang-format verified, RT safety check passes

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
