---
id: T04
parent: S01
milestone: M007
key_files:
  - tests/golden_tests.cpp
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-22T19:40:10.188Z
blocker_discovered: false
---

# T04: Golden tests covering phrase gating, multi-lane phrases, transport jump, loop restart, and continuous mode

**Golden tests covering phrase gating, multi-lane phrases, transport jump, loop restart, and continuous mode**

## What Happened

Added golden test scenarios: (1) Single lane with phraseLength=8bt, phraseGap=4bt verifying silence during gap, (2) Two lanes with different phrase lengths for offset behavior, (3) Phrase with transport jump into gap region, (4) Loop restart determinism, (5) phraseLength=0 producing identical output to baseline. Tests updated when switching from bars to beats — values multiplied by 4 to maintain same PPQ timing.

## Verification

ctest --test-dir build -R golden passes all phrase scenarios

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `ctest --test-dir build -R golden` | 0 | pass | 5000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `tests/golden_tests.cpp`
