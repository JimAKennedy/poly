---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T04: Implement configurable note duration

Replace the hardcoded ev.duration = sPpq * 0.5 with cfg.noteDuration when noteDuration > 0, otherwise fall back to sPpq * 0.5 as default. This allows per-lane control of note length.

## Inputs

- `engine/include/poly/types.h`
- `engine/src/engine.cpp`

## Expected Output

- `Modified engine.cpp using configurable note duration`

## Verification

cd build && cmake --build . && ctest --output-on-failure
