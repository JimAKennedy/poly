---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T05: Add swing and humanize tests plus golden updates

Write unit tests for: swing offset calculation at various swingAmount values, humanize jitter bounds, note duration configuration. Update the default golden test reference if needed. Add a new golden test with swing=0.5 and humanize=5ms to verify deterministic timing.

## Inputs

- `tests/golden/default_patch_4bars.txt`
- `engine/include/poly/types.h`

## Expected Output

- `New swing/humanize unit tests`
- `Updated golden reference files`

## Verification

cd build && cmake --build . && ctest --output-on-failure
