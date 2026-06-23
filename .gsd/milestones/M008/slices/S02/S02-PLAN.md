# S02: Timeline Mode

**Goal:** Add a timeline lane mode where the lane uses a fixed step pattern (not Euclidean) and is immune to macro changes. Timeline lanes serve as the rhythmic skeleton that other lanes relate to — e.g. Afro-Cuban clave, Afrobeat bell pattern.
**Demo:** Mark a lane as timeline — it becomes immune to macro changes and serves as the rhythmic skeleton

## Must-Haves

- A lane marked as timeline uses its fixed pattern verbatim. Macro knobs (complexity, density, syncopation) do not affect timeline lanes. Timeline lanes still respond to envelopes and phrase features. State serialized at version 10.

## Proof Level

- This slice proves: Unit tests for timeline mode flag, macro immunity, and golden tests for timeline patterns.

## Integration Closure

Timeline mode composes with additive cells (S01), phrase gating, envelopes, and kotekan. Macros skip timeline lanes cleanly.

## Verification

- None — deterministic engine feature.

## Tasks

- [ ] **T01: Add timeline flag and fixed pattern to LaneConfig** `est:20min`
  Add bool timeline and std::array<bool, kMaxSteps> fixedPattern plus int fixedPatternLength to LaneConfig. When timeline=true, the lane uses fixedPattern instead of Euclidean distribution.
  - Files: `engine/include/poly/types.h`
  - Verify: cmake --build build && ctest --test-dir build

- [ ] **T02: Engine: use fixed pattern and skip macros for timeline lanes** `est:45min`
  In renderRange(), when cfg.timeline is true, use cfg.fixedPattern instead of euclidean(). In macro application (scene.cpp applyMacros), skip lanes with timeline=true so complexity/density/syncopation don't alter the reference pattern.
  - Files: `engine/src/engine.cpp`, `engine/src/scene.cpp`
  - Verify: cmake --build build && ctest --test-dir build

- [ ] **T03: Serialize timeline fields and expose parameters** `est:30min`
  Add timeline, fixedPattern, fixedPatternLength to state serialization (version 10, same bump as S01). Add controller parameters for timeline toggle and pattern.
  - Files: `plugin/source/processor.cpp`, `plugin/source/controller.cpp`
  - Verify: cmake --build build && ctest --test-dir build

- [ ] **T04: Tests for timeline mode** `est:30min`
  Add tests: (1) timeline lane uses fixed pattern, not Euclidean, (2) macros don't affect timeline lanes, (3) timeline + envelopes still work, (4) timeline + phrase gating works, (5) golden test for a timeline bell pattern.
  - Files: `tests/scene_tests.cpp`, `tests/golden_tests.cpp`
  - Verify: cmake --build build && ctest --test-dir build

## Files Likely Touched

- engine/include/poly/types.h
- engine/src/engine.cpp
- engine/src/scene.cpp
- plugin/source/processor.cpp
- plugin/source/controller.cpp
- tests/scene_tests.cpp
- tests/golden_tests.cpp
