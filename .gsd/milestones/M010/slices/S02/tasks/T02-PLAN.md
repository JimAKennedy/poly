---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T02: Wire view into controller and layout

Register LaneEditView in controller.cpp createCustomView. Add to poly.uidesc layout. Shrink VelocityView from 76px to 40px. Adjust window height and all y-origins below the new section.

## Inputs

- `Current poly.uidesc layout positions`

## Expected Output

- `LaneEditView appears in UI between lanes and phrase section`
- `Velocity view shorter`
- `Window fits at ~680px`

## Verification

cmake --build build && ctest --test-dir build
