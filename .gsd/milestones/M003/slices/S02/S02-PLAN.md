# S02: Scene and Snapshot System

**Goal:** Add A/B scene system with crossfade morph so users can design two groove states and smoothly transition between them
**Demo:** User stores two scenes, switches instantly, and morphs between them with a crossfader producing smooth musical transitions

## Must-Haves

- Two GrooveState slots (A/B) stored and switchable. Morph parameter (0.0=A, 1.0=B) interpolates all lane parameters. Scene select and morph are automatable in Cubase. State serialization preserves both scenes.

## Proof Level

- This slice proves: Unit tests for interpolation, serialization round-trip, and parameter mapping; manual Cubase QA for automation morph

## Integration Closure

Requires state version bump (kCurrentStateVersion 1 to 2). Processor owns scene state. Controller exposes 2 new params.

## Verification

- Scene index readable via output parameter for UI feedback.

## Tasks

- [x] **T01: Scene data model and interpolation** `est:2h`
  Add SceneState struct containing two GrooveState slots (sceneA, sceneB), a scene select enum (A/B/Morph), and a morph amount (0.0-1.0). Implement interpolateGrooveState(a, b, t) that lerps all numeric LaneConfig fields, MacroValues, and envelope parameters. Discrete fields (role, midiNote, cycle) snap at t=0.5.
  - Files: `engine/include/poly/types.h`, `engine/include/poly/scene.h`, `engine/src/scene.cpp`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T02: Scene state serialization** `est:1h`
  Extend state_io to serialize SceneState (both GrooveStates plus scene select and morph). Bump kCurrentStateVersion to 2. Add backwards-compat read path that loads v1 state into sceneA and initializes sceneB as a copy.
  - Files: `engine/include/poly/state_io.h`, `engine/include/poly/types.h`
  - Verify: cmake --build build && ctest --test-dir build -R plugin

- [x] **T03: Scene plugin integration** `est:1.5h`
  Update PolyProcessor to hold SceneState instead of single GrooveState. Resolve active groove from scene select + morph before passing to resolveMacros. Add kSceneSelect and kSceneMorph parameters to ParamIDs and controller. Wire applyParameter for the new IDs.
  - Files: `plugin/source/processor.h`, `plugin/source/processor.cpp`, `plugin/source/controller.cpp`, `plugin/source/plugids.h`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T04: Scene tests** `est:1h`
  Unit tests for interpolateGrooveState at t=0, t=0.5, t=1 boundaries. Verify discrete field snapping. Verify serialization round-trip for v2 format. Verify v1 backwards compatibility loading.
  - Files: `tests/scene_tests.cpp`
  - Verify: cmake --build build && ctest --test-dir build -R scene

## Files Likely Touched

- engine/include/poly/types.h
- engine/include/poly/scene.h
- engine/src/scene.cpp
- engine/include/poly/state_io.h
- plugin/source/processor.h
- plugin/source/processor.cpp
- plugin/source/controller.cpp
- plugin/source/plugids.h
- tests/scene_tests.cpp
