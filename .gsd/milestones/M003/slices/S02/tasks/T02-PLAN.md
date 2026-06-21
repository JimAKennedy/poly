---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T02: Scene state serialization

Extend state_io to serialize SceneState (both GrooveStates plus scene select and morph). Bump kCurrentStateVersion to 2. Add backwards-compat read path that loads v1 state into sceneA and initializes sceneB as a copy.

## Inputs

- `engine/include/poly/state_io.h`

## Expected Output

- `engine/include/poly/state_io.h`

## Verification

cmake --build build && ctest --test-dir build -R plugin
