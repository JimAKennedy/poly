---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T05: Add integration tests for plugin bridge

Write tests for: PPQ-to-sample-offset conversion accuracy, state serialization round-trip (write then read, compare GrooveState fields), NoteOff scheduling across block boundaries. These test the bridge logic without requiring the VST3 SDK host.

## Inputs

- `engine/include/poly/types.h`
- `plugin/source/processor.h`

## Expected Output

- `tests/plugin_tests.cpp`

## Verification

cd build && cmake --build . && ctest --output-on-failure
