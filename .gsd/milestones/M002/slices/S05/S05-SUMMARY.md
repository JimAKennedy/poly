---
id: S05
parent: M002
milestone: M002
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - plugin/source/plugids.h
  - plugin/source/controller.cpp
  - plugin/source/controller.h
  - plugin/source/processor.cpp
  - plugin/source/processor.h
  - tests/plugin_tests.cpp
key_decisions:
  - Per-lane params use base + lane*stride addressing for compact ParamID space
  - Transport jump detection via ppq position comparison, not accumulator
  - Note-off scheduling via fixed-size pendingNoteOffs array, not heap container
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-20T22:19:59.827Z
blocker_discovered: false
---

# S05: VST3 Plugin Integration

**Full VST3 bridge: param IDs, transport, MIDI emission, note-off scheduling, state serialization**

## What Happened

Wired the complete VST3 plugin bridge. EditController registers all parameters (per-lane x 8, macros x 6, globals). Processor bridges ProcessContext to TransportContext with transport jump detection. Engine NoteBuffer events emitted to output IEventList with correct sample offsets. Note-off scheduling tracks active notes. State serialization uses kStateVersion convention — getState writes version then GrooveState, setState branches on version. All RT-safe: no allocation in process().

## Verification

101/101 tests pass including plugin integration tests for param registration, state round-trip, and MIDI output.

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
