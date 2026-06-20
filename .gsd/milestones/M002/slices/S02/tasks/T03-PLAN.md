---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T03: Apply envelope modulation in renderRange

In renderRange, after computing base velocity, evaluate all active per-lane envelopes and global envelopes at the note's ppqPosition. For EnvTarget::Velocity: modulate velocity by envelope value * depth. For EnvTarget::Density: modulate probability threshold. Superimpose multiple envelopes multiplicatively for velocity, additively for density. Clamp results to valid ranges.

## Inputs

- `engine/include/poly/types.h`
- `engine/src/engine.cpp`

## Expected Output

- `engine/src/engine.cpp`

## Verification

cd build && cmake --build . && ctest --output-on-failure
