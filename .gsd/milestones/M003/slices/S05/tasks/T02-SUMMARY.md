---
id: T02
parent: S05
milestone: M003
key_files:
  - engine/include/poly/smf_writer.h
  - engine/src/smf_writer.cpp
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-21T03:21:15.132Z
blocker_discovered: false
---

# T02: Implemented SMF writer: Type 0 MIDI file with tempo track, VLQ encoding, PPQ offset normalization

**Implemented SMF writer: Type 0 MIDI file with tempo track, VLQ encoding, PPQ offset normalization**

## What Happened

writeSMF() produces valid Standard MIDI File Type 0 with 480 PPQN, tempo meta event, sorted note on/off events with delta times, and end-of-track marker. VLQ encoding tested up to 3 bytes. ppqOffset parameter normalizes captured events to start from beat 0. Note-offs sort before note-ons at same tick to prevent hanging notes.

## Verification

Build succeeds, all SMF writer and VLQ tests pass

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build && ctest --test-dir build -R SmfWriter` | 0 | pass | 12000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/include/poly/smf_writer.h`
- `engine/src/smf_writer.cpp`
