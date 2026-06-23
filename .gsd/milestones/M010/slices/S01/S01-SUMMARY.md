---
id: S01
parent: M010
milestone: M010
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - plugin/source/plugids.h
  - plugin/source/processor.cpp
  - plugin/source/controller.cpp
  - plugin/source/ui/header_view.cpp
key_decisions:
  - Separate ID block at 800+ for core params — avoids breaking existing kParamsPerLane layout
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-23T01:27:01.476Z
blocker_discovered: false
---

# S01: VST3 Parameter Wiring for Core Euclidean Params

**All 5 core Euclidean per-lane params (steps, subdivision, hits, rotation, MIDI note) exposed as VST3 parameters with full preset and state serialization support**

## What Happened

Added 40 new VST3 parameters (5 per lane × 8 lanes) for core Euclidean fields that were previously hardcoded. Used a separate ID block starting at 800 to preserve backward compatibility with existing presets and automation. Wired through processor applyParameter, controller registration with appropriate ranges/defaults, setComponentState sync, and preset apply/reset. Golden test baseline unchanged since defaults match previously hardcoded values.

## Verification

216/216 tests pass. RT safety check passes. Plugin deploys and loads in Cubase. Presets fully reset all params including new core Euclidean ones.

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
