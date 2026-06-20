---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T03: Add macro mapping tests and golden updates

Write unit tests verifying: each macro at 0.0, 0.5, 1.0 produces expected parameter ranges. Test that macros compose correctly (density + complexity together). Add golden test with macros at non-default values to verify deterministic output under macro resolution.

## Inputs

- `engine/include/poly/macro.h`
- `engine/include/poly/types.h`

## Expected Output

- `tests/macro_tests.cpp`

## Verification

cd build && cmake --build . && ctest --output-on-failure
