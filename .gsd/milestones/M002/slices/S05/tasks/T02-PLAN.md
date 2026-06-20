---
estimated_steps: 1
estimated_files: 2
skills_used: []
---

# T02: Implement ProcessContext to TransportContext bridge

In processor::process(), extract ProcessContext fields (projectTimeMusic, tempo, sampleRate, state flags for playing/looping/cycle) and populate TransportContext. Handle the case where projectTimeMusic is not valid (kProjectTimeMusicValid flag). Detect transport jumps by comparing current ppqStart with expected position from previous block.

## Inputs

- `engine/include/poly/types.h`
- `plugin/source/processor.h`
- `plugin/source/processor.cpp`

## Expected Output

- `plugin/source/processor.cpp`
- `plugin/source/processor.h`

## Verification

cd build && cmake --build . && ctest --output-on-failure
