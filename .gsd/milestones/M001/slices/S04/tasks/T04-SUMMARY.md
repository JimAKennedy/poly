---
id: T04
parent: S04
milestone: M001
key_files:
  - docs/automation-mapping.md
key_decisions: []
duration: 
verification_result: untested
completed_at: 2026-06-20T20:14:31.712Z
blocker_discovered: false
---

# T04: Wrote docs/automation-mapping.md specifying ParamID ranges, exposed vs internal parameters, and normalized/plain conversion conventions.

**Wrote docs/automation-mapping.md specifying ParamID ranges, exposed vs internal parameters, and normalized/plain conversion conventions.**

## What Happened

Defined the ParamID layout (0-99 global, 100-199 macros, 1000+ per-lane with 800-stride formula). Documented 13 automatable parameters (1 global, 6 macros, 7 per-lane) and explained why structural parameters (cycle steps, hit count, rotation, seed) are excluded from automation. Included normalized↔plain conversion formulas and thread safety notes.

## Verification

ParamID ranges don't conflict. Exposed vs internal distinction is clear with rationale for each exclusion.

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| — | No verification commands discovered | — | — | — |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `docs/automation-mapping.md`
