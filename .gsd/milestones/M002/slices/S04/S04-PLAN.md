# S04: Macro Resolution

**Goal:** Implement coherent multi-parameter macro mapping for complexity, density, syncopation, swing, tension, and humanize
**Demo:** Each macro visibly and audibly affects multiple lane parameters in a musically coherent way across its full range

## Must-Haves

- Each macro maps to multiple lane parameters coherently (not simple multiply); macro at 0 and 1 produce distinct musical results; all mappings are RT-safe with no allocation

## Proof Level

- This slice proves: Unit tests for each macro mapping; golden tests showing macro sweep output

## Integration Closure

Macro resolution runs before renderRange, producing resolved GrooveState; no persistent state

## Verification

- Tests verify macro values produce expected parameter distributions across lanes

## Tasks

- [ ] **T01: Design macro-to-parameter mapping** `est:45min`
  Define how each macro affects lane parameters. Complexity: hitCount, rotation, envelope depth. Density: probability, hitCount, activation weight. Syncopation: rotation, accent bias. Swing: swingAmount across lanes. Tension: velocity spread, emphasis prob, envelope depth. Humanize: humanizeMs, velocity spread. Write a resolveMacros(const GrooveState& input) -> GrooveState function that applies all mappings. Pure function, no allocation.
  - Files: `engine/include/poly/macro.h`, `engine/src/macro.cpp`, `engine/CMakeLists.txt`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [ ] **T02: Integrate macro resolution into engine pipeline** `est:20min`
  Call resolveMacros before renderRange in the processing pipeline. The caller (processor or test harness) resolves macros on the GrooveState before passing to renderRange. Add a convenience method or document the call order. Ensure resolved state is stack-allocated or uses the existing GrooveState (no heap).
  - Files: `engine/src/engine.cpp`, `tools/harness/main.cpp`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [ ] **T03: Add macro mapping tests and golden updates** `est:30min`
  Write unit tests verifying: each macro at 0.0, 0.5, 1.0 produces expected parameter ranges. Test that macros compose correctly (density + complexity together). Add golden test with macros at non-default values to verify deterministic output under macro resolution.
  - Files: `tests/macro_tests.cpp`, `tests/golden_tests.cpp`, `tests/CMakeLists.txt`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

## Files Likely Touched

- engine/include/poly/macro.h
- engine/src/macro.cpp
- engine/CMakeLists.txt
- engine/src/engine.cpp
- tools/harness/main.cpp
- tests/macro_tests.cpp
- tests/golden_tests.cpp
- tests/CMakeLists.txt
