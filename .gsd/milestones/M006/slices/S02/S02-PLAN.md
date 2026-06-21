# S02: Interaction Test Suite

**Goal:** Write interaction tests covering controller lifecycle, parameter edits via simulated UI input on LaneGridView and VelocityView
**Demo:** Headless tests simulate clicks/drags on LaneGridView and VelocityView, verify parameter changes propagate to engine config

## Must-Haves

- Complete the planned slice outcomes.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Macro knob interaction tests** `est:20min`
  Test that scrolling/dragging the 6 macro knobs (Complexity, Density, Syncopation, Swing, Tension, Humanize) changes their parameter values. Verify parameter changes appear in getParameterValue() and the edit log.
  - Files: `tests/ui/interaction/interaction_smoke_tests.cpp`
  - Verify: All macro knob tests pass via ctest -R InteractionSmokeTest

- [x] **T02: Controller lifecycle edge cases** `est:10min`
  Test double-open prevention, open-close-reopen cycle, parameter read before open (should return 0), and edit log clearing.
  - Files: `tests/ui/interaction/interaction_smoke_tests.cpp`
  - Verify: All lifecycle tests pass

- [x] **T03: View tree inspection tests** `est:15min`
  Test that getFrame() returns a valid CFrame with expected child structure. Verify getControlRect() returns sensible bounds for macro knobs. Test that custom views (LaneGridView, VelocityView) are present in the view tree.
  - Files: `tests/ui/interaction/interaction_smoke_tests.cpp`
  - Verify: All view tree tests pass

## Files Likely Touched

- tests/ui/interaction/interaction_smoke_tests.cpp
