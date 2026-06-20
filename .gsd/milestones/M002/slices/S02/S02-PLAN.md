# S02: Envelope Superposition v1

**Goal:** Implement shape functions, PPQ-derived envelope phase, and apply envelope modulation to velocity and density targets for both per-lane and global envelopes
**Demo:** Velocity and density evolve over multi-bar periods; overlapping envelopes with non-dividing periods create emergent evolving patterns

## Must-Haves

- Sine/Ramp/Triangle shapes produce correct waveforms; envelope phase is PPQ-derived (no accumulation); velocity and density modulation audible; golden tests prove determinism

## Proof Level

- This slice proves: Unit tests for shape functions + phase math; golden tests for envelope modulation output

## Integration Closure

Envelope evaluation integrates into renderRange after pattern generation and before note emission; reads existing EnvelopeAssign and Envelope types

## Verification

- Golden test output shows velocity modulation across multi-bar periods

## Tasks

- [ ] **T01: Implement shape evaluation functions** `est:30min`
  Create envelope.h/cpp with shape evaluation: evaluateShape(Shape shape, float phase) returning 0..1. Sine: 0.5*(1+sin(2*pi*phase)), Ramp: phase (sawtooth 0 to 1), Triangle: 1-abs(2*phase-1). Phase is always in [0,1). Curve and StepList can return 0.5 for now (placeholder). All functions must be RT-safe (no allocation).
  - Files: `engine/include/poly/envelope.h`, `engine/src/envelope.cpp`, `engine/CMakeLists.txt`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [ ] **T02: Implement PPQ-derived envelope phase calculation** `est:20min`
  Add computeEnvelopePhase(double ppqPosition, float periodBars, float phaseOffset) that derives phase from absolute PPQ. periodBars * 4.0 gives periodPpq (4 PPQ per bar in 4/4). Phase = fmod(ppqPosition / periodPpq + phaseOffset, 1.0). Handle negative fmod results. This must never accumulate.
  - Files: `engine/include/poly/envelope.h`, `engine/src/envelope.cpp`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [ ] **T03: Apply envelope modulation in renderRange** `est:45min`
  In renderRange, after computing base velocity, evaluate all active per-lane envelopes and global envelopes at the note's ppqPosition. For EnvTarget::Velocity: modulate velocity by envelope value * depth. For EnvTarget::Density: modulate probability threshold. Superimpose multiple envelopes multiplicatively for velocity, additively for density. Clamp results to valid ranges.
  - Files: `engine/src/engine.cpp`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [ ] **T04: Add envelope unit and golden tests** `est:30min`
  Write unit tests for: evaluateShape at key phase points (0, 0.25, 0.5, 0.75), computeEnvelopePhase with various periodBars and offsets, envelope phase consistency across loop boundaries. Add a golden test with a multi-lane patch using envelopes with non-dividing periods (e.g. 3 bars and 7 bars) to verify emergent patterns are deterministic.
  - Files: `tests/envelope_tests.cpp`, `tests/golden_tests.cpp`, `tests/golden/envelope_patch_8bars.txt`, `tests/CMakeLists.txt`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

## Files Likely Touched

- engine/include/poly/envelope.h
- engine/src/envelope.cpp
- engine/CMakeLists.txt
- engine/src/engine.cpp
- tests/envelope_tests.cpp
- tests/golden_tests.cpp
- tests/golden/envelope_patch_8bars.txt
- tests/CMakeLists.txt
