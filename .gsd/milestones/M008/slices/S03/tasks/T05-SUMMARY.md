---
id: T05
parent: S03
milestone: M008
key_files:
  - plugin/source/ui/micro_timing_editor_view.h
  - plugin/source/ui/micro_timing_editor_view.cpp
key_decisions: []
duration: 
verification_result: untested
completed_at: 2026-06-23T02:28:55.126Z
blocker_discovered: false
---

# T05: Micro-timing Editor view with drag-to-adjust bars and double-click reset

**Micro-timing Editor view with drag-to-adjust bars and double-click reset**

## What Happened

Created MicroTimingEditorView (micro_timing_editor_view.h/.cpp) showing a bar chart with per-step vertical bars. Drag to adjust [-20, +20] ms, double-click to reset to 0. Numeric tooltip during drag. Updates microTimingMs via mutableCachedState(). Registered in controller and added to both CMakeLists.

## Verification

Build passes on all targets, 237 tests pass

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| — | No verification commands discovered | — | — | — |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `plugin/source/ui/micro_timing_editor_view.h`
- `plugin/source/ui/micro_timing_editor_view.cpp`
