---
id: S01
parent: M003
milestone: M003
provides:
  - (none)
requires:
  []
affects:
  []
key_files: []
key_decisions:
  - FillLikelihood/ActivationWeight use separate RNG channels (4, 5) to maintain determinism independence
  - Envelope evaluation moved before pattern check to enable fill notes on non-pattern steps
  - State version bumped 1->2 with full v1 backwards compatibility
  - Curve uses exp-based easing; StepList uses 16 fixed entries (no heap allocation)
patterns_established:
  - Per-target modulation semantics: multiplicative for scaling targets, additive for probability/bias targets, gate-based for activation targets
  - evaluateShapeFull() takes full Envelope struct for shapes needing extra parameters, delegates to evaluateShape() for simple shapes
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-21T01:28:20.122Z
blocker_discovered: false
---

# S01: Extended Envelope System

**All 8 envelope targets operational with Curve/StepList shapes, state v2 serialization, and comprehensive golden tests**

## What Happened

Extended the envelope modulation system from 2 targets (Velocity, Density) to all 8 planned targets. Restructured renderRange() to evaluate envelopes before the pattern check, enabling FillLikelihood to inject notes on non-pattern steps. Each target has carefully chosen modulation semantics: multiplicative for Velocity and NoteLength, additive for Probability/AccentBias/TimingLooseness, and gate-based for ActivationWeight and FillLikelihood. Added two new envelope shapes: Curve (exponential easing with curvature parameter) and StepList (quantized lookup table with 16 entries). Bumped state serialization to v2 with full backwards compatibility for v1 presets. Added 12 new tests covering all targets and shapes, plus a golden test exercising all new targets simultaneously across block sizes.

## Verification

133/133 tests pass including 15 golden determinism tests. New tests cover: all 8 envelope targets individually, Curve shape with positive/negative/zero curvature, StepList lookup and empty fallback, zero-depth neutrality across all targets, and block-size independence for extended targets.

## Requirements Advanced

None.

## Requirements Validated

None.

## New Requirements Surfaced

None.

## Requirements Invalidated or Re-scoped

None.

## Operational Readiness

None.

## Deviations

None.

## Known Limitations

None.

## Follow-ups

None.

## Files Created/Modified

- `engine/src/engine.cpp` — Restructured renderRange() with all 8 EnvTarget cases, FillLikelihood gate, ActivationWeight suppression
- `engine/include/poly/types.h` — Added curvature, stepValues[16], stepCount to Envelope struct
- `engine/src/envelope.cpp` — Added evaluateShapeFull() for Curve and StepList shapes
- `engine/include/poly/envelope.h` — Declared evaluateShapeFull()
- `engine/include/poly/state_io.h` — Bumped to v2 with curvature/stepList serialization and v1 backwards compat
- `tests/envelope_tests.cpp` — 12 new tests for all targets and shapes
- `tests/golden_tests.cpp` — Added ExtendedEnvelopeTargetsBlockIndependence golden test
