# S06: Phase and Envelope Visualization

**Goal:** Add visual displays for envelope curves and cycle phase alignment so users can see how their groove evolves over time
**Demo:** UI shows how lane cycles align and diverge over time; envelope curves visible with their modulation targets

## Must-Haves

- Envelope curve view renders shape with current phase marker. Cycle phase indicators show position within each lane cycle. Phase alignment overview shows multi-lane cycle interactions. All new views have visual regression baselines.

## Proof Level

- This slice proves: Visual regression tests with baselines; interaction tests for phase update; build on all platforms

## Integration Closure

New VSTGUI custom views registered in controller. Requires read-only output params for current phase values. poly.uidesc updated.

## Verification

- Phase output parameters provide real-time feedback on engine state to UI.

## Tasks

- [x] **T01: Envelope curve custom view** `est:2h`
  Implement EnvelopeCurveView (VSTGUI CView subclass) that draws the current envelope shape as a filled curve. Takes envelope shape, period, depth, and current phase as inputs. Renders shape waveform with a vertical phase-position marker. Register in controller createCustomView.
  - Files: `plugin/source/ui/envelope_curve_view.h`, `plugin/source/ui/envelope_curve_view.cpp`, `plugin/source/controller.cpp`, `plugin/resource/poly.uidesc`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T02: Cycle phase indicators** `est:1.5h`
  Add per-lane phase output parameters (kLanePhaseOutput base) emitted from process() showing current normalized position within each lane cycle. Display as a small circular or linear progress indicator in LaneGridView for each lane row.
  - Files: `plugin/source/processor.cpp`, `plugin/source/plugids.h`, `plugin/source/ui/lane_grid_view.cpp`, `plugin/source/ui/lane_grid_view.h`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T03: Phase alignment overview** `est:2h`
  Implement PhaseAlignmentView: compact multi-lane visualization showing all active lane cycles as concentric rings or parallel timelines, with phase markers. Shows where cycles align and diverge. Register in createCustomView and add to poly.uidesc.
  - Files: `plugin/source/ui/phase_alignment_view.h`, `plugin/source/ui/phase_alignment_view.cpp`, `plugin/source/controller.cpp`, `plugin/resource/poly.uidesc`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T04: Visual regression baselines for new views** `est:1h`
  Add visual regression test cases for EnvelopeCurveView and PhaseAlignmentView. Render each view in key states (default, mid-phase, multiple shapes). Generate baseline PNGs. Add to visual_smoke_tests.cpp.
  - Files: `tests/ui/visual/visual_smoke_tests.cpp`
  - Verify: cmake --build build && ctest --test-dir build -R visual

## Files Likely Touched

- plugin/source/ui/envelope_curve_view.h
- plugin/source/ui/envelope_curve_view.cpp
- plugin/source/controller.cpp
- plugin/resource/poly.uidesc
- plugin/source/processor.cpp
- plugin/source/plugids.h
- plugin/source/ui/lane_grid_view.cpp
- plugin/source/ui/lane_grid_view.h
- plugin/source/ui/phase_alignment_view.h
- plugin/source/ui/phase_alignment_view.cpp
- tests/ui/visual/visual_smoke_tests.cpp
