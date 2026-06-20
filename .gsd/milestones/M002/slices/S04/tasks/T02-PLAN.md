---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T02: Integrate macro resolution into engine pipeline

Call resolveMacros before renderRange in the processing pipeline. The caller (processor or test harness) resolves macros on the GrooveState before passing to renderRange. Add a convenience method or document the call order. Ensure resolved state is stack-allocated or uses the existing GrooveState (no heap).

## Inputs

- `engine/include/poly/macro.h`
- `engine/include/poly/types.h`

## Expected Output

- `engine/src/engine.cpp`
- `tools/harness/main.cpp`

## Verification

cd build && cmake --build . && ctest --output-on-failure
