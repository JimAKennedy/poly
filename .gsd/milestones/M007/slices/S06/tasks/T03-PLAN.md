---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T03: Design and implement Kotekan Interlock preset

Create preset with a kotekan pair: one lane plays E(3,8) on a high woodblock, its complement plays the gaps on a low woodblock. Add a steady pulse lane and a ghost shimmer. Demonstrates Balinese interlocking per research paper section 5.2.

## Inputs

- `engine/include/poly/presets.h`

## Expected Output

- `presets.h with makeKotekanInterlock()`

## Verification

Build compiles; preset_tests pass
