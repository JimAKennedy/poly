# S02: Engine Domain Model and Headless Harness

**Goal:** Implement the full engine domain model (LaneConfig, Envelope, GrooveState, TransportContext) from IMPLEMENTATION_PLAN.md §3, expand Engine::renderRange() signature to accept GrooveState, add a headless CLI harness for testing the engine without the VST3 SDK, and verify engine isolation (compiles without VST3 SDK).
**Demo:** Harness accepts a patch and transport config, outputs structured note events to stdout; engine compiles without VST3 SDK

## Must-Haves

- Engine domain model headers match §3 spec. Engine compiles and links independently of VST3 SDK. Headless harness runs and produces output. All existing tests still pass.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Engine domain model types** `est:30min`
  Expand types.h with the full domain model from IMPLEMENTATION_PLAN.md §3: Role enum, Cycle struct, AccentMask, EnvTarget enum, Shape enum, Envelope struct, EnvelopeAssign, LaneConfig struct, MacroValues, GrooveState. Use fixed-size containers (std::array, inline arrays) instead of std::vector for RT safety. Update Engine::renderRange() signature to accept const GrooveState&.
  - Files: `engine/include/poly/types.h`, `engine/include/poly/engine.h`, `engine/src/engine.cpp`
  - Verify: cmake --build build --target poly_engine compiles clean with zero warnings

- [x] **T02: Headless CLI harness** `est:25min`
  Create tools/harness/ with a CLI program that instantiates Engine, configures a GrooveState with sensible defaults, runs renderRange() over a simulated transport range, and prints NoteEvents to stdout in a diffable text format. Add CMakeLists.txt for the harness target. The harness must link only against poly_engine, not the VST3 SDK.
  - Files: `tools/harness/main.cpp`, `tools/harness/CMakeLists.txt`, `CMakeLists.txt`
  - Verify: cmake --build build --target poly_harness && ./build/tools/harness/poly_harness runs and produces output

- [x] **T03: Engine isolation verification** `est:10min`
  Verify that poly_engine and poly_harness compile and link without any VST3 SDK symbols. Check that no VST3 headers are included transitively. Run the harness binary to confirm it executes.
  - Verify: nm build/engine/libpoly_engine.a | grep -i steinberg returns empty; ./build/tools/harness/poly_harness exits 0

## Files Likely Touched

- engine/include/poly/types.h
- engine/include/poly/engine.h
- engine/src/engine.cpp
- tools/harness/main.cpp
- tools/harness/CMakeLists.txt
- CMakeLists.txt
