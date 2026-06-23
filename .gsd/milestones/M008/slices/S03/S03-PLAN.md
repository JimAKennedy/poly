# S03: Extended Micro-timing Maps

**Goal:** Add per-step micro-timing maps to LaneConfig — an array of timing offsets (in ms) per step position within the cycle. Goes beyond swing (which only shifts odd steps) to enable groove templates like Brazilian ginga, J Dilla pocket, or Afrobeat push.
**Demo:** Apply a per-step timing map (not just swing) — e.g. Brazilian ginga feel or J Dilla pocket

## Must-Haves

- A lane with a micro-timing map applies per-step timing offsets. Offsets compose with swing and humanize. Timing map has no effect when all values are 0 (default). Works with both equal and additive cells. State serialized.

## Proof Level

- This slice proves: Unit tests verifying per-step offsets are applied correctly, composition with swing/humanize, and golden tests for groove-mapped patterns.

## Integration Closure

Micro-timing maps compose with additive cells (S01), swing, humanize, timing offset, and phrase features. Equal-cell and additive-cell patterns both apply micro-timing correctly.

## Verification

- None — deterministic engine feature.

## Tasks

- [ ] **T01: Add micro-timing map to LaneConfig** `est:15min`
  Add std::array<float, kMaxSteps> microTimingMs to LaneConfig, defaulting to all zeros. Each entry is the timing offset in ms for that step position within the cycle. Range: [-20, +20] ms per step.
  - Files: `engine/include/poly/types.h`
  - Verify: cmake --build build && ctest --test-dir build

- [ ] **T02: Apply micro-timing map in renderRange** `est:30min`
  In renderRange(), after swing and before humanize, look up microTimingMs[cycleStep] and convert to PPQ offset using tempo. Apply the offset to ppq. Composes additively with swing and existing timingOffsetMs.
  - Files: `engine/src/engine.cpp`
  - Verify: cmake --build build && ctest --test-dir build

- [ ] **T03: Serialize micro-timing map and expose parameters** `est:30min`
  Add microTimingMs array to state serialization (version 10). Add controller parameters. Version 9 load defaults to all-zeros array.
  - Files: `plugin/source/processor.cpp`, `plugin/source/controller.cpp`
  - Verify: cmake --build build && ctest --test-dir build

- [ ] **T04: Tests for micro-timing maps** `est:30min`
  Add tests: (1) per-step timing offset shifts PPQ correctly, (2) micro-timing + swing compose, (3) micro-timing + humanize compose, (4) micro-timing + additive cells compose, (5) all-zero map produces no change (regression), (6) golden test for groove-mapped pattern.
  - Files: `tests/swing_humanize_tests.cpp`, `tests/golden_tests.cpp`
  - Verify: cmake --build build && ctest --test-dir build

## Files Likely Touched

- engine/include/poly/types.h
- engine/src/engine.cpp
- plugin/source/processor.cpp
- plugin/source/controller.cpp
- tests/swing_humanize_tests.cpp
- tests/golden_tests.cpp
