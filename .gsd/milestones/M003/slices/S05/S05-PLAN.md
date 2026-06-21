# S05: MIDI Export Workflow

**Goal:** Enable MIDI capture and export so users can freeze generated grooves into standard MIDI files for downstream editing in Cubase
**Demo:** User captures a generated groove as MIDI on a Cubase track for further editing

## Must-Haves

- Ring buffer captures last N bars of generated MIDI. Export produces valid Standard MIDI File (type 0). Export triggered via parameter or mechanism accessible from Cubase. Captured MIDI is tempo-independent (stored in PPQ).

## Proof Level

- This slice proves: Unit tests for ring buffer, SMF writer byte-level verification, round-trip import test

## Integration Closure

Processor accumulates events in capture buffer. Export runs on non-RT thread (message-based trigger). No engine changes.

## Verification

- Capture buffer fill level exposed as read-only output parameter.

## Tasks

- [x] **T01: MIDI capture ring buffer** `est:1.5h`
  Implement MidiCaptureBuffer: fixed-capacity ring buffer storing NoteEvents with absolute PPQ positions. Configurable capture length in bars (default 8). Push from process() after renderRange. Clear on transport jump or capture reset. Pre-allocate in initialize() for RT safety.
  - Files: `engine/include/poly/midi_capture.h`, `engine/src/midi_capture.cpp`, `engine/CMakeLists.txt`
  - Verify: cmake --build build && ctest --test-dir build -R midi_capture

- [x] **T02: Standard MIDI File writer** `est:2h`
  Implement writeSMF(): takes a span of NoteEvents and writes a valid Type 0 SMF with tempo track. PPQ positions converted to MIDI ticks (480 PPQN standard). Variable-length quantity encoding for delta times. Write to a provided output buffer or stream.
  - Files: `engine/include/poly/smf_writer.h`, `engine/src/smf_writer.cpp`, `engine/CMakeLists.txt`
  - Verify: cmake --build build && ctest --test-dir build -R smf

- [x] **T03: Export trigger and plugin integration** `est:1.5h`
  Add export trigger parameter (kExportTrigger) that when set to 1.0 initiates capture export. In process(), detect trigger and defer actual file write to a non-RT path (use IMessage to controller, controller writes file). Add capture length parameter. Wire up in processor and controller.
  - Files: `plugin/source/processor.cpp`, `plugin/source/processor.h`, `plugin/source/controller.cpp`, `plugin/source/plugids.h`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T04: MIDI export tests** `est:1h`
  Test ring buffer wrap-around behavior. Test SMF writer produces valid MIDI header, track header, and note events. Verify variable-length encoding. Test round-trip: generate events -> capture -> export -> parse and compare. Test capture clear on transport jump.
  - Files: `tests/midi_capture_tests.cpp`, `tests/smf_writer_tests.cpp`
  - Verify: cmake --build build && ctest --test-dir build -R midi

## Files Likely Touched

- engine/include/poly/midi_capture.h
- engine/src/midi_capture.cpp
- engine/CMakeLists.txt
- engine/include/poly/smf_writer.h
- engine/src/smf_writer.cpp
- plugin/source/processor.cpp
- plugin/source/processor.h
- plugin/source/controller.cpp
- plugin/source/plugids.h
- tests/midi_capture_tests.cpp
- tests/smf_writer_tests.cpp
