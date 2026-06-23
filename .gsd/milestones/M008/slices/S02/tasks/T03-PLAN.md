---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T03: Serialize timeline fields and expose parameters

Add timeline, fixedPattern, fixedPatternLength to state serialization (version 10, same bump as S01). Add controller parameters for timeline toggle and pattern.

## Inputs

- `plugin/source/processor.cpp`
- `plugin/source/controller.cpp`

## Expected Output

- `Serialization and parameters for timeline mode`

## Verification

cmake --build build && ctest --test-dir build
