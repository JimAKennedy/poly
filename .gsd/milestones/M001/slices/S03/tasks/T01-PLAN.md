---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T01: Euclidean rhythm algorithm

Implement Bjorklund algorithm to distribute hitCount pulses across steps. Pure function in engine, unit tested against known patterns: (3,8), (5,8), (4,16), (1,4), (4,4). Include rotation support.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `engine/include/poly/euclidean.h`
- `engine/src/euclidean.cpp`
- `tests/euclidean_tests.cpp`

## Verification

cd build && cmake --build . --target poly_tests && ctest --test-dir . -R euclidean
