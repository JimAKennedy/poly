---
id: T01
parent: S01
milestone: M007
key_files:
  - engine/include/poly/types.h
key_decisions:
  - Units are beats (quarter notes) not bars — more idiomatic for polymetric work
  - phraseLength=0 means continuous (no phrase gating) for backward compat
duration: 
verification_result: passed
completed_at: 2026-06-22T19:39:56.992Z
blocker_discovered: false
---

# T01: Added phraseLength, phraseGap, phraseOffset to LaneConfig as beat-based float fields

**Added phraseLength, phraseGap, phraseOffset to LaneConfig as beat-based float fields**

## What Happened

Added three new float fields to LaneConfig in types.h: phraseLength (0=continuous), phraseGap, and phraseOffset, all in beats (PPQ). Default values of 0.0f preserve backward compatibility. Initially implemented as bars, later converted to beats for more idiomatic polymetric control.

## Verification

Build compiles with no warnings; all 197 existing tests pass unchanged with default values

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build && ctest --test-dir build` | 0 | pass | 15000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/include/poly/types.h`
