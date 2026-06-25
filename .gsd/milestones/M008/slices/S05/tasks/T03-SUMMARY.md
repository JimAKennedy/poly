---
id: T03
parent: S05
milestone: M008
key_files:
  - plugin/resource/poly.uidesc
  - plugin/source/controller.cpp
  - plugin/CMakeLists.txt
  - tests/CMakeLists.txt
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-23T18:33:01.301Z
blocker_discovered: false
---

# T03: Registered CrossRhythmView in uidesc, controller, and CMakeLists

**Registered CrossRhythmView in uidesc, controller, and CMakeLists**

## What Happened

Added CrossRhythmView to poly.uidesc at origin (10, 656) with size 580x146, below the PhaseAlignmentView. Updated window size from 659 to 810 to accommodate. Added createCustomView handler in controller.cpp with ownership-transfer annotation. Added source files to plugin/CMakeLists.txt and tests/CMakeLists.txt POLY_PLUGIN_SOURCES.

## Verification

cmake --build build — plugin and all test targets link successfully.

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build` | 0 | pass | 1200ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `plugin/resource/poly.uidesc`
- `plugin/source/controller.cpp`
- `plugin/CMakeLists.txt`
- `tests/CMakeLists.txt`
