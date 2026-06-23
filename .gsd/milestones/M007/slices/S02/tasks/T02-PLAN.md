---
estimated_steps: 7
estimated_files: 1
skills_used: []
---

# T02: Implement deterministic pattern mutation in renderRange

After generating Euclidean pattern and computing cycleStep, apply mutation:
- Compute absolute cycle index: cycleIndex = absStep / stepsInCycle
- For each step, roll deterministicRand(seed, laneId, cycleIndex, stepInCycle + mutation_salt)
- If roll < mutationRate, apply one of: toggle step (add/remove hit), displace step (+/- 1 position), reduce velocity to ghost level
- Mutation type selected deterministically from hash of cycleIndex + stepInCycle
- Must not mutate anchor steps (constraint system)
Key invariant: same (seed, laneId, cycleIndex) always produces same mutations.

## Inputs

- `engine/src/engine.cpp`
- `engine/include/poly/rng.h`

## Expected Output

- `engine.cpp with mutation logic`

## Verification

Build + existing tests pass; mutation rate 0 produces identical output to baseline
