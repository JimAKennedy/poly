---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T03: Parameter naming cleanup

Rename automation lane labels for scannability. Macros get 'Macro | Complexity' format. Ensure names are unique and under the VST3 128-char title limit. Verify no ParamID collisions with any new params from S02/S03.

## Inputs

- `plugin/source/controller.cpp`
- `plugin/source/plugids.h`

## Expected Output

- `plugin/source/controller.cpp`

## Verification

cmake --build build && ctest --test-dir build -R plugin
