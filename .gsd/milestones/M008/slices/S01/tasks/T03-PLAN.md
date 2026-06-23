---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T03: State serialization for additive cells (version 10)

Bump kStateVersion to 10. Serialize cellCount and cellSizes array. Version 9 load path defaults cellCount=0 (equal cells, backward compatible).

## Inputs

- `plugin/source/processor.cpp`

## Expected Output

- `processor.cpp with version 10 serialization`

## Verification

cmake --build build && ctest --test-dir build
