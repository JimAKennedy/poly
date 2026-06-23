---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T02: Apply micro-timing map in renderRange

In renderRange(), after swing and before humanize, look up microTimingMs[cycleStep] and convert to PPQ offset using tempo. Apply the offset to ppq. Composes additively with swing and existing timingOffsetMs.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `renderRange applies per-step micro-timing offsets`

## Verification

cmake --build build && ctest --test-dir build
