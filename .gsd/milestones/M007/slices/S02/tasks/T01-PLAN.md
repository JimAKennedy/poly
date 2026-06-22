---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T01: Add mutationRate to LaneConfig

Add mutationRate (float, 0.0-1.0, default 0.0) to LaneConfig. 0.0 = no mutation (backward compatible). Controls probability of per-step variation each cycle.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `types.h with mutationRate field`

## Verification

Build compiles; existing tests pass unchanged
