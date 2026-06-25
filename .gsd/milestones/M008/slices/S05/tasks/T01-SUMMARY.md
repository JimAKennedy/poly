---
id: T01
parent: S05
milestone: M008
key_files:
  - plugin/source/ui/cross_rhythm_view.h
  - plugin/source/ui/cross_rhythm_view.cpp
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-23T18:32:49.868Z
blocker_discovered: false
---

# T01: Designed cross-rhythm view: linear timeline with lane rows and convergence markers

**Designed cross-rhythm view: linear timeline with lane rows and convergence markers**

## What Happened

Designed a linear timeline view where each active lane gets a horizontal row with tick markers at step/cell boundaries. Convergence points (where 2+ lanes share a cycle boundary within tolerance) are highlighted with gold diamond markers and vertical highlight lines. View auto-scales display span using LCM of cycle lengths, capped at 8 bars. Reads full lane configs from PolyController::cachedState() for accurate additive cell rendering.

## Verification

Design review — approach validated against PhaseAlignmentView and CellEditorView patterns.

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `design review` | 0 | pass | 0ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `plugin/source/ui/cross_rhythm_view.h`
- `plugin/source/ui/cross_rhythm_view.cpp`
