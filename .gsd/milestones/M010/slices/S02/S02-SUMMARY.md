---
id: S02
parent: M010
milestone: M010
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - plugin/source/ui/lane_edit_view.h
  - plugin/source/ui/lane_edit_view.cpp
  - plugin/CMakeLists.txt
  - plugin/source/controller.cpp
  - plugin/resource/poly.uidesc
key_decisions:
  - Merged Pattern + Voice into one dense row (Option A+B from design) instead of separate sections
  - Shrunk VelocityView from 76px to 40px to minimize height increase
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-23T01:27:35.890Z
blocker_discovered: false
---

# S02: Lane Edit UI View

**New LaneEditView with 10 knobs (Pattern + Voice groups) per lane, VelocityView shrunk, window height adjusted to 670px**

## What Happened

Created a new LaneEditView custom view with 10 knobs in two groups: Pattern (Steps, Subdiv, Hits, Rotation, Note) and Voice (Vel, Ghost, Spread, Swing, Kotekan). Lane tabs sync with PhraseEditView via shared kSelectedLane param. Custom display formats for each knob type — integer, subdivision notation, MIDI note names, percentage, lane source labels. Shrunk VelocityView from 76px to 40px. Net window height increase of 42px (628→670px). Wired into controller createCustomView and poly.uidesc layout.

## Verification

216/216 tests pass. RT safety check passes. Plugin deployed to ~/Library/Audio/Plug-Ins/VST3/poly_plugin.vst3 and loads in Cubase with new Lane Edit section visible.

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
