---
id: T04
parent: S03
milestone: M002
key_files:
  - engine/src/engine.cpp
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-20T21:33:44.883Z
blocker_discovered: false
---

# T04: Replaced hardcoded duration with configurable noteDuration, falling back to stepPpq*0.5

**Replaced hardcoded duration with configurable noteDuration, falling back to stepPpq*0.5**

## What Happened

Changed ev.duration from hardcoded sPpq*0.5 to cfg.noteDuration when > 0, otherwise falling back to sPpq*0.5. Allows per-lane control of note length from staccato to legato.

## Verification

Tests verify default half-step duration, custom duration (0.75), and short staccato (0.1)

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `./build/tests/poly_tests --gtest_filter=SwingHumanize.*Duration*` | 0 | 3 duration tests passed | 1ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/src/engine.cpp`
