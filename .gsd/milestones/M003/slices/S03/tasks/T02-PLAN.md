---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T02: Anchor and backbeat constraints

In renderRange, after probability gate: if the step is in anchorSteps, force it to fire (skip probability roll). For backbeat protection: if backbeatProtect is true and the step is at a backbeat position (step 2, 6 in 8-step; step 1, 3 in 4-step), preserve base velocity and emphasisProb from pre-macro values. Apply after macro resolution but before envelope modulation.

## Inputs

- `engine/src/engine.cpp`
- `engine/include/poly/types.h`

## Expected Output

- `engine/src/engine.cpp`

## Verification

cmake --build build && ctest --test-dir build -R constraint
