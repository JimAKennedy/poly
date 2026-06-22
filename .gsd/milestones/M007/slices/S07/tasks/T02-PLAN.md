---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T02: Wire into UI layout and deploy

Register in controller createCustomView, update uidesc layout, add to CMakeLists, build and deploy.

## Inputs

- None specified.

## Expected Output

- `plugin/source/controller.cpp`

## Verification

cmake --build build && ctest --test-dir build
