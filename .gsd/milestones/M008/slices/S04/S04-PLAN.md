# S04: Genre Preset Packs

**Goal:** Create factory presets showcasing M008 features across 5+ distinct rhythmic traditions: Afrobeat 12/8, Balkan aksak, Bossa Nova, Carnatic-inspired, and IDM/glitch. Each preset uses additive cells, timeline patterns, and/or micro-timing maps.
**Demo:** Load genre-specific presets — Afro-House, Balkan, Carnatic-inspired, IDM, Afrobeat patterns using M007+M008 features

## Must-Haves

- At least 5 new factory presets, each using at least one M008 feature (additive cells, timeline, or micro-timing). Presets load correctly and produce musically characteristic output. Preset tests pass.

## Proof Level

- This slice proves: Preset smoke tests verifying each preset loads, produces events, and round-trips through serialization.

## Integration Closure

Presets exercise the full M008 feature set in combination with M007 features. Existing preset indices unchanged.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [ ] **T01: Design 5 genre preset configurations** `est:30min`
  Define preset parameters for: (1) Afrobeat 12/8 with timeline bell, (2) Balkan 7/8 aksak [2+2+3], (3) Bossa Nova with clave timeline and ginga micro-timing, (4) Carnatic-inspired with aksak tala cells, (5) IDM with irregular additive cells and heavy mutation. Document each preset's cell sizes, patterns, and timing maps.
  - Files: `engine/include/poly/presets.h`
  - Verify: Design review — no code changes yet

- [ ] **T02: Implement genre presets in presets.h** `est:1h`
  Add 5 new factory preset functions using additive cells, timeline mode, and micro-timing maps. Update kFactoryPresetCount. Wire into the preset list.
  - Files: `engine/include/poly/presets.h`
  - Verify: cmake --build build && ctest --test-dir build

- [ ] **T03: Preset tests for genre presets** `est:30min`
  Add smoke tests for each new preset: loads without error, produces events over 4 bars, round-trips through serialization with identical output.
  - Files: `tests/preset_tests.cpp`
  - Verify: cmake --build build && ctest --test-dir build

## Files Likely Touched

- engine/include/poly/presets.h
- tests/preset_tests.cpp
