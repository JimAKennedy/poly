---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T04: MIDI export tests

Test ring buffer wrap-around behavior. Test SMF writer produces valid MIDI header, track header, and note events. Verify variable-length encoding. Test round-trip: generate events -> capture -> export -> parse and compare. Test capture clear on transport jump.

## Inputs

- `engine/include/poly/midi_capture.h`
- `engine/include/poly/smf_writer.h`

## Expected Output

- `tests/midi_capture_tests.cpp`
- `tests/smf_writer_tests.cpp`

## Verification

cmake --build build && ctest --test-dir build -R midi
