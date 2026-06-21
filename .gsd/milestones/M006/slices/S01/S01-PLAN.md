# S01: Harness Integration and Smoke Tests

**Goal:** Copy harness files from audio-meta, wire CMake targets, write smoke tests proving both rendering pipeline and headless host work
**Demo:** poly_visual_tests and poly_interaction_tests targets build and run, smoke tests pass proving offscreen rendering and headless host work

## Must-Haves

- Complete the planned slice outcomes.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Copy harness files and CMake module** `est:15min`
  Copy visual_test_harness.h/cpp, image_compare.h/cpp, headless_ui_host.h/cpp from audio-meta/test_harness/ into tests/ui/visual/ and tests/ui/interaction/. Copy jk_ui_test_harness.cmake into cmake/. Adapt include paths and namespaces if needed.
  - Files: `tests/ui/visual/visual_test_harness.h`, `tests/ui/visual/visual_test_harness.cpp`, `tests/ui/visual/image_compare.h`, `tests/ui/visual/image_compare.cpp`, `tests/ui/interaction/headless_ui_host.h`, `tests/ui/interaction/headless_ui_host.cpp`, `cmake/jk_ui_test_harness.cmake`
  - Verify: All files exist at correct paths with no syntax errors

- [x] **T02: Wire CMake targets for UI tests** `est:20min`
  Add BUILD_VISUAL_TESTS and BUILD_INTERACTION_TESTS options to root CMakeLists.txt. Create test targets poly_visual_tests and poly_interaction_tests using jk_ui_test_harness.cmake functions. Link against GTest, VSTGUI, VST3 SDK. Ensure engine-only tests remain independent.
  - Files: `CMakeLists.txt`, `tests/CMakeLists.txt`
  - Verify: cmake --build build --target poly_visual_tests && cmake --build build --target poly_interaction_tests both succeed

- [x] **T03: Write visual rendering smoke test** `est:15min`
  Write a smoke test that creates a LaneGridView with a default LaneConfig, calls renderViewToBitmap(), asserts non-null bitmap, and saves output PNG. Verifies the offscreen rendering pipeline works end-to-end.
  - Files: `tests/ui/visual/visual_smoke_tests.cpp`
  - Verify: ctest --test-dir build -R visual_smoke --output-on-failure passes

- [x] **T04: Write headless host smoke test** `est:15min`
  Write a smoke test that creates HeadlessUIHost with PolyController factory, calls open(), asserts success, reads a parameter value, then closes. Verifies the headless host lifecycle works.
  - Files: `tests/ui/interaction/interaction_smoke_tests.cpp`
  - Verify: ctest --test-dir build -R interaction_smoke --output-on-failure passes

## Files Likely Touched

- tests/ui/visual/visual_test_harness.h
- tests/ui/visual/visual_test_harness.cpp
- tests/ui/visual/image_compare.h
- tests/ui/visual/image_compare.cpp
- tests/ui/interaction/headless_ui_host.h
- tests/ui/interaction/headless_ui_host.cpp
- cmake/jk_ui_test_harness.cmake
- CMakeLists.txt
- tests/CMakeLists.txt
- tests/ui/visual/visual_smoke_tests.cpp
- tests/ui/interaction/interaction_smoke_tests.cpp
