---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T04: Unit and golden tests for additive/aksak patterns

Add tests: (1) 7/8 aksak [2+2+3] produces correct PPQ positions, (2) 9/8 [2+2+2+3] works, (3) Euclidean E(2,3) over [2+2+3] distributes correctly, (4) block-size independence for aksak, (5) additive + phrase gating composition, (6) additive + drift composition. Add golden test for aksak pattern.

## Inputs

- `tests/euclidean_tests.cpp`
- `tests/golden_tests.cpp`

## Expected Output

- `New aksak test cases passing`

## Verification

cmake --build build && ctest --test-dir build (all new + existing tests pass)
