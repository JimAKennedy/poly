---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T03: Implement macro knob row

Add a row of CKnobBase controls for the 6 macro parameters (complexity, density, syncopation, swing, tension, humanize). Wire each to its corresponding ParamID from the controller. Use VSTGUI parameter binding so knob position reflects automation state.

## Inputs

- `plugin/source/plugids.h`
- `plugin/source/controller.cpp`
- `plugin/source/ui/editor.uidesc`

## Expected Output

- `plugin/source/ui/editor.uidesc`
- `plugin/source/controller.cpp`

## Verification

cd build && cmake --build . && ctest --output-on-failure
