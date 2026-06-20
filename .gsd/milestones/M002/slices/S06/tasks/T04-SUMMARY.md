---
id: T04
parent: S06
milestone: M002
key_files:
  - plugin/source/ui/velocity_view.h
  - plugin/source/ui/velocity_view.cpp
  - plugin/source/processor.cpp
  - plugin/source/plugids.h
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-20T22:19:32.693Z
blocker_discovered: false
---

# T04: Implemented VelocityView with per-lane velocity bars via RT-safe outputParameterChanges

**Implemented VelocityView with per-lane velocity bars via RT-safe outputParameterChanges**

## What Happened

CView subclass draws vertical bars per lane showing last-hit velocity. Processor writes velocity via outputParameterChanges (RT-safe, no allocation). Per-lane read-only params (400-407) registered in controller. Velocity bar uses design system green with velocity-scaled alpha.

## Verification

Build succeeds, 101/101 tests pass

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cd build && cmake --build . && ctest --output-on-failure` | 0 | pass | 5000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `plugin/source/ui/velocity_view.h`
- `plugin/source/ui/velocity_view.cpp`
- `plugin/source/processor.cpp`
- `plugin/source/plugids.h`
