---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T02: Engine Technical Specification

Write docs/engine-spec.md covering the poly_engine contract: timing model (PPQ-derived, no accumulation), cycle math, Euclidean rhythm generation, velocity/emphasis pipeline, envelope superposition design, and the renderRange() API. Reference actual types from engine/include/poly/.

## Inputs

- `engine/include/poly/engine.h`
- `engine/include/poly/types.h`
- `engine/include/poly/euclidean.h`
- `engine/include/poly/rng.h`
- `engine/src/engine.cpp`

## Expected Output

- `docs/engine-spec.md`

## Verification

All referenced types exist in engine/include/poly/; spec matches implemented renderRange() behavior
