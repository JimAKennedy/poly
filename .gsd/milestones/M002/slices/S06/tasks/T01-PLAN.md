---
estimated_steps: 1
estimated_files: 4
skills_used: []
---

# T01: VSTGUI spike and editor scaffold

Set up VSTGUI editor infrastructure: create .uidesc file, wire createView in controller, add editor open/close lifecycle. Start with a minimal window that opens and closes cleanly in the host. Evaluate VSTGUI custom view approach vs built-in controls for the lane grid.

## Inputs

- `plugin/source/controller.h`
- `plugin/source/controller.cpp`

## Expected Output

- `plugin/source/ui/editor.uidesc`
- `plugin/source/controller.cpp`

## Verification

cd build && cmake --build . && ctest --output-on-failure
