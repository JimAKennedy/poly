---
id: T03
parent: S02
milestone: M001
key_files: []
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-20T19:56:55.973Z
blocker_discovered: false
---

# T03: Confirmed zero VST3 SDK dependency in poly_engine and poly_harness

**Confirmed zero VST3 SDK dependency in poly_engine and poly_harness**

## What Happened

Verified engine isolation three ways: (1) nm on libpoly_engine.a shows no Steinberg symbols, (2) nm on poly_harness binary shows no Steinberg symbols, (3) grep across engine/ source finds no VST3/Steinberg includes. The engine compiles, links, and runs completely independently of the VST3 SDK.

## Verification

nm and grep checks all returned empty (exit 1 = no matches).

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `nm build/engine/libpoly_engine.a | grep -i steinberg` | 1 | pass — no Steinberg symbols | 50ms |
| 2 | `nm build/tools/harness/poly_harness | grep -i steinberg` | 1 | pass — no Steinberg symbols | 50ms |
| 3 | `grep -r steinberg/vst3/pluginterfaces engine/ -i` | 1 | pass — no VST3 includes in engine source | 30ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

None.
