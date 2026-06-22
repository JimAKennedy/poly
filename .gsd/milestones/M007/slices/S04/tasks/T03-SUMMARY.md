---
id: T03
parent: S04
milestone: M007
key_files:
  - engine/include/poly/state_io.h
  - plugin/source/plugids.h
  - plugin/source/controller.cpp
  - plugin/source/processor.cpp
  - tests/golden_tests.cpp
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-22T21:05:25.391Z
blocker_discovered: false
---

# T03: Added state serialization (v8), parameter registration, and 5 golden tests for timing offset

**Added state serialization (v8), parameter registration, and 5 golden tests for timing offset**

## What Happened

Bumped kCurrentStateVersion to 8. Added timingOffsetMs serialization in state_io.h (read/write at version >= 8). Registered kTimingOffset parameter (offset 14, bumped kLaneParamCount to 15) in controller.cpp. Added applyParameter case in processor.cpp mapping normalized [0,1] to [-20,+20] ms. Added 5 golden tests: ZeroMatchesBaseline, PositiveShiftsLater, NegativeShiftsEarlier, InteractsWithSwing, BlockSizeIndependence.

## Verification

All 211 tests pass including 5 new timing offset golden tests. RT safety check passes.

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `ctest --test-dir build -R GoldenTimingOffset` | 0 | 5/5 pass | 20ms |
| 2 | `ctest --test-dir build` | 0 | 211/211 pass | 1460ms |
| 3 | `scripts/check-realtime-safety.sh` | 0 | pass | 500ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/include/poly/state_io.h`
- `plugin/source/plugids.h`
- `plugin/source/controller.cpp`
- `plugin/source/processor.cpp`
- `tests/golden_tests.cpp`
