---
id: T02
parent: S01
milestone: M007
key_files:
  - engine/src/engine.cpp
key_decisions:
  - Phrase phase derived from absolute PPQ (not accumulated) per project convention
  - Beats map directly to PPQ (no 4x multiplier needed)
duration: 
verification_result: passed
completed_at: 2026-06-22T19:40:02.957Z
blocker_discovered: false
---

# T02: Implemented phrase gating in renderRange using absolute PPQ position

**Implemented phrase gating in renderRange using absolute PPQ position**

## What Happened

Added phrase gating logic to engine.cpp renderRange(). Computes phrase cycle from phraseLength+phraseGap, derives phase position from absolute PPQ via fmod (never accumulates). When phrasePos >= phraseLenPpq, the step is in the gap region and note output is suppressed. phraseLength=0 is a no-op (continuous). Offset shifts the entire pattern via phraseOffPpq subtraction before fmod. Negative fmod results are wrapped correctly.

## Verification

Build + all tests pass; phrase gating derives from absolute PPQ, no accumulation

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build && ctest --test-dir build` | 0 | pass | 15000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/src/engine.cpp`
