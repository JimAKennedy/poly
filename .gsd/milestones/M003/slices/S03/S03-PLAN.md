# S03: Constraint Layer

**Goal:** Add constraint layer that protects structural anchors (kick, backbeat) and enforces density bounds so macros and envelopes cannot destroy groove coherence
**Demo:** Anchor kick remains stable even when density envelope bottoms out; backbeat survives high-syncopation macro settings

## Must-Haves

- Anchor steps always fire regardless of probability/envelope attenuation. Backbeat emphasis is preserved when syncopation/tension macros are active. Per-lane density stays within configured min/max hit bounds. Constraints applied after macro resolution, before output.

## Proof Level

- This slice proves: Unit tests for each constraint type; integration test with macros at extremes verifying anchors survive

## Integration Closure

Constraint applied in renderRange after macro resolution. No UI changes. State serialization extended for constraint config.

## Verification

- None required; constraints are pure engine logic.

## Tasks

- [x] **T01: Constraint data model** `est:1h`
  Add ConstraintConfig to LaneConfig: anchorSteps (AccentMask marking steps that must always fire), backbeatProtect (bool, preserves emphasis on steps 2/4 in 4-beat cycles), densityMin/densityMax (int, min/max Euclidean hits after macro resolution). Add global ConstraintConfig to GrooveState for cross-lane density ceiling.
  - Files: `engine/include/poly/types.h`
  - Verify: cmake --build build

- [x] **T02: Anchor and backbeat constraints** `est:1.5h`
  In renderRange, after probability gate: if the step is in anchorSteps, force it to fire (skip probability roll). For backbeat protection: if backbeatProtect is true and the step is at a backbeat position (step 2, 6 in 8-step; step 1, 3 in 4-step), preserve base velocity and emphasisProb from pre-macro values. Apply after macro resolution but before envelope modulation.
  - Files: `engine/src/engine.cpp`
  - Verify: cmake --build build && ctest --test-dir build -R constraint

- [x] **T03: Density guardrails** `est:1h`
  After macro resolution clamps hitCount, enforce densityMin and densityMax bounds on the resolved hitCount. If macro-driven hitCount falls below densityMin, force it up. If above densityMax, force it down. Apply in resolveMacros output or as a post-resolution step before renderRange.
  - Files: `engine/src/engine.cpp`, `engine/src/macro.cpp`
  - Verify: cmake --build build && ctest --test-dir build -R constraint

- [x] **T04: Constraint serialization** `est:45m`
  Extend state_io to read/write the new ConstraintConfig fields. Bump state version if not already bumped by S02. Handle backwards compat for older versions (default: no anchors, no backbeat protect, density bounds 0/maxSteps).
  - Files: `engine/include/poly/state_io.h`
  - Verify: cmake --build build && ctest --test-dir build -R plugin

- [x] **T05: Constraint tests** `est:1h`
  Test anchor steps always fire with probability=0. Test backbeat protection preserves emphasis under extreme syncopation macro. Test density guardrails clamp hitCount at extremes (density macro=0 with densityMin=2, density macro=1 with densityMax=4). Test serialization round-trip.
  - Files: `tests/constraint_tests.cpp`
  - Verify: cmake --build build && ctest --test-dir build -R constraint

## Files Likely Touched

- engine/include/poly/types.h
- engine/src/engine.cpp
- engine/src/macro.cpp
- engine/include/poly/state_io.h
- tests/constraint_tests.cpp
