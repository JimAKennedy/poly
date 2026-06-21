---
id: S01
parent: M006
milestone: M006
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - tests/ui/visual/visual_test_harness.h
  - tests/ui/visual/visual_smoke_tests.cpp
  - tests/ui/interaction/headless_ui_host.h
  - tests/ui/interaction/interaction_smoke_tests.cpp
  - cmake/jk_ui_test_harness.cmake
  - tests/CMakeLists.txt
key_decisions:
  - Linked sdk_hosting target instead of manually compiling hostclasses.cpp
  - Separate INTERACTION_TEST_HARNESS_DIR variable in cmake module for cleaner directory layout
  - Construct PolyController directly (not via createInstance) to avoid VST3 diamond-inheritance ambiguous cast
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-21T01:00:27.348Z
blocker_discovered: false
---

# S01: Harness Integration and Smoke Tests

**Adopted audio-meta UI test harness into Poly with 4 passing smoke tests proving both rendering and headless host pipelines work**

## What Happened

Copied 6 harness files from audio-meta/test_harness/ into tests/ui/visual/ and tests/ui/interaction/. Adapted the CMake module to support separate directory variables for visual and interaction harness files. Wired two new CMake targets (poly_visual_tests, poly_interaction_tests) with proper SDK/VSTGUI/sdk_hosting linking. Wrote 4 smoke tests: 2 visual tests that render LaneGridView and VelocityView to offscreen bitmaps, and 2 interaction tests that verify HeadlessUIHost lifecycle and control discovery with PolyController. Resolved a HostApplication constructor segfault by linking sdk_hosting instead of manually compiling hostclasses.cpp, and an ambiguous cast issue by constructing PolyController directly. All 105 tests pass (101 engine + 4 UI).

## Verification

105/105 CTest tests pass. Visual tests produce output PNGs in build/tests/visual_output/. Interaction tests successfully open/close the headless host and find controls by tag.

## Requirements Advanced

None.

## Requirements Validated

None.

## New Requirements Surfaced

None.

## Requirements Invalidated or Re-scoped

None.

## Operational Readiness

None.

## Deviations

None.

## Known Limitations

None.

## Follow-ups

None.

## Files Created/Modified

None.
