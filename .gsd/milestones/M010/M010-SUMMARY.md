---
id: M010
title: "Expose Core Euclidean Parameters in UI"
status: complete
completed_at: 2026-06-23T01:28:02.615Z
key_decisions:
  - Separate ID block at 800+ for core params to avoid breaking existing param layout
  - Merged Pattern + Voice into one dense row instead of separate sections
  - Shrunk VelocityView from 76px to 40px to minimize height increase
  - Skipped S03 since its goals were covered by S01 tasks
key_files:
  - plugin/source/plugids.h
  - plugin/source/processor.cpp
  - plugin/source/controller.cpp
  - plugin/source/ui/header_view.cpp
  - plugin/source/ui/lane_edit_view.h
  - plugin/source/ui/lane_edit_view.cpp
  - plugin/resource/poly.uidesc
  - plugin/CMakeLists.txt
lessons_learned:
  - (none)
---

# M010: Expose Core Euclidean Parameters in UI

**All core Euclidean params (steps, subdivision, hits, rotation, MIDI note) exposed as VST3 parameters with a new 10-knob LaneEditView for pattern and voice control**

## What Happened

Added 40 new VST3 parameters (5 core Euclidean fields × 8 lanes) in a separate ID block starting at 800 to preserve backward compatibility. Created LaneEditView custom view with 10 knobs in two groups: Pattern (Steps, Subdiv, Hits, Rotation, Note) and Voice (Vel, Ghost, Spread, Swing, Kotekan). Lane tabs sync with PhraseEditView via shared kSelectedLane. Updated all presets and resetToInit to fully reset new params. Shrunk VelocityView from 76px to 40px; net window height +42px (628→670px). S03 (Golden Test and Preset Update) was skipped — its goals were fully covered by S01 tasks T02 and T03. All 216 tests pass, RT safety check passes, plugin deployed successfully.

## Success Criteria Results

- ✅ All 5 core Euclidean params (steps, subdivision, hits, rotation, midiNote) exposed as VST3 parameters\n- ✅ LaneEditView shows 10 knobs per lane covering pattern + voice params\n- ✅ Presets fully reset all parameters including new ones\n- ✅ All 216 tests pass, plugin deploys and runs in Cubase

## Definition of Done Results

Not provided.

## Requirement Outcomes

Not provided.

## Deviations

None.

## Follow-ups

None.
