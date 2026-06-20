---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T01: Implement accent mask velocity boost

In renderRange, after Euclidean pattern check and probability gate, check if the current cycleStep is marked in cfg.accents.steps[]. If true, boost velocity by a fixed accent amount (e.g. +0.15, clamped to 1.0). The accent mask is already defined in LaneConfig but not used.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `Modified engine.cpp with accent mask application in renderRange`
- `New accent_mask unit tests`

## Verification

cd build && cmake --build . && ctest --output-on-failure
