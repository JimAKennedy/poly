---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T01: Add kotekan linking fields to LaneConfig

Add kotekanSourceLane (int, -1=none, 0-7=source lane index) to LaneConfig. When set, this lane's pattern is the complement of the source lane's Euclidean pattern (all gaps filled, all hits removed). The lane keeps its own midiNote, baseVelocity, and other params.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `types.h with kotekanSourceLane field`

## Verification

Build compiles; existing tests pass unchanged
