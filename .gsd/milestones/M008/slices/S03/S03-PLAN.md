# S03: Extended Micro-timing Maps

**Goal:** Add per-step micro-timing maps to LaneConfig — an array of timing offsets (in ms) per step position within the cycle. Goes beyond swing to enable groove templates like Brazilian ginga, J Dilla pocket, or Afrobeat push. Custom Micro-timing Editor view for visual offset editing.
**Demo:** Apply a per-step timing map (not just swing) — e.g. Brazilian ginga feel or J Dilla pocket

## Must-Haves

- A lane with a micro-timing map applies per-step timing offsets. Offsets compose with swing and humanize. Timing map has no effect when all values are 0 (default). Works with both equal and additive cells. State serialized. Micro-timing Editor allows drag-to-adjust per-step offsets visually.

## Proof Level

- This slice proves: Unit tests verifying per-step offsets are applied correctly, composition with swing/humanize, golden tests for groove-mapped patterns. UI builds and renders.

## Integration Closure

Micro-timing maps compose with additive cells (S01), swing, humanize, timing offset, and phrase features. Equal-cell and additive-cell patterns both apply micro-timing correctly. Editor integrates with existing lane edit panel.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Add micro-timing map to LaneConfig** `est:15min`
  Add std::array<float, kMaxSteps> microTimingMs to LaneConfig, defaulting to all zeros. Each entry is the timing offset in ms for that step position within the cycle. Range: [-20, +20] ms per step.
  - Files: `engine/include/poly/types.h`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T02: Apply micro-timing map in renderRange** `est:30min`
  In renderRange(), after swing and before humanize, look up microTimingMs[cycleStep] and convert to PPQ offset using tempo. Apply the offset to ppq. Composes additively with swing and existing timingOffsetMs.
  - Files: `engine/src/engine.cpp`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T03: Serialize micro-timing map (state-only, no VST3 params)** `est:20min`
  Add microTimingMs array to state serialization (version 10, same bump as S01/S02). Do NOT expose individual microTimingMs values as VST3 parameters — the array is state-serialized only, edited through the Micro-timing Editor view (T05). Version 9 load defaults to all-zeros array. No new controller parameters in this task.
  - Files: `plugin/source/processor.cpp`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T04: Tests for micro-timing maps** `est:30min`
  Add tests: (1) per-step timing offset shifts PPQ correctly, (2) micro-timing + swing compose, (3) micro-timing + humanize compose, (4) micro-timing + additive cells compose, (5) all-zero map produces no change (regression), (6) golden test for groove-mapped pattern.
  - Files: `tests/swing_humanize_tests.cpp`, `tests/golden_tests.cpp`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T05: Micro-timing Editor View** `est:1h30min`
  Create a VSTGUI custom view (MicroTimingEditorView) for editing per-step micro-timing offsets per lane. Shows a bar chart with one vertical bar per step — bar height represents the ms offset (center line = 0, up = positive, down = negative). Drag a bar to adjust its value within [-20, +20] ms range. Double-click to reset to 0. Shows numeric tooltip during drag. Updates microTimingMs on LaneConfig via state serialization (not VST3 params). View width adapts to current step count.
  - Files: `plugin/source/ui/micro_timing_editor_view.cpp`, `plugin/source/ui/micro_timing_editor_view.h`, `plugin/resource/poly.uidesc`
  - Verify: cmake --build build && ctest --test-dir build (build passes, UI tests pass)

## Files Likely Touched

- engine/include/poly/types.h
- engine/src/engine.cpp
- plugin/source/processor.cpp
- tests/swing_humanize_tests.cpp
- tests/golden_tests.cpp
- plugin/source/ui/micro_timing_editor_view.cpp
- plugin/source/ui/micro_timing_editor_view.h
- plugin/resource/poly.uidesc
