---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T04: Update golden tests for new envelope behavior

Regenerate the golden reference file if the default patch now produces different output due to envelope changes (it should not if no envelopes are active by default, but verify). Add a new golden test with a patch that exercises multiple envelope targets simultaneously to enforce determinism of the full envelope system.

## Inputs

- `tests/golden/default_patch_4bars.txt`
- `tests/golden_tests.cpp`

## Expected Output

- `tests/golden_tests.cpp`

## Verification

cmake --build build && ctest --test-dir build -R golden
