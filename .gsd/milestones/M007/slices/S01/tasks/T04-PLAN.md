---
estimated_steps: 6
estimated_files: 2
skills_used: []
---

# T04: Golden tests for phrase system

Add golden test scenarios:
1. Single lane with phraseLength=2, phraseGap=1 - verify silence during gap bars
2. Two lanes with different phraseLength values - verify offset phrase behavior
3. Phrase with transport jump into gap region - verify correct silence
4. Phrase with loop restart - verify deterministic output
5. phraseLength=0 (continuous) produces identical output to current baseline

## Inputs

- `tests/golden_tests.cpp`
- `tests/golden/default_patch_4bars.txt`

## Expected Output

- `New golden test cases and reference files`

## Verification

ctest --test-dir build -R golden passes
