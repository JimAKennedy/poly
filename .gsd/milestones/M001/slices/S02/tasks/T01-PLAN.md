---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T01: Engine domain model types

Expand types.h with the full domain model from IMPLEMENTATION_PLAN.md §3: Role enum, Cycle struct, AccentMask, EnvTarget enum, Shape enum, Envelope struct, EnvelopeAssign, LaneConfig struct, MacroValues, GrooveState. Use fixed-size containers (std::array, inline arrays) instead of std::vector for RT safety. Update Engine::renderRange() signature to accept const GrooveState&.

## Inputs

- `IMPLEMENTATION_PLAN.md §3`

## Expected Output

- `engine/include/poly/types.h with full domain model`
- `engine/include/poly/engine.h with updated renderRange signature`

## Verification

cmake --build build --target poly_engine compiles clean with zero warnings
