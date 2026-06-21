---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T01: MIDI capture ring buffer

Implement MidiCaptureBuffer: fixed-capacity ring buffer storing NoteEvents with absolute PPQ positions. Configurable capture length in bars (default 8). Push from process() after renderRange. Clear on transport jump or capture reset. Pre-allocate in initialize() for RT safety.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `engine/include/poly/midi_capture.h`
- `engine/src/midi_capture.cpp`

## Verification

cmake --build build && ctest --test-dir build -R midi_capture
