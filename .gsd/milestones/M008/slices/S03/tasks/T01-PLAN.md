---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T01: Add micro-timing map to LaneConfig

Add std::array<float, kMaxSteps> microTimingMs to LaneConfig, defaulting to all zeros. Each entry is the timing offset in ms for that step position within the cycle. Range: [-20, +20] ms per step.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `LaneConfig has microTimingMs field`

## Verification

cmake --build build && ctest --test-dir build
