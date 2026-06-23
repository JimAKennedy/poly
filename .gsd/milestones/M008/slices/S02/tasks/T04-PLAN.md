---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T04: Tests for timeline mode

Add tests: (1) timeline lane uses fixed pattern, not Euclidean, (2) macros don't affect timeline lanes, (3) timeline + envelopes still work, (4) timeline + phrase gating works, (5) golden test for a timeline bell pattern.

## Inputs

- `engine/include/poly/types.h`
- `engine/src/engine.cpp`
- `engine/src/scene.cpp`

## Expected Output

- `Timeline test cases pass`
- `Golden test for timeline pattern`

## Verification

cmake --build build && ctest --test-dir build
