---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T02: Engine: use fixed pattern and skip macros for timeline lanes

In renderRange(), when cfg.timeline is true, use cfg.fixedPattern instead of euclidean(). In macro application (scene.cpp applyMacros), skip lanes with timeline=true so complexity/density/syncopation don't alter the reference pattern.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `Timeline lanes use fixed pattern, macros skip them`

## Verification

cmake --build build && ctest --test-dir build
