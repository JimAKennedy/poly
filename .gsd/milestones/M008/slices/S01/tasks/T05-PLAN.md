---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T05: Expose additive cell parameters to controller

Add VST3 parameters for cellCount and cellSizes (per lane) to controller.cpp. Wire up so Cubase generic editor can set additive cells.

## Inputs

- `plugin/source/controller.cpp`
- `plugin/source/plugids.h`

## Expected Output

- `controller.cpp with additive cell parameters`

## Verification

cmake --build build && ctest --test-dir build
