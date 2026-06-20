---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T04: Update golden tests for dynamic shaping

Regenerate golden test reference file with a default patch that exercises accent masks, emphasis probability, and ghost floor. The default_patch_4bars.txt golden needs updating since velocity values will change. Add a new golden test with a patch specifically configured for dynamic shaping features.

## Inputs

- `tests/golden/default_patch_4bars.txt`

## Expected Output

- `Updated golden reference file`
- `New dynamic_shaping golden test case`

## Verification

cd build && cmake --build . && ctest --output-on-failure
