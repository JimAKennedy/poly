---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T02: Wire CMake targets for UI tests

Add BUILD_VISUAL_TESTS and BUILD_INTERACTION_TESTS options to root CMakeLists.txt. Create test targets poly_visual_tests and poly_interaction_tests using jk_ui_test_harness.cmake functions. Link against GTest, VSTGUI, VST3 SDK. Ensure engine-only tests remain independent.

## Inputs

- `cmake/jk_ui_test_harness.cmake`
- `tests/CMakeLists.txt`

## Expected Output

- `poly_visual_tests target`
- `poly_interaction_tests target`

## Verification

cmake --build build --target poly_visual_tests && cmake --build build --target poly_interaction_tests both succeed
