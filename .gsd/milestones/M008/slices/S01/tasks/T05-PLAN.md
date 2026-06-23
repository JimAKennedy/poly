---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T05: Expose cellCount parameter to controller

Add VST3 parameter for cellCount only (per lane) to controller.cpp. cellSizes array is NOT exposed as individual VST3 params — it is state-serialized only, edited through the Cell Editor view (T06). Wire cellCount so Cubase generic editor can enable/disable additive mode (0 = equal cells).

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `cellCount visible in Cubase generic editor per lane`

## Verification

cmake --build build && ctest --test-dir build
