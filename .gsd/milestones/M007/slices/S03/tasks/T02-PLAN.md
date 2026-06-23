---
estimated_steps: 6
estimated_files: 1
skills_used: []
---

# T02: Implement drift in renderRange

In renderRange(), after computing cycleStep, apply drift:
- Compute bar position: barPos = ppq / 4.0
- driftSteps = floor(barPos * driftRate) (integer steps of drift)
- effectiveRotation = (cfg.rotation + driftSteps) % stepsInCycle
- Re-generate or re-index Euclidean pattern with effectiveRotation
Key: driftSteps derived from absolute PPQ, not accumulated. Transport jump to any position gets the correct drift for that bar.

## Inputs

- `engine/src/engine.cpp`

## Expected Output

- `engine.cpp with drift logic`

## Verification

Build + existing tests pass; drift rate 0 produces identical output
