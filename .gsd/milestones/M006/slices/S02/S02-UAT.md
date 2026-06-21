# S02: Interaction Test Suite — UAT

**Milestone:** M006
**Written:** 2026-06-21T01:04:00.362Z

- [ ] `cmake --build build --target poly_interaction_tests` compiles without errors
- [ ] `ctest --test-dir build -R "Interaction|MacroKnob|Lifecycle|ViewTree"` reports 14/14 pass
- [ ] Scroll on Complexity and Density knobs changes parameter values
- [ ] All 6 macro knobs discoverable by ParamID tag
- [ ] Edit log captures parameter edits from scroll gestures
- [ ] Double open returns false, reopen after close succeeds
- [ ] View tree contains LaneGridView and VelocityView instances
