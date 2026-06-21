---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T03: Density guardrails

After macro resolution clamps hitCount, enforce densityMin and densityMax bounds on the resolved hitCount. If macro-driven hitCount falls below densityMin, force it up. If above densityMax, force it down. Apply in resolveMacros output or as a post-resolution step before renderRange.

## Inputs

- `engine/src/macro.cpp`

## Expected Output

- `engine/src/macro.cpp`

## Verification

cmake --build build && ctest --test-dir build -R constraint
