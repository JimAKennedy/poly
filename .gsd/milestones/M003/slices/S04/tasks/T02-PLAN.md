---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T02: Parameter display formatting

Add proper units strings to all parameters: Probability -> '%', Humanize -> 'ms', Note Duration -> 'beats', Velocity -> '0-127', Swing -> '%'. Set appropriate stepCount for discrete params. Add getParamStringByValue/getParamValueByString overrides for formatted display where needed.

## Inputs

- `plugin/source/controller.cpp`

## Expected Output

- `plugin/source/controller.cpp`

## Verification

cmake --build build && ctest --test-dir build -R plugin
