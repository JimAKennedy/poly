---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T04: Constraint serialization

Extend state_io to read/write the new ConstraintConfig fields. Bump state version if not already bumped by S02. Handle backwards compat for older versions (default: no anchors, no backbeat protect, density bounds 0/maxSteps).

## Inputs

- `engine/include/poly/state_io.h`

## Expected Output

- `engine/include/poly/state_io.h`

## Verification

cmake --build build && ctest --test-dir build -R plugin
