---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T03: Implement engine output to IEventList MIDI emission

After renderRange, iterate NoteEventBuffer and emit NoteOn events to the output IEventList. Convert PPQ positions to sample offsets within the current block using tempo and sample rate. Schedule NoteOff events using note duration. Use a fixed-size pending-noteoff buffer (pre-allocated array) to track notes needing NoteOff in future blocks. All RT-safe.

## Inputs

- `engine/include/poly/types.h`
- `plugin/source/processor.h`
- `plugin/source/processor.cpp`

## Expected Output

- `plugin/source/processor.cpp`
- `plugin/source/processor.h`

## Verification

cd build && cmake --build . && ctest --output-on-failure
