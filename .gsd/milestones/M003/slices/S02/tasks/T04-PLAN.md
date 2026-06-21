---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T04: Scene tests

Unit tests for interpolateGrooveState at t=0, t=0.5, t=1 boundaries. Verify discrete field snapping. Verify serialization round-trip for v2 format. Verify v1 backwards compatibility loading.

## Inputs

- `engine/include/poly/scene.h`

## Expected Output

- `tests/scene_tests.cpp`

## Verification

cmake --build build && ctest --test-dir build -R scene
