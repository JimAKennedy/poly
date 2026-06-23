---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T01: Add timingOffsetMs to LaneConfig

Add timingOffsetMs (float, milliseconds, default 0.0) to LaneConfig. Positive = late (behind the beat), negative = early (ahead of the beat). Range: -20.0 to +20.0 ms.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `types.h with timingOffsetMs field`

## Verification

Build compiles; existing tests pass unchanged
