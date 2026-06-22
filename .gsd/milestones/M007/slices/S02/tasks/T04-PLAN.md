---
estimated_steps: 5
estimated_files: 2
skills_used: []
---

# T04: Golden tests for mutation

Add golden test scenarios:
1. mutationRate=0 produces byte-identical output to baseline
2. mutationRate=0.3 produces consistent variations across runs (determinism)
3. Mutation respects anchor steps (no mutation on anchored positions)
4. Transport jump + mutation produces correct output at new position

## Inputs

- `tests/golden_tests.cpp`

## Expected Output

- `New golden test cases for mutation`

## Verification

ctest --test-dir build -R golden passes
