---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T04: Tests for timeline mode

Add tests: (1) timeline lane uses fixed pattern, not Euclidean, (2) macros don't affect timeline lanes, (3) timeline + envelopes still work, (4) timeline + phrase gating works, (5) golden test for a timeline bell pattern.

## Inputs

- `tests/scene_tests.cpp`
- `tests/golden_tests.cpp`

## Expected Output

- `New timeline test cases passing`

## Verification

cmake --build build && ctest --test-dir build
