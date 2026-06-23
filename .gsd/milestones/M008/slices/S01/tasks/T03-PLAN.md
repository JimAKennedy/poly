---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T03: State serialization for additive cells (version 10)

Bump kStateVersion to 10. Serialize cellCount and cellSizes array. Version 9 load path defaults cellCount=0 (equal cells, backward compatible).

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `State version 10 with additive cell persistence`

## Verification

cmake --build build && ctest --test-dir build
