---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T03: State serialization for mutation params

Update processor.cpp to serialize mutationRate per lane. Bump state version if not already bumped by S01.

## Inputs

- `plugin/source/processor.cpp`

## Expected Output

- `processor.cpp with mutation serialization`

## Verification

Build compiles; RT safety check passes
