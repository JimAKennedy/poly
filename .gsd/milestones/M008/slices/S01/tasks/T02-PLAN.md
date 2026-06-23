---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T02: Engine renderRange additive step timing

Modify renderRange() to use cumulative cell PPQ offsets when cellCount > 0. For each absolute step, compute PPQ as cycleIndex * cyclePpqLength + cumPpq[localStep] instead of absStep * sPpq. Step duration for note length also varies per cell. Equal-cell path (cellCount=0) unchanged.

## Inputs

- `engine/src/engine.cpp`
- `engine/include/poly/types.h`

## Expected Output

- `engine/src/engine.cpp with additive step timing in renderRange()`

## Verification

cmake --build build && ctest --test-dir build (existing golden tests pass unchanged)
