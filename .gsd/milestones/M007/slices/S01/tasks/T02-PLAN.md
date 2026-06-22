---
estimated_steps: 5
estimated_files: 1
skills_used: []
---

# T02: Implement phrase gating in renderRange

In renderRange(), after computing absStep and ppq, compute phrase phase from absolute PPQ position:
- phraseCyclePpq = (phraseLength + phraseGap) * 4.0 (bars to PPQ)
- phrasePos = fmod(ppq - phraseOffset * 4.0, phraseCyclePpq)
- if phrasePos >= phraseLength * 4.0, skip the step (we're in the gap)
Must handle phraseLength=0 (continuous) as no-op. Must derive from absolute PPQ, not accumulate.

## Inputs

- `engine/src/engine.cpp`
- `engine/include/poly/types.h`

## Expected Output

- `engine/src/engine.cpp with phrase gating`

## Verification

Build + existing tests pass; manual inspection confirms phrase gating logic derives from absolute PPQ
