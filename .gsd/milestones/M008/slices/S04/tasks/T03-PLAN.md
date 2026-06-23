---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T03: Preset tests for genre presets

Add smoke tests for each new preset: loads without error, produces events over 4 bars, round-trips through serialization with identical output.

## Inputs

- `tests/preset_tests.cpp`

## Expected Output

- `Preset tests for 5 new presets passing`

## Verification

cmake --build build && ctest --test-dir build
