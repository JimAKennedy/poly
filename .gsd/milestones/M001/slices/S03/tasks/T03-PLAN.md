---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T03: Deterministic position-based RNG for probability and velocity

Implement position-seeded RNG: state derived from (seed, laneId, absolute step index) so output is identical regardless of block boundaries. Apply probability gating (skip hits when roll > lane.probability) and velocity spread (baseVelocity +/- spread). No accumulator state — pure function of position.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `engine/include/poly/rng.h`
- `engine/src/engine.cpp`

## Verification

cd build && cmake --build . && ./tools/harness/poly_harness 4 120 0.05 > /tmp/a.txt && ./tools/harness/poly_harness 4 120 0.2 > /tmp/b.txt && diff <(grep -v '^#' /tmp/a.txt) <(grep -v '^#' /tmp/b.txt)
