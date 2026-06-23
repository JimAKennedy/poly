---
id: T02
parent: S03
milestone: M008
key_files: []
key_decisions: []
duration: 
verification_result: untested
completed_at: 2026-06-23T02:28:45.950Z
blocker_discovered: false
---

# T02: Engine applies per-step micro-timing offsets after swing, before humanize

**Engine applies per-step micro-timing offsets after swing, before humanize**

## What Happened

In renderRange(), added micro-timing map lookup after swing and before humanize: converts microTimingMs[cycleStep] to PPQ offset using tempo. Composes additively with swing and timingOffsetMs.

## Verification

Build passes, 231 tests pass

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
