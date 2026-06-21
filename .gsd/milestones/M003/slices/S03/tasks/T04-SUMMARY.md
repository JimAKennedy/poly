---
id: T04
parent: S03
milestone: M003
key_files:
  - engine/include/poly/state_io.h
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-21T03:10:00.598Z
blocker_discovered: false
---

# T04: Extended state_io for ConstraintConfig fields, bumped state version to v4 with backwards compat

**Extended state_io for ConstraintConfig fields, bumped state version to v4 with backwards compat**

## What Happened

Extended state_io to read/write ConstraintConfig fields (anchorSteps, backbeatProtect, densityMin/densityMax). Bumped state version to v4. Handle backwards compat for older versions: default no anchors, no backbeat protect, density bounds 0/maxSteps.

## Verification

Build succeeds, plugin_tests pass

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build && ctest --test-dir build -R plugin` | 0 | pass | 12000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/include/poly/state_io.h`
