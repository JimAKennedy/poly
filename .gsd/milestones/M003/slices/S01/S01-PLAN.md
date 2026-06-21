# S01: Extended Envelope System

**Goal:** Implement all 8 envelope targets and all 5 shapes so envelopes can modulate any lane parameter over any timescale
**Demo:** Envelopes target length, looseness, activation, and fill alongside velocity/density; 4/8/16/custom bar phrase lengths create section-scale evolution

## Must-Haves

- All EnvTarget variants (Velocity, Density, Probability, AccentBias, NoteLength, TimingLooseness, ActivationWeight, FillLikelihood) modulate their respective parameters in renderRange(). All Shape variants (Ramp, Sine, Triangle, Curve, StepList) produce distinct waveforms. Golden tests confirm determinism.

## Proof Level

- This slice proves: Unit tests for each target and shape; golden test update; all 121+ existing tests still pass

## Integration Closure

Engine-only change. Plugin layer and UI untouched. Existing Velocity/Density behavior unchanged for backwards compat.

## Verification

- No new observability surfaces needed; existing golden tests cover determinism.

## Tasks

- [x] **T01: Implement remaining envelope targets in renderRange** `est:2h`
  Add switch cases in renderRange() for the 6 unhandled EnvTarget values: Probability (refine existing density-style), AccentBias (modulate emphasisProb), NoteLength (modulate note duration), TimingLooseness (modulate humanizeMs equivalent), ActivationWeight (modulate lane active probability per-cycle), FillLikelihood (add fill notes at envelope peaks). Each target needs clear modulation semantics: multiplicative for velocity-like targets, additive for probability-like targets.
  - Files: `engine/src/engine.cpp`, `engine/include/poly/types.h`
  - Verify: cmake --build build && ctest --test-dir build -R envelope

- [x] **T02: Implement Curve and StepList shapes** `est:1.5h`
  Replace the stub implementations in evaluateShape(). Curve: implement exponential ease-in/ease-out using a curvature parameter (repurpose depth or add a shape parameter field). StepList: add a step values array to the Envelope struct (fixed-size, e.g. 16 floats) and implement lookup by quantized phase. Update state_io for any new fields (bump kCurrentStateVersion).
  - Files: `engine/src/envelope.cpp`, `engine/include/poly/types.h`, `engine/include/poly/state_io.h`
  - Verify: cmake --build build && ctest --test-dir build -R envelope

- [x] **T03: Extended envelope unit tests** `est:1.5h`
  Add tests for all 8 envelope targets applied in renderRange: verify each target modulates the correct parameter, verify modulation direction and clamping, verify per-lane and global envelopes interact correctly. Add shape tests for Curve and StepList evaluateShape outputs. Test edge cases: depth=0 (no modulation), depth=1 (full modulation), phase boundaries.
  - Files: `tests/envelope_tests.cpp`
  - Verify: cmake --build build && ctest --test-dir build -R envelope

- [x] **T04: Update golden tests for new envelope behavior** `est:30m`
  Regenerate the golden reference file if the default patch now produces different output due to envelope changes (it should not if no envelopes are active by default, but verify). Add a new golden test with a patch that exercises multiple envelope targets simultaneously to enforce determinism of the full envelope system.
  - Files: `tests/golden_tests.cpp`, `tests/golden/default_patch_4bars.txt`
  - Verify: cmake --build build && ctest --test-dir build -R golden

## Files Likely Touched

- engine/src/engine.cpp
- engine/include/poly/types.h
- engine/src/envelope.cpp
- engine/include/poly/state_io.h
- tests/envelope_tests.cpp
- tests/golden_tests.cpp
- tests/golden/default_patch_4bars.txt
