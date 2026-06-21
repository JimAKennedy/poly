---
id: T03
parent: S02
milestone: M003
key_files:
  - plugin/source/processor.h
  - plugin/source/processor.cpp
  - plugin/source/controller.cpp
  - plugin/source/plugids.h
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-21T03:09:34.838Z
blocker_discovered: false
---

# T03: Updated PolyProcessor to hold SceneState, added kSceneSelect and kSceneMorph parameters, wired plugin integration

**Updated PolyProcessor to hold SceneState, added kSceneSelect and kSceneMorph parameters, wired plugin integration**

## What Happened

Updated PolyProcessor to hold SceneState instead of single GrooveState. Resolves active groove from scene select + morph before passing to resolveMacros. Added kSceneSelect and kSceneMorph parameters to ParamIDs and controller. Wired applyParameter for the new IDs.

## Verification

Build succeeds, all tests pass

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build && ctest --test-dir build` | 0 | pass | 18000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `plugin/source/processor.h`
- `plugin/source/processor.cpp`
- `plugin/source/controller.cpp`
- `plugin/source/plugids.h`
