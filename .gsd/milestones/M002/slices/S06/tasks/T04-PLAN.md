---
estimated_steps: 1
estimated_files: 4
skills_used: []
---

# T04: Implement velocity view

Create a custom view that displays per-lane velocity values during playback. Show vertical bars for each lane's most recent note velocity, updating in real time. Use message passing from processor to controller (non-RT thread) to communicate current velocity state. Ensure no RT-unsafe operations.

## Inputs

- `plugin/source/controller.cpp`
- `plugin/source/processor.cpp`
- `plugin/source/ui/editor.uidesc`

## Expected Output

- `plugin/source/ui/velocity_view.h`
- `plugin/source/ui/velocity_view.cpp`

## Verification

cd build && cmake --build . && ctest --output-on-failure
