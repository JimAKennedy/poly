---
estimated_steps: 1
estimated_files: 4
skills_used: []
---

# T03: Scene plugin integration

Update PolyProcessor to hold SceneState instead of single GrooveState. Resolve active groove from scene select + morph before passing to resolveMacros. Add kSceneSelect and kSceneMorph parameters to ParamIDs and controller. Wire applyParameter for the new IDs.

## Inputs

- `plugin/source/processor.cpp`
- `plugin/source/plugids.h`

## Expected Output

- `plugin/source/processor.cpp`
- `plugin/source/controller.cpp`
- `plugin/source/plugids.h`

## Verification

cmake --build build && ctest --test-dir build
