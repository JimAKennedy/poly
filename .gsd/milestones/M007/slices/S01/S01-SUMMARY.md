---
id: S01
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
  - plugin/source/processor.cpp
  - tests/golden_tests.cpp
  - plugin/source/ui/phase_alignment_view.cpp
key_decisions:
  - Phrase units are beats (quarter notes) not bars
  - Phrase phase derived from absolute PPQ per project convention
  - phraseLength=0 means continuous for backward compat
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-22T19:40:29.027Z
blocker_discovered: false
---

# S01: Phrase System

**Per-lane phrase gating with independent len/gap/offset controls in beats, plus golden tests and phase alignment visualization**

## What Happened

Implemented the full phrase gating system: LaneConfig fields (phraseLength, phraseGap, phraseOffset in beats), renderRange gating from absolute PPQ, state serialization with version bump, 5 golden test scenarios, and phase alignment view showing play/gap boundaries. Originally implemented in bars, then converted to beats (PPQ direct) for more idiomatic polymetric control. All features are deterministic and RT-safe.

## Verification

All 197 tests pass including 5 new phrase golden test scenarios. RT safety check clean. Cubase UAT confirms phrase gating, offset shifting, and phase alignment visualization working correctly.

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
