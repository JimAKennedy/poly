---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T02: Implement Curve and StepList shapes

Replace the stub implementations in evaluateShape(). Curve: implement exponential ease-in/ease-out using a curvature parameter (repurpose depth or add a shape parameter field). StepList: add a step values array to the Envelope struct (fixed-size, e.g. 16 floats) and implement lookup by quantized phase. Update state_io for any new fields (bump kCurrentStateVersion).

## Inputs

- `engine/src/envelope.cpp`
- `engine/include/poly/types.h`

## Expected Output

- `engine/src/envelope.cpp`
- `engine/include/poly/types.h`

## Verification

cmake --build build && ctest --test-dir build -R envelope
