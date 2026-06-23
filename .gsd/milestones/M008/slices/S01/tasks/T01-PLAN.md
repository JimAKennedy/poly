---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T01: Extend Cycle and LaneConfig with additive cell array

Add cellSizes array and cellCount to LaneConfig. When cellCount > 0, the cycle uses variable-width cells instead of equal steps. Add helper function to compute cumulative PPQ offsets and total cycle length from cell sizes.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `engine/include/poly/types.h with cellSizes/cellCount fields and helper`

## Verification

cmake --build build && ctest --test-dir build (existing tests still pass — no engine changes yet)
