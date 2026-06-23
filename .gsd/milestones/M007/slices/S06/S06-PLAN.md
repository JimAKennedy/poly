# S06: Updated Factory Presets

**Goal:** Create factory presets that showcase M007 features across different musical contexts — Afro-House phrases, Reich phasing, kotekan interlocking, and pocket groove
**Demo:** New preset bank demonstrating phrase breathing, mutation, drift, kotekan, and groove pocket across genres

## Must-Haves

- At least 4 new presets covering: Afro-House phrases, Reich phasing, kotekan interlocking, pocket groove; all presets sound musically interesting in Cubase; preset_tests verify all load without error

## Proof Level

- This slice proves: Cubase UAT with subjective musical quality assessment

## Integration Closure

Presets added to existing preset bank with state version compatibility; preset_tests verify all new presets load without error

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Design and implement Afro-House Phrases preset** `est:30min`
  Create preset with 4-6 lanes using offset phrase lengths (1, 2, 3, 4 bars) with small gaps. Shaker continuous, conga on 2-bar phrase, djembe on 3-bar phrase. Demonstrates Afro-House offset loop behavior from research paper section 4.2.
  - Files: `engine/include/poly/presets.h`
  - Verify: Build compiles; preset_tests pass

- [x] **T02: Design and implement Reich Phasing preset** `est:30min`
  Create preset with 2 lanes: identical E(5,12) pattern, same MIDI note, one with driftRate=0.25 (shifts 1 step every 4 bars). Demonstrates gradual phase separation creating resultant patterns per research paper section 8.2.
  - Files: `engine/include/poly/presets.h`
  - Verify: Build compiles; preset_tests pass

- [x] **T03: Design and implement Kotekan Interlock preset** `est:30min`
  Create preset with a kotekan pair: one lane plays E(3,8) on a high woodblock, its complement plays the gaps on a low woodblock. Add a steady pulse lane and a ghost shimmer. Demonstrates Balinese interlocking per research paper section 5.2.
  - Files: `engine/include/poly/presets.h`
  - Verify: Build compiles; preset_tests pass

- [x] **T04: Design and implement Pocket Groove preset** `est:30min`
  Create preset with per-lane timing offsets: kick at +3ms, snare at -2ms, hi-hat at +1ms. Combined with light mutation (0.15) for organic variation. Demonstrates J Dilla / MPC pocket feel per research paper section 9.1.
  - Files: `engine/include/poly/presets.h`
  - Verify: Build compiles; preset_tests pass

- [x] **T05: Update preset infrastructure and tests** `est:30min`
  Update kFactoryPresetCount, makeFactoryPreset(), getFactoryPresetInfo(), and controller preset list to include all 4 new presets. Update preset_tests to cover new presets.
  - Files: `engine/include/poly/presets.h`, `plugin/source/controller.cpp`, `tests/preset_tests.cpp`
  - Verify: ctest --test-dir build -R preset passes; all 9 presets load without error

## Files Likely Touched

- engine/include/poly/presets.h
- plugin/source/controller.cpp
- tests/preset_tests.cpp
