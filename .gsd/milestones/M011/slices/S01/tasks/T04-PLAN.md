---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T04: Run full local verification (pre-commit + build + test)

Run the complete pre-commit suite and build+test locally to confirm all code-quality and compilation issues are resolved before pushing.

## Inputs

- None specified.

## Expected Output

- `clean pre-commit run`
- `clean build`
- `all tests passing`

## Verification

pre-commit run --all-files && cmake --build build && ctest --test-dir build
