---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T02: Headless CLI harness

Create tools/harness/ with a CLI program that instantiates Engine, configures a GrooveState with sensible defaults, runs renderRange() over a simulated transport range, and prints NoteEvents to stdout in a diffable text format. Add CMakeLists.txt for the harness target. The harness must link only against poly_engine, not the VST3 SDK.

## Inputs

- `engine/include/poly/types.h`
- `engine/include/poly/engine.h`

## Expected Output

- `tools/harness/main.cpp`
- `tools/harness/CMakeLists.txt`

## Verification

cmake --build build --target poly_harness && ./build/tools/harness/poly_harness runs and produces output
