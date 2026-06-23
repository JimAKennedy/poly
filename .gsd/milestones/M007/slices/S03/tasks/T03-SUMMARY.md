---
id: T03
parent: S03
milestone: M007
key_files:
  - plugin/source/plugids.h
  - plugin/source/processor.cpp
  - plugin/source/controller.cpp
  - engine/include/poly/state_io.h
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-22T19:55:12.215Z
blocker_discovered: false
---

# T03: Added driftRate parameter (kDriftRate=13) and state version 7 serialization

**Added driftRate parameter (kDriftRate=13) and state version 7 serialization**

## What Happened

Added kDriftRate offset in plugids.h, applyParameter case in processor.cpp mapping normalized [0,1] to [-4,+4] steps/bar (bipolar centered at 0.5), controller registration with default 0.5 (no drift), and v7 state serialization in state_io.h. RT safety check passes — no allocation in process().

## Verification

Build compiles; RT safety check passes; state version correctly bumped to 7

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `scripts/check-realtime-safety.sh` | 0 | pass | 200ms |
| 2 | `cmake --build build` | 0 | pass | 5000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `plugin/source/plugids.h`
- `plugin/source/processor.cpp`
- `plugin/source/controller.cpp`
- `engine/include/poly/state_io.h`
