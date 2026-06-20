# S01: Dynamic Shaping

**Goal:** Complete the velocity pipeline by implementing accent mask application, emphasis probability gating, and ghost floor velocity in renderRange
**Demo:** Lane output shows natural velocity variation with distinct accents, ghost notes, and controlled emphasis probability

## Must-Haves

- Accent positions produce higher velocity, ghost floor limits minimum velocity, emphasis probability gates accent expression; all deterministic with golden test coverage

## Proof Level

- This slice proves: Unit tests + golden tests with accent/emphasis/ghost scenarios

## Integration Closure

Extends existing renderRange velocity calculation; no new APIs or cross-module dependencies

## Verification

- Velocity values in golden test output show accent/ghost/emphasis variation

## Tasks

- [x] **T01: Implement accent mask velocity boost** `est:30min`
  In renderRange, after Euclidean pattern check and probability gate, check if the current cycleStep is marked in cfg.accents.steps[]. If true, boost velocity by a fixed accent amount (e.g. +0.15, clamped to 1.0). The accent mask is already defined in LaneConfig but not used.
  - Files: `engine/src/engine.cpp`, `tests/euclidean_tests.cpp`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [x] **T02: Implement emphasis probability gate** `est:20min`
  When a step is an accent position, use emphasisProb to gate whether the accent actually expresses. Use deterministicRand with a new channel (e.g. channel 2) to roll emphasis. If roll >= emphasisProb, the accent is suppressed and the note uses base velocity instead.
  - Files: `engine/src/engine.cpp`, `tests/euclidean_tests.cpp`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [x] **T03: Implement ghost floor velocity** `est:15min`
  After all velocity calculations (base + spread + accent), clamp the final velocity so it never falls below ghostFloor/127.0f. This ensures even quiet notes maintain a minimum presence. Apply before the final clamp to [0, 1].
  - Files: `engine/src/engine.cpp`, `tests/euclidean_tests.cpp`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [x] **T04: Update golden tests for dynamic shaping** `est:20min`
  Regenerate golden test reference file with a default patch that exercises accent masks, emphasis probability, and ghost floor. The default_patch_4bars.txt golden needs updating since velocity values will change. Add a new golden test with a patch specifically configured for dynamic shaping features.
  - Files: `tests/golden_tests.cpp`, `tests/golden/default_patch_4bars.txt`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

## Files Likely Touched

- engine/src/engine.cpp
- tests/euclidean_tests.cpp
- tests/golden_tests.cpp
- tests/golden/default_patch_4bars.txt
