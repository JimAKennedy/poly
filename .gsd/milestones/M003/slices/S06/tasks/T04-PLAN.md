---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T04: Visual regression baselines for new views

Add visual regression test cases for EnvelopeCurveView and PhaseAlignmentView. Render each view in key states (default, mid-phase, multiple shapes). Generate baseline PNGs. Add to visual_smoke_tests.cpp.

## Inputs

- `tests/ui/visual/visual_smoke_tests.cpp`
- `tests/ui/visual/visual_test_harness.h`

## Expected Output

- `tests/ui/visual/visual_smoke_tests.cpp`

## Verification

cmake --build build && ctest --test-dir build -R visual
