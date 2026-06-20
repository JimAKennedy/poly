---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T03: Implement ghost floor velocity

After all velocity calculations (base + spread + accent), clamp the final velocity so it never falls below ghostFloor/127.0f. This ensures even quiet notes maintain a minimum presence. Apply before the final clamp to [0, 1].

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `Modified engine.cpp with ghost floor clamping`
- `Ghost floor unit tests`

## Verification

cd build && cmake --build . && ctest --output-on-failure
