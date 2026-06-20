---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T01: Design macro-to-parameter mapping

Define how each macro affects lane parameters. Complexity: hitCount, rotation, envelope depth. Density: probability, hitCount, activation weight. Syncopation: rotation, accent bias. Swing: swingAmount across lanes. Tension: velocity spread, emphasis prob, envelope depth. Humanize: humanizeMs, velocity spread. Write a resolveMacros(const GrooveState& input) -> GrooveState function that applies all mappings. Pure function, no allocation.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `engine/include/poly/macro.h`
- `engine/src/macro.cpp`

## Verification

cd build && cmake --build . && ctest --output-on-failure
