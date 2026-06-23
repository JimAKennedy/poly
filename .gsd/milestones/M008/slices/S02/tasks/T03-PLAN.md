---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T03: Serialize timeline fields and expose scalar parameters

Add timeline, fixedPattern, fixedPatternLength to state serialization (version 10, same bump as S01). Add VST3 controller parameters for timeline toggle and fixedPatternLength ONLY. fixedPattern array is NOT exposed as individual VST3 params — it is state-serialized only, edited through the Timeline Step Editor (T05). Version 9 load defaults timeline=false, fixedPatternLength=0.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `timeline toggle and fixedPatternLength visible in Cubase generic editor`

## Verification

cmake --build build && ctest --test-dir build
