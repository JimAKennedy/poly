---
id: T02
parent: S01
milestone: M006
key_files:
  - CMakeLists.txt
  - tests/CMakeLists.txt
key_decisions:
  - Linked sdk_hosting instead of manually compiling hostclasses.cpp — avoids vtable/dependency issues
  - moduleHandle symbol provided in each test binary since macmain.cpp is not linked into test targets
duration: 
verification_result: passed
completed_at: 2026-06-21T00:59:57.784Z
blocker_discovered: false
---

# T02: Wired CMake targets poly_visual_tests and poly_interaction_tests with proper SDK/VSTGUI linking

**Wired CMake targets poly_visual_tests and poly_interaction_tests with proper SDK/VSTGUI linking**

## What Happened

Added BUILD_VISUAL_TESTS and BUILD_INTERACTION_TESTS options to root CMakeLists.txt. Created both test targets in tests/CMakeLists.txt with: plugin source compilation, harness integration via jk_add_visual_tests/jk_add_interaction_tests, linking to sdk/vstgui_support/sdk_hosting, and UIDESC_RESOURCE_DIR definition for interaction tests. Engine-only tests remain independent with zero SDK dependency.

## Verification

Both targets compile and link. All 105 tests pass (101 engine + 4 UI).

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `ctest --test-dir build --output-on-failure` | 0 | 105/105 tests pass | 460ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `CMakeLists.txt`
- `tests/CMakeLists.txt`
