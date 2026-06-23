---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T05: Micro-timing Editor View

Create a VSTGUI custom view (MicroTimingEditorView) for editing per-step micro-timing offsets per lane. Shows a bar chart with one vertical bar per step — bar height represents the ms offset (center line = 0, up = positive, down = negative). Drag a bar to adjust its value within [-20, +20] ms range. Double-click to reset to 0. Shows numeric tooltip during drag. Updates microTimingMs on LaneConfig via state serialization (not VST3 params). View width adapts to current step count.

## Inputs

- `engine/include/poly/types.h`
- `plugin/source/ui/lane_edit_view.cpp`

## Expected Output

- `Micro-timing editor renders with draggable per-step offset bars`
- `Double-click resets individual step to zero`

## Verification

cmake --build build && ctest --test-dir build (build passes, UI tests pass)
