---
id: S03
parent: M002
milestone: M002
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - engine/include/poly/types.h
  - engine/src/engine.cpp
  - tests/swing_humanize_tests.cpp
  - tests/golden_tests.cpp
key_decisions:
  - Removed post-jitter boundary check to ensure block-size independence — notes emit based on nominal step position
  - Swing applied to odd cycle-steps (1,3,5) rather than even, matching standard swing convention where the offbeat is delayed
  - Humanize uses deterministicRand channel 3 to avoid collision with existing channels 0-2
patterns_established:
  - Timing modifiers (swing, humanize) applied after pattern matching but before note emission
  - No post-modification boundary filtering — determinism preserved by emitting based on nominal position
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-20T21:34:12.344Z
blocker_discovered: false
---

# S03: Swing and Humanization

**Implemented swing offset, humanize timing jitter, and configurable note duration with 14 new tests**

## What Happened

Added swing (shifts odd cycle-steps forward by swingAmount * stepPpq / 3 for classic swing feel), humanize (bounded deterministic PPQ jitter via deterministicRand channel 3), and configurable note duration (per-lane noteDuration field, falling back to stepPpq*0.5). All three features integrate cleanly into renderRange's existing note emission pipeline. Key design decision: removed post-jitter boundary check to ensure block-size-independent determinism — notes emit based on nominal step position regardless of timing offset.

## Verification

64/64 tests pass including 14 new tests: 4 swing (offset correctness, full triplet, determinism), 5 humanize (jitter presence, bounds, determinism, zero, different seed), 2 combined (swing+humanize bounds), 3 duration (default, custom, staccato), plus 1 golden determinism test for block independence

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
