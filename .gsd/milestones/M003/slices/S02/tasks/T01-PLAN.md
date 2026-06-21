---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T01: Scene data model and interpolation

Add SceneState struct containing two GrooveState slots (sceneA, sceneB), a scene select enum (A/B/Morph), and a morph amount (0.0-1.0). Implement interpolateGrooveState(a, b, t) that lerps all numeric LaneConfig fields, MacroValues, and envelope parameters. Discrete fields (role, midiNote, cycle) snap at t=0.5.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `engine/include/poly/scene.h`
- `engine/src/scene.cpp`

## Verification

cmake --build build && ctest --test-dir build
