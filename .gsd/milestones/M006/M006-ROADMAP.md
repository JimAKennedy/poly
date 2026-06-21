# M006: UI Test Framework Adoption

**Vision:** Adopt the jk.digital UI test harness from audio-meta to enable headless interaction testing and visual regression testing of Poly's VSTGUI editor (LaneGridView, VelocityView), ensuring UI changes are caught before they reach users.

## Success Criteria

- Harness files copied and building as separate CMake targets (poly_visual_tests, poly_interaction_tests)
- HeadlessUIHost can instantiate PolyController + editor, simulate clicks/drags on LaneGridView and VelocityView
- Visual regression baselines established for both custom views in default and key states
- CI runs interaction tests on every push; visual tests run on macOS with artifact upload on failure
- All existing 101+ engine tests continue to pass unchanged

## Slices

- [x] **S01: S01** `risk:low` `depends:[]`
  > After this: poly_visual_tests and poly_interaction_tests targets build and run, smoke tests pass proving offscreen rendering and headless host work

- [x] **S02: S02** `risk:medium` `depends:[]`
  > After this: Headless tests simulate clicks/drags on LaneGridView and VelocityView, verify parameter changes propagate to engine config

- [x] **S03: S03** `risk:medium` `depends:[]`
  > After this: Offscreen renders of LaneGridView and VelocityView in default and key states, with baseline PNGs and comparison tests

- [x] **S04: S04** `risk:low` `depends:[]`
  > After this: CI runs interaction tests on every push, visual tests on macOS with diff artifact upload on failure

## Boundary Map

Not provided.
