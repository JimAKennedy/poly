---
estimated_steps: 7
estimated_files: 1
skills_used: []
---

# T02: Implement kotekan complement generation in renderRange

In renderRange(), when processing a lane with kotekanSourceLane >= 0:
- Look up source lane config
- Generate source lane Euclidean pattern
- Invert: complement[i] = !source[i]
- Use complement pattern instead of own Euclidean generation
- All other processing (envelopes, constraints, velocity, timing) uses this lane's own config
- Handle edge cases: source lane inactive, circular reference (A->B->A), source out of range

## Inputs

- `engine/src/engine.cpp`

## Expected Output

- `engine.cpp with kotekan logic`

## Verification

Build + existing tests pass; kotekanSourceLane=-1 produces identical output
