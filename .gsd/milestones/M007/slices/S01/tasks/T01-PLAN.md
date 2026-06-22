---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T01: Add phrase fields to LaneConfig and types

Add phraseLength (float, bars, 0=continuous), phraseGap (float, bars, 0=no gap), phraseOffset (float, bars, 0=no offset) to LaneConfig in types.h. phraseLength=0 means continuous (backward compatible). Update kStateVersion.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `engine/include/poly/types.h with new fields`

## Verification

Build compiles with no warnings; existing tests pass unchanged (default values preserve backward compat)
