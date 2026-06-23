---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T03: Verify build and tests pass with expanded param block

Build and run full test suite. Update golden test baseline if param block expansion changes default output.

## Inputs

- `Current golden baseline`

## Expected Output

- `All tests green`

## Verification

cmake --build build && ctest --test-dir build --output-on-failure
