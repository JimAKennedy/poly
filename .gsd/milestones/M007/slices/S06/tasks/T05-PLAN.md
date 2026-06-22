---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T05: Update preset infrastructure and tests

Update kFactoryPresetCount, makeFactoryPreset(), getFactoryPresetInfo(), and controller preset list to include all 4 new presets. Update preset_tests to cover new presets.

## Inputs

- `engine/include/poly/presets.h`
- `plugin/source/controller.cpp`
- `tests/preset_tests.cpp`

## Expected Output

- `Updated preset infrastructure and passing tests`

## Verification

ctest --test-dir build -R preset passes; all 9 presets load without error
