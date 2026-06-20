---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T01: Implement shape evaluation functions

Create envelope.h/cpp with shape evaluation: evaluateShape(Shape shape, float phase) returning 0..1. Sine: 0.5*(1+sin(2*pi*phase)), Ramp: phase (sawtooth 0 to 1), Triangle: 1-abs(2*phase-1). Phase is always in [0,1). Curve and StepList can return 0.5 for now (placeholder). All functions must be RT-safe (no allocation).

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `engine/include/poly/envelope.h`
- `engine/src/envelope.cpp`

## Verification

cd build && cmake --build . && ctest --output-on-failure
