---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T02: Update preset reset and apply for new params

Update header_view.cpp applyPreset and resetToInit to include Steps, Subdivision, Hits, Rotation, MidiNote params. Each preset must set explicit values for all 5 new params.

## Inputs

- `presets.h factory preset definitions`
- `Current header_view.cpp applyPreset`

## Expected Output

- `header_view.cpp sets all 5 new params per lane in applyPreset and resetToInit`

## Verification

cmake --build build && ctest --test-dir build
