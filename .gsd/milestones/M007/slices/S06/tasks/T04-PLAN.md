---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T04: Design and implement Pocket Groove preset

Create preset with per-lane timing offsets: kick at +3ms, snare at -2ms, hi-hat at +1ms. Combined with light mutation (0.15) for organic variation. Demonstrates J Dilla / MPC pocket feel per research paper section 9.1.

## Inputs

- `engine/include/poly/presets.h`

## Expected Output

- `presets.h with makePocketGroove()`

## Verification

Build compiles; preset_tests pass
