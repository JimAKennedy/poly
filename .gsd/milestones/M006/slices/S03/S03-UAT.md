# S03: Visual Regression Baselines — UAT

**Milestone:** M006
**Written:** 2026-06-21T01:06:39.805Z

- [ ] `cmake --build build --target poly_visual_tests` compiles without errors
- [ ] `ctest --test-dir build -R VisualRegression` reports 6/6 pass
- [ ] 6 baseline PNGs exist in `tests/ui/visual/references/`
- [ ] Deleting a baseline and re-running auto-creates it
- [ ] Modifying a baseline causes test failure with diff image in output dir
- [ ] Full suite `ctest --test-dir build` reports 121/121 pass
