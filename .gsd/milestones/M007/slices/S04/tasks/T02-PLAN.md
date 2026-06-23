---
estimated_steps: 5
estimated_files: 1
skills_used: []
---

# T02: Apply timing offset in renderRange

In renderRange(), after swing and humanize adjustments (lines 157-168), add timing offset:
- offsetPpq = timingOffsetMs * tempo / 60000.0
- ppq += offsetPpq
- Clamp: if ppq < tc.ppqStart, clamp to tc.ppqStart (don't go before block start)
Applied after swing and humanize so all three timing modifications stack.

## Inputs

- `engine/src/engine.cpp`

## Expected Output

- `engine.cpp with timing offset`

## Verification

Build + existing tests pass; offset 0 produces identical output
