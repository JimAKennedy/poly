---
id: T02
parent: S03
milestone: M003
key_files:
  - engine/src/engine.cpp
  - engine/src/constraint.cpp
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-21T03:09:52.547Z
blocker_discovered: false
---

# T02: Implemented anchor and backbeat constraints: anchor steps force-fire, backbeat positions preserve pre-macro emphasis

**Implemented anchor and backbeat constraints: anchor steps force-fire, backbeat positions preserve pre-macro emphasis**

## What Happened

In renderRange, after probability gate: if step is in anchorSteps, force it to fire (skip probability roll). For backbeat protection: if backbeatProtect is true and step is at a backbeat position, preserve base velocity and emphasisProb from pre-macro values. Applied after macro resolution but before envelope modulation.

## Verification

Build succeeds, constraint_tests pass

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build && ctest --test-dir build -R constraint` | 0 | pass | 12000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/src/engine.cpp`
- `engine/src/constraint.cpp`
