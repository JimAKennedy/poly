---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T01: Add driftRate to LaneConfig

Add driftRate (float, steps per bar, default 0.0) to LaneConfig. At 1.0, the pattern rotates by 1 step every bar. Fractional values produce gradual sub-step drift. Negative values drift backward.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `types.h with driftRate field`

## Verification

Build compiles; existing tests pass unchanged
