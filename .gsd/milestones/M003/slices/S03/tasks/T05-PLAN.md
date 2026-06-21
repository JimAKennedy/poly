---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T05: Constraint tests

Test anchor steps always fire with probability=0. Test backbeat protection preserves emphasis under extreme syncopation macro. Test density guardrails clamp hitCount at extremes (density macro=0 with densityMin=2, density macro=1 with densityMax=4). Test serialization round-trip.

## Inputs

- `engine/include/poly/types.h`
- `engine/src/engine.cpp`

## Expected Output

- `tests/constraint_tests.cpp`

## Verification

cmake --build build && ctest --test-dir build -R constraint
