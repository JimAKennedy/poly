---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T04: Interaction smoke tests for cross-rhythm view

Add interaction smoke test verifying the cross-rhythm view renders without crash when loaded and when lane parameters change.

## Inputs

- `tests/ui/interaction/interaction_smoke_tests.cpp`

## Expected Output

- `Smoke test for cross-rhythm view passing`

## Verification

cmake --build build && ctest --test-dir build
