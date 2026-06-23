# S02: Timeline Mode

**Goal:** Add a timeline lane mode where the lane uses a fixed step pattern (not Euclidean) and is immune to macro changes. Timeline lanes serve as the rhythmic skeleton. Custom Timeline Step Editor view for editing the fixed pattern, with lane grid indicator for timeline mode.
**Demo:** Mark a lane as timeline — it becomes immune to macro changes and serves as the rhythmic skeleton

## Must-Haves

- A lane marked as timeline uses its fixed pattern verbatim. Macro knobs do not affect timeline lanes. Timeline lanes still respond to envelopes and phrase features. State serialized at version 10. Timeline Step Editor allows visual step toggling. Lane grid shows timeline indicator.

## Proof Level

- This slice proves: Unit tests for timeline mode flag, macro immunity, golden tests for timeline patterns. UI builds and renders.

## Integration Closure

Timeline mode composes with additive cells (S01), phrase gating, envelopes, and kotekan. Macros skip timeline lanes cleanly. Step Editor integrates with existing lane edit panel.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Add timeline flag and fixed pattern to LaneConfig** `est:20min`
  Add bool timeline and std::array<bool, kMaxSteps> fixedPattern plus int fixedPatternLength to LaneConfig. When timeline=true, the lane uses fixedPattern instead of Euclidean distribution.
  - Files: `engine/include/poly/types.h`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T02: Engine: use fixed pattern and skip macros for timeline lanes** `est:45min`
  In renderRange(), when cfg.timeline is true, use cfg.fixedPattern instead of euclidean(). In macro application (scene.cpp applyMacros), skip lanes with timeline=true so complexity/density/syncopation don't alter the reference pattern.
  - Files: `engine/src/engine.cpp`, `engine/src/scene.cpp`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T03: Serialize timeline fields and expose scalar parameters** `est:30min`
  Add timeline, fixedPattern, fixedPatternLength to state serialization (version 10, same bump as S01). Add VST3 controller parameters for timeline toggle and fixedPatternLength ONLY. fixedPattern array is NOT exposed as individual VST3 params — it is state-serialized only, edited through the Timeline Step Editor (T05). Version 9 load defaults timeline=false, fixedPatternLength=0.
  - Files: `plugin/source/processor.cpp`, `plugin/source/controller.cpp`, `plugin/source/plugids.h`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T04: Tests for timeline mode** `est:30min`
  Add tests: (1) timeline lane uses fixed pattern, not Euclidean, (2) macros don't affect timeline lanes, (3) timeline + envelopes still work, (4) timeline + phrase gating works, (5) golden test for a timeline bell pattern.
  - Files: `tests/scene_tests.cpp`, `tests/golden_tests.cpp`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T05: Timeline Step Editor View and lane grid timeline indicator** `est:1h30min`
  Create a VSTGUI custom view (TimelineStepEditorView) for editing the fixed step pattern per lane. Shows a row of step cells matching fixedPatternLength — click to toggle each step on/off. Active steps filled, inactive steps hollow. Updates fixedPattern on LaneConfig via state serialization (not VST3 params). Also add a timeline mode indicator to LaneGridView — show 'TL' badge with distinct color when lane has timeline=true, so users can see at a glance which lanes are timeline anchors vs generative.
  - Files: `plugin/source/ui/timeline_step_editor_view.cpp`, `plugin/source/ui/timeline_step_editor_view.h`, `plugin/source/ui/lane_grid_view.cpp`, `plugin/source/ui/lane_grid_view.h`, `plugin/resource/poly.uidesc`
  - Verify: cmake --build build && ctest --test-dir build (build passes, UI tests pass)

## Files Likely Touched

- engine/include/poly/types.h
- engine/src/engine.cpp
- engine/src/scene.cpp
- plugin/source/processor.cpp
- plugin/source/controller.cpp
- plugin/source/plugids.h
- tests/scene_tests.cpp
- tests/golden_tests.cpp
- plugin/source/ui/timeline_step_editor_view.cpp
- plugin/source/ui/timeline_step_editor_view.h
- plugin/source/ui/lane_grid_view.cpp
- plugin/source/ui/lane_grid_view.h
- plugin/resource/poly.uidesc
