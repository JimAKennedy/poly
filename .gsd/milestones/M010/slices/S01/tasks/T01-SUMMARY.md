---
id: T01
parent: S01
milestone: M010
key_files:
  - plugin/source/plugids.h
  - plugin/source/processor.cpp
  - plugin/source/controller.cpp
key_decisions:
  - Used separate ID block starting at 800 instead of expanding kParamsPerLane — avoids breaking existing automation and presets
duration: 
verification_result: passed
completed_at: 2026-06-23T01:26:38.647Z
blocker_discovered: false
---

# T01: Added 5 core Euclidean per-lane VST3 parameters in a separate ID block (800+)

**Added 5 core Euclidean per-lane VST3 parameters in a separate ID block (800+)**

## What Happened

Added kCoreSteps, kCoreSubdivision, kCoreHits, kCoreRotation, kCoreMidiNote as new param offsets in plugids.h using a separate ID block starting at 800 to avoid breaking existing param IDs. Wired applyParameter in processor.cpp to map normalized values to engine LaneConfig fields. Registered all 40 params (5 per lane × 8 lanes) in controller.cpp with appropriate ranges and defaults. Added setComponentState sync for all new params.

## Verification

cmake --build build && ctest --test-dir build — 216/216 tests pass

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
