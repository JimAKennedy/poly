---
id: T02
parent: S01
milestone: M008
key_files:
  - engine/src/engine.cpp
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-23T02:06:44.958Z
blocker_discovered: false
---

# T02: Engine renderRange now computes step PPQ from cumulative additive cell offsets when cellCount > 0

**Engine renderRange now computes step PPQ from cumulative additive cell offsets when cellCount > 0**

## What Happened

Modified renderRange() to use computeAdditiveCells() for step iteration when isAdditive. Each step's PPQ is computed as cycleIdx*totalPpq + cumPpq[localCell]. Step duration varies per cell. Swing and note duration use the per-step duration. Equal-cell path (cellCount=0) is completely unchanged.

## Verification

Build succeeds, 216/216 tests pass unchanged (equal-cell regression confirmed)

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build && ctest --test-dir build` | 0 | pass | 12000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/src/engine.cpp`
