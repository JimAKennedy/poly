---
estimated_steps: 6
estimated_files: 2
skills_used: []
---

# T04: Golden tests for drift including transport jump

Add golden test scenarios:
1. driftRate=0 produces identical output to baseline
2. driftRate=1.0 rotates pattern by 1 step per bar
3. Two lanes with identical patterns but different driftRates produce phasing
4. Transport jump to bar 8 with driftRate=0.5 produces correct 4-step rotation
5. Loop restart correctly resets drift to bar-start drift state

## Inputs

- `tests/golden_tests.cpp`

## Expected Output

- `New golden test cases for drift`

## Verification

ctest --test-dir build -R golden passes
