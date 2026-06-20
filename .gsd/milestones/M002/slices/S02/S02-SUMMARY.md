---
id: S02
parent: M002
milestone: M002
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - engine/include/poly/envelope.h
  - engine/src/envelope.cpp
  - engine/src/engine.cpp
  - tests/envelope_tests.cpp
  - tests/golden_tests.cpp
key_decisions: []
patterns_established:
  - Velocity envelopes superimpose multiplicatively: velMod *= (1 - depth*(1-value))
  - Density envelopes adjust probability additively: probMod += depth*(value*2-1)
  - Envelope phase always derived from absolute PPQ via computeEnvelopePhase()
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-20T21:27:34.825Z
blocker_discovered: false
---

# S02: Envelope Superposition v1

**Shape functions, PPQ-derived phase, and velocity/density modulation with per-lane and global envelope superposition**

## What Happened

Implemented the full envelope evaluation pipeline: evaluateShape() for Sine/Ramp/Triangle (Curve/StepList placeholder), computeEnvelopePhase() deriving from absolute PPQ (never accumulating), and renderRange integration. Velocity envelopes superimpose multiplicatively, density envelopes adjust probability additively. Both per-lane envelopes (via EnvelopeAssign) and global envelopes (via GrooveState) are supported. Non-dividing periods (3-bar, 5-bar, 7-bar) create emergent evolving patterns as designed.

## Verification

50/50 tests pass: 4 shape evaluation, 4 phase calculation, 8 integration, 2 golden determinism (block-size independence and loop restart with envelopes), plus 32 pre-existing tests unchanged.

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

- `engine/include/poly/envelope.h` — New: shape evaluation and phase calculation declarations
- `engine/src/envelope.cpp` — New: evaluateShape() and computeEnvelopePhase() implementations
- `engine/CMakeLists.txt` — Added envelope.cpp to poly_engine sources
- `engine/src/engine.cpp` — Integrated envelope modulation into renderRange
- `tests/envelope_tests.cpp` — New: 16 envelope unit and integration tests
- `tests/golden_tests.cpp` — Added 2 envelope determinism golden tests
- `tests/CMakeLists.txt` — Added envelope_tests.cpp to test executable
