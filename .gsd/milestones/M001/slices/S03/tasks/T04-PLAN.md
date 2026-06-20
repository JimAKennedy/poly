---
estimated_steps: 1
estimated_files: 4
skills_used: []
---

# T04: Golden test suite with transport stress scenarios

Set up Google Test in CMake. Write golden tests: (1) straight playback captures reference, (2) same patch+seed reproduces identical output, (3) loop restart produces identical sub-range, (4) different block sizes produce identical events, (5) position jump then continue matches straight-through. Store reference as checked-in .golden files.

## Inputs

- `engine/include/poly/engine.h`
- `engine/include/poly/types.h`

## Expected Output

- `tests/CMakeLists.txt`
- `tests/golden_tests.cpp`

## Verification

cd build && cmake --build . --target poly_tests && ctest --test-dir . --output-on-failure
