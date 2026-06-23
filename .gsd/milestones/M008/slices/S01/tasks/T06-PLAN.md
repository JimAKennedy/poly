---
estimated_steps: 1
estimated_files: 5
skills_used: []
---

# T06: Cell Editor View and lane grid additive indicator

Create a VSTGUI custom view (CellEditorView) for editing additive cell sequences per lane. Shows cell widths as proportional rectangles (e.g. [2|2|3] for 7/8). Click to select a cell, drag to resize, +/- buttons to add/remove cells. Updates cellSizes and cellCount on LaneConfig via state serialization (not VST3 params). Also add an additive mode indicator to LaneGridView — show the cell pattern shorthand (e.g. '2+2+3') when cellCount > 0. View reads current lane state from processor via bridge.

## Inputs

- `engine/include/poly/types.h`
- `plugin/source/ui/lane_edit_view.cpp`

## Expected Output

- `Cell editor view renders in plugin UI`
- `Lane grid shows additive indicator for lanes with cellCount > 0`

## Verification

cmake --build build && ctest --test-dir build (build passes, UI tests pass)
