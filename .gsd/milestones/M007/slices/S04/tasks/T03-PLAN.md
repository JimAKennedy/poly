---
estimated_steps: 5
estimated_files: 2
skills_used: []
---

# T03: State serialization and golden tests for timing offset

Serialize timingOffsetMs in processor.cpp. Add golden tests:
1. timingOffsetMs=0 matches baseline
2. timingOffsetMs=5.0 shifts all events forward by 5ms equivalent PPQ
3. Negative offset shifts events earlier
4. Offset interacts correctly with swing (both applied)

## Inputs

- `plugin/source/processor.cpp`
- `tests/golden_tests.cpp`

## Expected Output

- `Updated processor.cpp and golden tests`

## Verification

ctest --test-dir build -R golden passes; RT safety check passes
