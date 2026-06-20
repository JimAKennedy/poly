---
id: T03
parent: S06
milestone: M002
key_files:
  - plugin/resource/poly.uidesc
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-20T22:19:28.436Z
blocker_discovered: false
---

# T03: Added macro knob row with 6 CKnob controls bound to macro ParamIDs

**Added macro knob row with 6 CKnob controls bound to macro ParamIDs**

## What Happened

6 CKnob controls in uidesc (50x50) bound to macro ParamIDs 200-205 via control-tags. Labels for complexity, density, syncopation, swing, tension, humanize. Colors match jk.digital design system.

## Verification

Build succeeds, knobs bound to correct param IDs

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cd build && cmake --build . && ctest --output-on-failure` | 0 | pass | 5000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `plugin/resource/poly.uidesc`
