---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T02: Implement genre presets in presets.h

Add 5 new factory preset functions using additive cells, timeline mode, and micro-timing maps. Update kFactoryPresetCount. Wire into the preset list.

## Inputs

- `engine/include/poly/presets.h`
- `engine/include/poly/types.h`

## Expected Output

- `presets.h with 5 new genre presets`

## Verification

cmake --build build && ctest --test-dir build
