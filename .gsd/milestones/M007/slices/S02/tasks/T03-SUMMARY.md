---
id: T03
parent: S02
milestone: M007
key_files:
  - engine/include/poly/state_io.h
  - plugin/source/plugids.h
  - plugin/source/processor.cpp
  - plugin/source/controller.cpp
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-22T19:48:29.571Z
blocker_discovered: false
---

# T03: Added mutationRate serialization (state version 6) and parameter wiring

**Added mutationRate serialization (state version 6) and parameter wiring**

## What Happened

Bumped kCurrentStateVersion from 5 to 6. Added mutationRate read/write in state_io.h version >= 6 blocks. Added kMutationRate = 12 parameter ID in plugids.h (bumped kLaneParamCount to 13). Wired parameter handling in processor.cpp applyParameter and controller.cpp (parameter registration + setComponentState restore). Older presets (version <= 5) deserialize correctly with default mutationRate=0.

## Verification

Build compiles; RT safety check passes; all tests pass

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `scripts/check-realtime-safety.sh` | 0 | pass | 500ms |
| 2 | `cmake --build build && ctest --test-dir build` | 0 | 201/201 tests pass | 9500ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/include/poly/state_io.h`
- `plugin/source/plugids.h`
- `plugin/source/processor.cpp`
- `plugin/source/controller.cpp`
