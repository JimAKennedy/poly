---
estimated_steps: 5
estimated_files: 2
skills_used: []
---

# T03: State serialization and golden tests for kotekan

Serialize kotekanSourceLane. Add golden tests:
1. Kotekan pair: lane A plays E(3,8), lane B is complement (plays the 5 gaps)
2. Source + complement notes together fill all 8 positions exactly
3. Kotekan with different velocities per voice
4. kotekanSourceLane=-1 matches baseline

## Inputs

- `plugin/source/processor.cpp`
- `tests/golden_tests.cpp`

## Expected Output

- `Updated processor.cpp and golden tests`

## Verification

ctest --test-dir build -R golden passes
