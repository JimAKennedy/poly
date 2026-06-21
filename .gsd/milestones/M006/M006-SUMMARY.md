---
id: M006
title: "UI Test Framework Adoption"
status: complete
completed_at: 2026-06-21T01:08:19.753Z
key_decisions:
  - Construct PolyController directly instead of via createInstance() to avoid VST3 diamond-inheritance ambiguous cast
  - Link sdk_hosting target for HostApplication vtable instead of manual source compilation
  - Separate INTERACTION_TEST_HARNESS_DIR from VISUAL_TEST_HARNESS_DIR in CMake module
  - Visual tests macOS-only in CI (CoreGraphics dependency); interaction tests on all platforms with GTEST_SKIP
  - Auto-create baselines when no reference PNG exists, enforcing comparison once committed
key_files:
  - tests/ui/visual/visual_smoke_tests.cpp
  - tests/ui/interaction/interaction_smoke_tests.cpp
  - tests/ui/visual/visual_test_harness.h
  - tests/ui/visual/image_compare.h
  - tests/ui/interaction/headless_ui_host.h
  - cmake/jk_ui_test_harness.cmake
  - tests/CMakeLists.txt
  - .github/workflows/ci.yml
lessons_learned:
  - VST3 SDK diamond inheritance (FUnknown → EditController) makes static_cast ambiguous — construct plugin classes directly in test code
  - sdk_hosting target provides HostApplication with proper vtable linkage; manual .cpp inclusion crashes
  - VSTGUI headless rendering needs initPlatform(CFBundleGetMainBundle()) on macOS before any CView operations
---

# M006: UI Test Framework Adoption

**Adopted jk.digital UI test harness from audio-meta, delivering 20 UI tests (6 visual regression + 14 interaction) with baseline PNGs and CI integration**

## What Happened

Adopted the shared jk.digital UI test harness from audio-meta into Poly across 4 slices. S01 copied the harness files (visual_test_harness, image_compare, headless_ui_host), adapted the CMake module for separate visual/interaction directories, wired two new test targets (poly_visual_tests, poly_interaction_tests), and proved the pipeline with smoke tests. S02 expanded interaction coverage to 14 tests across 4 fixtures (MacroKnobTest, LifecycleTest, ViewTreeTest, InteractionSmokeTest) covering scroll/drag gestures, all 6 macro knobs, lifecycle edge cases, edit log capture, and view tree inspection via dynamic_cast. S03 replaced smoke rendering with 6 proper visual regression tests comparing rendered output against committed baseline PNGs using pixel-level comparison with tolerance and diff image generation on mismatch — covering LaneGridView (default, high complexity, max lanes) and VelocityView (default, high density, high tension). S04 integrated everything into CI: interaction tests enabled on all 3 platforms (GTEST_SKIP on non-macOS), visual tests on macOS only, with diff artifact upload on failure. Total test count: 121 (101 engine + 6 visual + 14 interaction).

## Success Criteria Results

- Harness files copied and building as separate CMake targets: PASS (poly_visual_tests + poly_interaction_tests)
- HeadlessUIHost can instantiate PolyController + editor, simulate clicks/drags: PASS (14 interaction tests)
- Visual regression baselines established for both custom views in default and key states: PASS (6 baselines in tests/ui/visual/references/)
- CI runs interaction tests on every push; visual tests on macOS with artifact upload: PASS (ci.yml updated)
- All existing 101+ engine tests continue to pass unchanged: PASS (121/121 total)

## Definition of Done Results

Not provided.

## Requirement Outcomes

Not provided.

## Deviations

None.

## Follow-ups

Potential future work: add visual regression tests for additional parameter combinations, explore Linux headless rendering via X virtual framebuffer for CI parity, add screenshot capture to interaction tests for visual verification of gesture results
