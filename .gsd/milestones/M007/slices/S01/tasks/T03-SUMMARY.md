---
id: T03
parent: S01
milestone: M007
key_files:
  - plugin/source/processor.cpp
  - plugin/source/processor.h
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-22T19:40:06.108Z
blocker_discovered: false
---

# T03: State serialization for phrase params with version bump and backward compat

**State serialization for phrase params with version bump and backward compat**

## What Happened

Updated processor.cpp getState()/setState() to serialize phraseLength, phraseGap, phraseOffset per lane. kStateVersion bumped. setState() branches on version for backward compatibility with old presets. Also added VST3 parameter mapping (normalized 0-1 to 0-64 beats range) in applyParameter().

## Verification

Build compiles; RT safety check passes

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build && ctest --test-dir build` | 0 | pass | 15000ms |
| 2 | `scripts/check-realtime-safety.sh` | 0 | pass | 2000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `plugin/source/processor.cpp`
- `plugin/source/processor.h`
