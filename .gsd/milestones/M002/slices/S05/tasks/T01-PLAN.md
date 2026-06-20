---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T01: Define and register EditController parameters

Define ParamID enum covering: per-lane params (probability, baseVelocity, emphasisProb, ghostFloor, velocitySpread, swingAmount, humanizeMs, noteDuration, active) x 8 lanes + macro params (complexity, density, syncopation, swing, tension, humanize) + global params (activeLaneCount, seed). Register all in EditController::initialize() with appropriate units, ranges, and default values.

## Inputs

- `engine/include/poly/types.h`
- `plugin/source/controller.h`
- `plugin/source/controller.cpp`

## Expected Output

- `plugin/source/plugids.h`
- `plugin/source/controller.cpp`

## Verification

cd build && cmake --build . && ctest --output-on-failure
