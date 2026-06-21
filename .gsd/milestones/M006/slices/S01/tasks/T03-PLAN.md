---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T03: Write visual rendering smoke test

Write a smoke test that creates a LaneGridView with a default LaneConfig, calls renderViewToBitmap(), asserts non-null bitmap, and saves output PNG. Verifies the offscreen rendering pipeline works end-to-end.

## Inputs

- `tests/ui/visual/visual_test_harness.h`
- `plugin/source/ui/lane_grid_view.h`

## Expected Output

- `tests/ui/visual/visual_smoke_tests.cpp`
- `build output PNG`

## Verification

ctest --test-dir build -R visual_smoke --output-on-failure passes
