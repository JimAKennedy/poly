---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T03: Serialize micro-timing map (state-only, no VST3 params)

Add microTimingMs array to state serialization (version 10, same bump as S01/S02). Do NOT expose individual microTimingMs values as VST3 parameters — the array is state-serialized only, edited through the Micro-timing Editor view (T05). Version 9 load defaults to all-zeros array. No new controller parameters in this task.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `microTimingMs persisted in state, backward-compatible load from v9`

## Verification

cmake --build build && ctest --test-dir build
