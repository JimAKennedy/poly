---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T01: VST3 Unit hierarchy

Implement IUnitInfo in PolyController. Create unit hierarchy: root -> Lane 1..8 (each with their params), Macros unit, Global unit. Register units in initialize(). Assign each parameter unitId to the correct unit.

## Inputs

- `plugin/source/controller.cpp`

## Expected Output

- `plugin/source/controller.cpp`
- `plugin/source/controller.h`

## Verification

cmake --build build && ctest --test-dir build -R plugin
