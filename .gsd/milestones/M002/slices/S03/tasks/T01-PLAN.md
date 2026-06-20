---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T01: Add swing and duration fields to LaneConfig

Add float swingAmount (0..1, 0=no swing, 1=full triplet swing) and float noteDuration (in PPQ, default 0 meaning auto = stepPpq*0.5) to LaneConfig. Keep defaults backward-compatible so existing tests pass without changes.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `Updated LaneConfig with swingAmount and noteDuration fields`

## Verification

cd build && cmake --build . && ctest --output-on-failure
