---
id: T02
parent: S04
milestone: M007
key_files:
  - engine/src/engine.cpp
key_decisions:
  - Clamp timing offset to 0.0 instead of tc.ppqStart to preserve block-size independence
duration: 
verification_result: passed
completed_at: 2026-06-22T21:05:14.013Z
blocker_discovered: false
---

# T02: Applied timing offset in renderRange after swing and humanize

**Applied timing offset in renderRange after swing and humanize**

## What Happened

Added timing offset application in engine.cpp renderRange() after swing and humanize adjustments. Converts ms to PPQ using tempo (offsetPpq = timingOffsetMs * tempo / 60000.0). Clamps to 0.0 (not block start) to preserve block-size independence — clamping to tc.ppqStart would make output depend on block boundaries.

## Verification

Build + existing tests pass; offset 0 produces identical output verified by golden test

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build && ctest --test-dir build` | 0 | 211/211 pass | 4500ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/src/engine.cpp`
