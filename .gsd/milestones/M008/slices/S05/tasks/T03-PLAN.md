---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T03: Register view in uidesc and controller

Add cross-rhythm view to poly.uidesc layout. Register the view factory in controller.cpp createView(). Wire parameter tags for lane configs.

## Inputs

- `plugin/resource/poly.uidesc`
- `plugin/source/controller.cpp`

## Expected Output

- `View registered and visible in plugin UI`

## Verification

cmake --build build && ctest --test-dir build
