---
id: T01
parent: S01
milestone: M003
key_files:
  - engine/src/engine.cpp
key_decisions:
  - FillLikelihood uses RNG channel 4, ActivationWeight uses channel 5 to avoid collision with existing channels 0-3
  - Envelope evaluation moved before pattern check to enable fill notes on non-pattern steps
  - AccentBias is additive to emphasisProb, NoteLength is multiplicative on duration
duration: 
verification_result: passed
completed_at: 2026-06-21T01:26:00.935Z
blocker_discovered: false
---

# T01: Implemented all 8 envelope targets in renderRange with restructured flow for FillLikelihood and ActivationWeight

**Implemented all 8 envelope targets in renderRange with restructured flow for FillLikelihood and ActivationWeight**

## What Happened

Added switch cases for Probability, AccentBias, NoteLength, TimingLooseness, ActivationWeight, and FillLikelihood. Restructured the inner loop to evaluate envelopes before the pattern check so FillLikelihood can add notes on non-pattern steps. ActivationWeight uses a separate RNG channel (5) to suppress notes. AccentBias modulates emphasisProb. TimingLooseness adds to humanize jitter. NoteLength multiplies note duration. Used a lambda to deduplicate per-lane and global envelope evaluation.

## Verification

cmake --build build && ctest --test-dir build: 133/133 pass

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `ctest --test-dir build --output-on-failure` | 0 | 133/133 tests pass | 1630ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/src/engine.cpp`
