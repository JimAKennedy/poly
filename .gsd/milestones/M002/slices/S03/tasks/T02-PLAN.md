---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T02: Implement swing offset in renderRange

After computing the note's ppqPosition, check if the cycle-local step index is even (0-indexed, so steps 1, 3, 5... get swung). Shift ppqPosition forward by swingAmount * stepPpq * (1.0/3.0). This creates the classic swing feel. Ensure the shifted position still falls within [ppqStart, ppqEnd).

## Inputs

- `engine/include/poly/types.h`
- `engine/src/engine.cpp`

## Expected Output

- `Modified engine.cpp with swing offset applied to even steps`

## Verification

cd build && cmake --build . && ctest --output-on-failure
