---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T03: Extended envelope unit tests

Add tests for all 8 envelope targets applied in renderRange: verify each target modulates the correct parameter, verify modulation direction and clamping, verify per-lane and global envelopes interact correctly. Add shape tests for Curve and StepList evaluateShape outputs. Test edge cases: depth=0 (no modulation), depth=1 (full modulation), phase boundaries.

## Inputs

- `engine/include/poly/types.h`
- `engine/src/engine.cpp`
- `engine/src/envelope.cpp`

## Expected Output

- `tests/envelope_tests.cpp`

## Verification

cmake --build build && ctest --test-dir build -R envelope
