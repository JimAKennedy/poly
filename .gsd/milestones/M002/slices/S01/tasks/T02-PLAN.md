---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T02: Implement emphasis probability gate

When a step is an accent position, use emphasisProb to gate whether the accent actually expresses. Use deterministicRand with a new channel (e.g. channel 2) to roll emphasis. If roll >= emphasisProb, the accent is suppressed and the note uses base velocity instead.

## Inputs

- `engine/include/poly/types.h`
- `engine/include/poly/rng.h`

## Expected Output

- `Modified engine.cpp with emphasis probability gating`
- `Emphasis probability tests`

## Verification

cd build && cmake --build . && ctest --output-on-failure
