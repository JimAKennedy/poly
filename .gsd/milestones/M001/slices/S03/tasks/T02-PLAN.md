---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T02: Core cycle timing in renderRange

Implement the core rendering loop: for each active lane, compute step positions from absolute PPQ using cycle length and subdivision, check which steps fall within [ppqStart, ppqEnd], emit NoteEvent for each hit. No probability or velocity modulation yet — just raw Euclidean hits at baseVelocity. Handle block-boundary alignment correctly.

## Inputs

- `engine/include/poly/types.h`
- `engine/include/poly/euclidean.h`

## Expected Output

- `engine/src/engine.cpp`

## Verification

cd build && cmake --build . && ./tools/harness/poly_harness 4 120 0.1 | head -20
