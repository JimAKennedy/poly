---
id: T05
parent: S01
milestone: M008
key_files:
  - plugin/source/plugids.h
  - plugin/source/processor.cpp
  - plugin/source/controller.cpp
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-23T02:11:01.952Z
blocker_discovered: false
---

# T05: Exposed cellCount as VST3 core param; wired in processor applyParameter and controller state restore

**Exposed cellCount as VST3 core param; wired in processor applyParameter and controller state restore**

## What Happened

Added kCoreCellCount=5 to plugids.h core params. Added Cell Count param (0-64 steps, default 0) to controller. Wired applyParameter in processor to set cfg.cellCount. Added state restore in setComponentState. cellSizes array intentionally NOT exposed as VST3 params — edited through custom UI view only.

## Verification

Build succeeds, 224/224 tests pass

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build && ctest --test-dir build` | 0 | pass | 15000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `plugin/source/plugids.h`
- `plugin/source/processor.cpp`
- `plugin/source/controller.cpp`
