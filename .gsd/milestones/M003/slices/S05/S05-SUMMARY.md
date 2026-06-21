---
id: S05
parent: M003
milestone: M003
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - engine/include/poly/midi_capture.h
  - engine/src/midi_capture.cpp
  - engine/include/poly/smf_writer.h
  - engine/src/smf_writer.cpp
  - plugin/source/processor.h
  - plugin/source/processor.cpp
  - plugin/source/controller.cpp
  - plugin/source/plugids.h
  - tests/midi_capture_tests.cpp
  - tests/smf_writer_tests.cpp
key_decisions:
  - Atomic staging buffer pattern for RT-safe export (process copies to staging, main thread reads)
  - 480 PPQN standard resolution for SMF output
  - Note-offs sorted before note-ons at same tick to prevent hanging notes
  - Capture buffer clears on transport jump to avoid stale data
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-21T03:21:48.329Z
blocker_discovered: false
---

# S05: MIDI Export Workflow

**MIDI capture ring buffer, Standard MIDI File writer, and RT-safe export trigger with IMessage retrieval**

## What Happened

Implemented the full MIDI export pipeline: (1) MidiCaptureBuffer — 2048-event RT-safe ring buffer that accumulates NoteEvents with absolute PPQ positions during playback, clears on transport jump. (2) SMF writer — produces valid Type 0 Standard MIDI Files at 480 PPQN with tempo meta event, proper VLQ delta encoding, note-off-before-note-on ordering at same tick, and PPQ offset normalization. (3) Plugin integration — processor pushes events after renderRange, uses atomic staging buffer pattern for thread-safe export: process() snapshots to staging array on trigger, processor's notify() generates SMF and sends binary data via IMessage to controller. Added Export unit with kExportTrigger, kCaptureLength (1-32 bars), and kCaptureReady parameters. 19 new tests (8 capture buffer, 11 SMF writer) bring total to 177.

## Verification

All 177 tests pass including 8 MidiCaptureBuffer tests (push, clear, wrap-around, extract, sort) and 11 SmF writer tests (VLQ encoding, header/track validation, tempo, notes, ordering, offset, edge cases).

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
