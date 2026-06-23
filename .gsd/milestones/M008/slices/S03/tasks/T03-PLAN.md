---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T03: Serialize micro-timing map and expose parameters

Add microTimingMs array to state serialization (version 10). Add controller parameters. Version 9 load defaults to all-zeros array.

## Inputs

- `plugin/source/processor.cpp`
- `plugin/source/controller.cpp`

## Expected Output

- `Serialization and parameters for micro-timing`

## Verification

cmake --build build && ctest --test-dir build
