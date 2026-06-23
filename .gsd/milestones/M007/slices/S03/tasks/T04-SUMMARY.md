---
id: T04
parent: S03
milestone: M007
key_files:
  - tests/golden_tests.cpp
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-22T19:55:20.915Z
blocker_discovered: false
---

# T04: Added 5 golden tests for drift: zero-rate baseline, one-step-per-bar, phasing, transport jump, block independence

**Added 5 golden tests for drift: zero-rate baseline, one-step-per-bar, phasing, transport jump, block independence**

## What Happened

Tests 26-30 cover: driftRate=0 matches baseline; driftRate=1.0 produces different output and is deterministic; two lanes with different drift rates show phase divergence across bars; transport jump to bar 4 with drift=0.5 matches straight-through; drift deterministic across block sizes (0.05, 0.5, 2.0 PPQ).

## Verification

ctest --test-dir build -R Golden passes all drift tests

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `ctest --test-dir build` | 0 | 206/206 pass including 5 new drift tests | 1530ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `tests/golden_tests.cpp`
