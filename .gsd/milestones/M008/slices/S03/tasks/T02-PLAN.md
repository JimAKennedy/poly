---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T02: Apply micro-timing map in renderRange

In renderRange(), after swing and before humanize, look up microTimingMs[cycleStep] and convert to PPQ offset using tempo. Apply the offset to ppq. Composes additively with swing and existing timingOffsetMs.

## Inputs

- `engine/src/engine.cpp`

## Expected Output

- `engine.cpp with micro-timing map application`

## Verification

cmake --build build && ctest --test-dir build
