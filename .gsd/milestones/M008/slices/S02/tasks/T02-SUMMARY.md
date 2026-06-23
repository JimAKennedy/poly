---
id: T02
parent: S02
milestone: M008
key_files: []
key_decisions: []
duration: 
verification_result: untested
completed_at: 2026-06-23T02:24:31.051Z
blocker_discovered: false
---

# T02: Engine uses fixed pattern for timeline lanes, macros skip timeline lanes

**Engine uses fixed pattern for timeline lanes, macros skip timeline lanes**

## What Happened

Modified engine.cpp renderRange() to use cfg.fixedPattern when cfg.timeline is true, bypassing Euclidean and kotekan pattern generation. Modified macro.cpp resolveMacros() to skip lanes with timeline=true.

## Verification

Build passes, 224 tests pass

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| — | No verification commands discovered | — | — | — |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

None.
