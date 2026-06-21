---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T01: Constraint data model

Add ConstraintConfig to LaneConfig: anchorSteps (AccentMask marking steps that must always fire), backbeatProtect (bool, preserves emphasis on steps 2/4 in 4-beat cycles), densityMin/densityMax (int, min/max Euclidean hits after macro resolution). Add global ConstraintConfig to GrooveState for cross-lane density ceiling.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `engine/include/poly/types.h`

## Verification

cmake --build build
