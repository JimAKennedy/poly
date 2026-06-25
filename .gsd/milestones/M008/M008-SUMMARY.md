---
id: M008
title: "Rhythmic Grammar Extensions"
status: complete
completed_at: 2026-06-23T18:34:00.388Z
key_decisions:
  - Additive cells use cellCount == cycle.steps for Euclidean distribution
  - CrossRhythmView reads full lane configs via PolyController::cachedState() rather than parameter-only access
  - Display span auto-scales via LCM of cycle lengths capped at 8 bars
  - Existing parametric preset tests auto-cover new presets via kFactoryPresetCount loop
key_files:
  - engine/include/poly/types.h
  - engine/include/poly/presets.h
  - engine/src/engine.cpp
  - plugin/source/ui/cross_rhythm_view.cpp
  - plugin/source/ui/cross_rhythm_view.h
lessons_learned:
  - Genre presets that exercise multiple M008 features simultaneously serve as effective integration tests
  - Auto-scaling timeline via LCM handles diverse cycle combinations without manual configuration
---

# M008: Rhythmic Grammar Extensions

**Unlocked non-Western and additive rhythmic structures with aksak meters, timeline lanes, micro-timing maps, genre presets, and cross-rhythm visualization**

## What Happened

M008 extended Poly's rhythmic vocabulary beyond standard Western time signatures. S01 added additive/aksak meter support with unequal cell sequences (e.g. [2+2+3] for 7/8). S02 introduced timeline lane mode providing invariant reference patterns immune to macro changes. S03 added per-step micro-timing maps enabling groove templates like Brazilian ginga or J Dilla pocket. S04 created 5 genre preset packs (Afrobeat 12/8, Balkan Aksak, Bossa Nova, Carnatic Tala, IDM Glitch) that showcase these features in musically authentic configurations. S05 added a cross-rhythm visualization view showing how all lanes' cycles align and diverge, with convergence point markers. Total: 23 tasks across 5 slices, 237 tests passing.

## Success Criteria Results

All 5 success criteria met: additive cells, timeline mode, micro-timing maps, 5 genre presets, cross-rhythm visualization.

## Definition of Done Results

Not provided.

## Requirement Outcomes

Not provided.

## Deviations

None.

## Follow-ups

None.
