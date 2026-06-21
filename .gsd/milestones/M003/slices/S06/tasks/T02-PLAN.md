---
estimated_steps: 1
estimated_files: 4
skills_used: []
---

# T02: Cycle phase indicators

Add per-lane phase output parameters (kLanePhaseOutput base) emitted from process() showing current normalized position within each lane cycle. Display as a small circular or linear progress indicator in LaneGridView for each lane row.

## Inputs

- `plugin/source/processor.cpp`
- `plugin/source/ui/lane_grid_view.cpp`

## Expected Output

- `plugin/source/processor.cpp`
- `plugin/source/ui/lane_grid_view.cpp`

## Verification

cmake --build build && ctest --test-dir build
