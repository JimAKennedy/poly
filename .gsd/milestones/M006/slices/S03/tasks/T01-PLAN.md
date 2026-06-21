---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T01: Visual regression tests with state variation

Replace smoke-only rendering with proper regression tests that render LaneGridView and VelocityView in multiple parameter states (default, high complexity, max lanes, etc.), compare against reference PNGs using compareImages(), and generate diff images on mismatch.

## Inputs

- `tests/ui/visual/visual_test_harness.h`
- `tests/ui/visual/image_compare.h`
- `plugin/source/plugids.h`

## Expected Output

- `tests/ui/visual/visual_smoke_tests.cpp with regression test fixtures`

## Verification

cmake --build build --target poly_visual_tests && ctest --test-dir build -R VisualRegression --output-on-failure
