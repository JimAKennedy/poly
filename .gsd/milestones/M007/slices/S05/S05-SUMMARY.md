---
id: S05
parent: M007
milestone: M007
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - engine/include/poly/types.h
  - engine/src/engine.cpp
  - engine/include/poly/state_io.h
  - plugin/source/plugids.h
  - plugin/source/controller.cpp
  - plugin/source/processor.cpp
  - plugin/source/ui/phrase_edit_view.cpp
  - plugin/source/ui/lane_grid_view.cpp
  - tests/golden_tests.cpp
key_decisions:
  - Kotekan complement uses source lane's cycle steps; extra steps beyond source length treated as hits
  - Circular reference detection is pairwise only (A→B→A), not transitive chains — sufficient for 8 lanes
  - Timing offset knob added alongside Mut/Drift in phrase editor with rebalanced 6-knob layout
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-22T21:17:16.876Z
blocker_discovered: false
---

# S05: Kotekan Pairs

**Kotekan pair mode generates complementary interlocking patterns between linked lanes**

## What Happened

Implemented Balinese kotekan-style interlocking patterns: a lane with kotekanSourceLane set generates the exact complement of the source lane's Euclidean pattern. E(3,8) source produces 3 hits; complement fills the 5 gaps. Each lane keeps its own MIDI note, velocity, and processing chain. Circular references (A→B→A) degrade gracefully to independent patterns. Also added timing offset knob to the phrase editor UI (6 knobs total with rebalanced layout) and kotekan link indicator in the lane grid view.

## Verification

216/216 tests pass. 5 new kotekan golden tests verify: no-source baseline match, complement hit counts, per-voice velocity independence, circular reference fallback, block-size determinism. clang-format and RT safety checks pass.

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

None.
