---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T02: Design and implement Reich Phasing preset

Create preset with 2 lanes: identical E(5,12) pattern, same MIDI note, one with driftRate=0.25 (shifts 1 step every 4 bars). Demonstrates gradual phase separation creating resultant patterns per research paper section 8.2.

## Inputs

- `engine/include/poly/presets.h`

## Expected Output

- `presets.h with makeReichPhasing()`

## Verification

Build compiles; preset_tests pass
