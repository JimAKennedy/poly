---
id: T03
parent: S02
milestone: M002
key_files:
  - engine/src/engine.cpp
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-20T21:27:08.088Z
blocker_discovered: false
---

# T03: Integrated envelope modulation into renderRange — velocity multiplicative, density additive

**Integrated envelope modulation into renderRange — velocity multiplicative, density additive**

## What Happened

In renderRange, after pattern check and before probability gate: evaluate all active per-lane and global envelopes. Velocity envelopes apply multiplicatively: velMod *= (1 - depth*(1-value)). Density envelopes modify probability additively: probMod += depth*(value*2-1). Velocity modulation applies after accent/ghost floor, before final clamp. Density modulation adjusts effective probability before the gate.

## Verification

Build succeeds, 8 integration tests verify: no-envelope baseline, sine velocity modulation, zero-depth no-effect, inactive envelope ignored, density modulation changes note count, global envelope applied, multiple envelopes multiplicative, determinism

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build && ctest --output-on-failure` | 0 | pass | 3000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/src/engine.cpp`
