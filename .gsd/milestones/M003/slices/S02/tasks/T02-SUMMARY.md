---
id: T02
parent: S02
milestone: M003
key_files:
  - engine/include/poly/state_io.h
  - engine/include/poly/types.h
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-21T03:09:30.222Z
blocker_discovered: false
---

# T02: Extended state_io to serialize SceneState, bumped kCurrentStateVersion to 3 with v1/v2 backwards compat

**Extended state_io to serialize SceneState, bumped kCurrentStateVersion to 3 with v1/v2 backwards compat**

## What Happened

Extended state_io to serialize both GrooveState slots plus scene select and morph amount. Bumped kCurrentStateVersion to 3. Added backwards-compatible read path: v1 state loads into sceneA with sceneB as copy, v2 loads similarly.

## Verification

Build succeeds, plugin_tests pass with serialization round-trip and backwards compat

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build && ctest --test-dir build -R plugin` | 0 | pass | 12000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/include/poly/state_io.h`
- `engine/include/poly/types.h`
