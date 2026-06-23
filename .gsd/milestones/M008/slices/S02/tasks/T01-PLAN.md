---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T01: Add timeline flag and fixed pattern to LaneConfig

Add bool timeline and std::array<bool, kMaxSteps> fixedPattern plus int fixedPatternLength to LaneConfig. When timeline=true, the lane uses fixedPattern instead of Euclidean distribution.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `types.h with timeline fields`

## Verification

cmake --build build && ctest --test-dir build
