---
id: M007
title: "Phrase Architecture and Pattern Evolution"
status: complete
completed_at: 2026-06-23T00:57:43.636Z
key_decisions:
  - Phrase gaps silence all note output rather than holding last note — cleaner for MIDI instruments
  - Mutation uses per-cycle RNG seeded from (lane seed + cycle index) for determinism
  - Phase drift rotates via bar counter rather than fractional accumulation — avoids floating-point drift
  - Kotekan circular references degrade to independent patterns rather than erroring
  - State version bumped to 9 with backward-compatible defaults for all new LaneConfig fields
key_files:
  - engine/include/poly/types.h
  - engine/src/engine.cpp
  - engine/include/poly/presets.h
  - engine/src/scene.cpp
  - plugin/source/processor.cpp
  - plugin/source/controller.cpp
  - plugin/source/ui/phrase_edit_view.cpp
  - plugin/source/ui/lane_grid_view.cpp
  - tests/golden_tests.cpp
  - tests/preset_tests.cpp
lessons_learned:
  - Block-size independence tests are the most reliable way to catch accumulator-vs-absolute bugs in new engine features
  - Factory presets that exercise multiple features simultaneously are excellent integration smoke tests
  - Phrase editor UI layout needs careful rebalancing when adding knobs — went from 5 to 6 knobs requiring spacing recalculation
---

# M007: Phrase Architecture and Pattern Evolution

**Added phrase structure, per-cycle mutation, phase drift, timing offset, and kotekan pairs — transforming Poly from continuous Euclidean loops into evolving, breathing, interlocking grooves.**

## What Happened

M007 delivered five interconnected engine features that dramatically expand Poly's musical range. Phrase structure (S01) lets lanes play for N bars then rest for M bars, creating breathing patterns. Per-cycle mutation (S02) introduces subtle RNG-driven variations each cycle — displaced hits, ghost notes, substitutions — while respecting anchor beats. Phase drift (S03) implements Reich-style gradual phasing where a lane's pattern rotates by one step per bar. Timing offset (S04) adds ±20ms per-lane micro-timing for pocket/groove feel beyond what swing provides. Kotekan pairs (S05) generate complementary gap-filling patterns between linked lanes, with circular reference guards. Four factory presets (S06) demonstrate the features across genres, and state version 9 with updated UI (S07) round out the milestone. All features compose in renderRange(), maintain block-size-independent determinism, and pass RT safety checks.

## Success Criteria Results

All 9 success criteria met: phrase lengths/gaps, mutation, drift, timing offset, kotekan pairs, determinism, 40 golden tests, 4 factory presets, RT safety clean.

## Definition of Done Results

Not provided.

## Requirement Outcomes

Not provided.

## Deviations

None.

## Follow-ups

Kotekan source selection via custom UI dropdown (currently only accessible through Cubase generic editor). Phase drift rate could benefit from a UI knob in the phrase editor. Consider a 'pattern evolution' preset category in a future preset manager UI.
