---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T02: Standard MIDI File writer

Implement writeSMF(): takes a span of NoteEvents and writes a valid Type 0 SMF with tempo track. PPQ positions converted to MIDI ticks (480 PPQN standard). Variable-length quantity encoding for delta times. Write to a provided output buffer or stream.

## Inputs

- `engine/include/poly/types.h`

## Expected Output

- `engine/include/poly/smf_writer.h`
- `engine/src/smf_writer.cpp`

## Verification

cmake --build build && ctest --test-dir build -R smf
