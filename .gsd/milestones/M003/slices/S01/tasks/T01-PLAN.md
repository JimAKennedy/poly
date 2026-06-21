---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T01: Implement remaining envelope targets in renderRange

Add switch cases in renderRange() for the 6 unhandled EnvTarget values: Probability (refine existing density-style), AccentBias (modulate emphasisProb), NoteLength (modulate note duration), TimingLooseness (modulate humanizeMs equivalent), ActivationWeight (modulate lane active probability per-cycle), FillLikelihood (add fill notes at envelope peaks). Each target needs clear modulation semantics: multiplicative for velocity-like targets, additive for probability-like targets.

## Inputs

- `engine/include/poly/types.h`
- `engine/src/engine.cpp`
- `docs/roadmap.md`

## Expected Output

- `engine/src/engine.cpp`

## Verification

cmake --build build && ctest --test-dir build -R envelope
