---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T03: State serialization for phrase params

Update processor.cpp getState()/setState() to serialize phraseLength, phraseGap, phraseOffset per lane. Bump kStateVersion. Branch on version in setState() for backward compat with old presets.

## Inputs

- `plugin/source/processor.cpp`

## Expected Output

- `processor.cpp with phrase serialization`

## Verification

Build compiles; RT safety check passes
