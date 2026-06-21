---
id: T01
parent: S01
milestone: M006
key_files:
  - tests/ui/visual/visual_test_harness.h
  - tests/ui/visual/visual_test_harness.cpp
  - tests/ui/visual/image_compare.h
  - tests/ui/visual/image_compare.cpp
  - tests/ui/interaction/headless_ui_host.h
  - tests/ui/interaction/headless_ui_host.cpp
  - cmake/jk_ui_test_harness.cmake
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-21T00:59:49.799Z
blocker_discovered: false
---

# T01: Copied all 6 harness files from audio-meta and the adapted CMake module into Poly's test tree

**Copied all 6 harness files from audio-meta and the adapted CMake module into Poly's test tree**

## What Happened

Copied visual_test_harness.h/cpp, image_compare.h/cpp into tests/ui/visual/, headless_ui_host.h/cpp into tests/ui/interaction/, and jk_ui_test_harness.cmake into cmake/. Adapted the cmake module to use separate INTERACTION_TEST_HARNESS_DIR for the interaction harness files since they live in a different directory from the visual harness.

## Verification

All files exist at correct paths, no syntax errors confirmed by successful compilation of both targets.

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build --target poly_visual_tests` | 0 | pass | 5000ms |
| 2 | `cmake --build build --target poly_interaction_tests` | 0 | pass | 3000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `tests/ui/visual/visual_test_harness.h`
- `tests/ui/visual/visual_test_harness.cpp`
- `tests/ui/visual/image_compare.h`
- `tests/ui/visual/image_compare.cpp`
- `tests/ui/interaction/headless_ui_host.h`
- `tests/ui/interaction/headless_ui_host.cpp`
- `cmake/jk_ui_test_harness.cmake`
