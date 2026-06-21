---
estimated_steps: 1
estimated_files: 4
skills_used: []
---

# T03: Export trigger and plugin integration

Add export trigger parameter (kExportTrigger) that when set to 1.0 initiates capture export. In process(), detect trigger and defer actual file write to a non-RT path (use IMessage to controller, controller writes file). Add capture length parameter. Wire up in processor and controller.

## Inputs

- `plugin/source/processor.cpp`
- `engine/include/poly/midi_capture.h`
- `engine/include/poly/smf_writer.h`

## Expected Output

- `plugin/source/processor.cpp`
- `plugin/source/controller.cpp`

## Verification

cmake --build build && ctest --test-dir build
