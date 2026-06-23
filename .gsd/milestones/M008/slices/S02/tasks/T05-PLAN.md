---
estimated_steps: 1
estimated_files: 5
skills_used: []
---

# T05: Timeline Step Editor View and lane grid timeline indicator

Create a VSTGUI custom view (TimelineStepEditorView) for editing the fixed step pattern per lane. Shows a row of step cells matching fixedPatternLength — click to toggle each step on/off. Active steps filled, inactive steps hollow. Updates fixedPattern on LaneConfig via state serialization (not VST3 params). Also add a timeline mode indicator to LaneGridView — show 'TL' badge with distinct color when lane has timeline=true, so users can see at a glance which lanes are timeline anchors vs generative.

## Inputs

- `engine/include/poly/types.h`
- `plugin/source/ui/lane_edit_view.cpp`

## Expected Output

- `Step editor view renders with toggleable step grid`
- `Lane grid shows TL indicator for timeline lanes`

## Verification

cmake --build build && ctest --test-dir build (build passes, UI tests pass)
