# S01: Harness Integration and Smoke Tests — UAT

**Milestone:** M006
**Written:** 2026-06-21T01:00:27.348Z

## S01: Harness Integration and Smoke Tests — UAT\n\n### Build Verification\n- [ ] `cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_VISUAL_TESTS=ON -DBUILD_INTERACTION_TESTS=ON` succeeds\n- [ ] `cmake --build build --target poly_visual_tests` succeeds\n- [ ] `cmake --build build --target poly_interaction_tests` succeeds\n- [ ] `cmake --build build --target poly_tests` still succeeds (engine isolation)\n\n### Test Verification\n- [ ] `ctest --test-dir build` shows 105/105 tests passing\n- [ ] Visual tests produce PNG files in `build/tests/visual_output/`\n- [ ] Interaction tests open and close HeadlessUIHost without crashes\n- [ ] `POLY_ENGINE_ONLY=ON` build still works with no UI test targets
