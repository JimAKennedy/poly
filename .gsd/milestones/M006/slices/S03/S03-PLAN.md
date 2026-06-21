# S03: Visual Regression Baselines

**Goal:** Establish visual regression baselines for LaneGridView and VelocityView in default and key parameter states, with comparison tests that fail on pixel drift
**Demo:** Offscreen renders of LaneGridView and VelocityView in default and key states, with baseline PNGs and comparison tests

## Must-Haves

- Complete the planned slice outcomes.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Visual regression tests with state variation** `est:20min`
  Replace smoke-only rendering with proper regression tests that render LaneGridView and VelocityView in multiple parameter states (default, high complexity, max lanes, etc.), compare against reference PNGs using compareImages(), and generate diff images on mismatch.
  - Files: `tests/ui/visual/visual_smoke_tests.cpp`
  - Verify: cmake --build build --target poly_visual_tests && ctest --test-dir build -R VisualRegression --output-on-failure

- [x] **T02: Generate and commit baseline reference PNGs** `est:5min`
  Run the visual tests in baseline-generation mode (no reference exists yet = save as reference), verify PNGs are valid, commit them to tests/ui/visual/references/
  - Files: `tests/ui/visual/references/*.png`
  - Verify: ls tests/ui/visual/references/*.png && file tests/ui/visual/references/*.png

## Files Likely Touched

- tests/ui/visual/visual_smoke_tests.cpp
- tests/ui/visual/references/*.png
