---
estimated_steps: 1
estimated_files: 4
skills_used: []
---

# T04: Add envelope unit and golden tests

Write unit tests for: evaluateShape at key phase points (0, 0.25, 0.5, 0.75), computeEnvelopePhase with various periodBars and offsets, envelope phase consistency across loop boundaries. Add a golden test with a multi-lane patch using envelopes with non-dividing periods (e.g. 3 bars and 7 bars) to verify emergent patterns are deterministic.

## Inputs

- `engine/include/poly/types.h`
- `engine/src/engine.cpp`

## Expected Output

- `tests/envelope_tests.cpp`
- `tests/golden/envelope_patch_8bars.txt`

## Verification

cd build && cmake --build . && ctest --output-on-failure
