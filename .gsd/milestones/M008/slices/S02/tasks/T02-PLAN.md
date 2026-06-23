---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T02: Engine: use fixed pattern and skip macros for timeline lanes

In renderRange(), when cfg.timeline is true, use cfg.fixedPattern instead of euclidean(). In macro application (scene.cpp applyMacros), skip lanes with timeline=true so complexity/density/syncopation don't alter the reference pattern.

## Inputs

- `engine/src/engine.cpp`
- `engine/src/scene.cpp`

## Expected Output

- `engine.cpp and scene.cpp with timeline logic`

## Verification

cmake --build build && ctest --test-dir build
