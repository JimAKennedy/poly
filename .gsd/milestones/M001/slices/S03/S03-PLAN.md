# S03: Timing Determinism Prototype

**Goal:** Implement timing/cycle math in renderRange() and prove it deterministic under transport stress with golden tests
**Demo:** Golden test suite passes: byte-identical event stream under loop, tempo, and jump stress scenarios

## Must-Haves

- Golden test suite passes: byte-identical event stream under loop restart, tempo change, and position jump scenarios. Harness produces audible polymetric output.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Euclidean rhythm algorithm** `est:30min`
  Implement Bjorklund algorithm to distribute hitCount pulses across steps. Pure function in engine, unit tested against known patterns: (3,8), (5,8), (4,16), (1,4), (4,4). Include rotation support.
  - Files: `engine/include/poly/euclidean.h`, `engine/src/euclidean.cpp`, `tests/euclidean_tests.cpp`
  - Verify: cd build && cmake --build . --target poly_tests && ctest --test-dir . -R euclidean

- [x] **T02: Core cycle timing in renderRange** `est:45min`
  Implement the core rendering loop: for each active lane, compute step positions from absolute PPQ using cycle length and subdivision, check which steps fall within [ppqStart, ppqEnd], emit NoteEvent for each hit. No probability or velocity modulation yet — just raw Euclidean hits at baseVelocity. Handle block-boundary alignment correctly.
  - Files: `engine/src/engine.cpp`, `engine/include/poly/engine.h`
  - Verify: cd build && cmake --build . && ./tools/harness/poly_harness 4 120 0.1 | head -20

- [x] **T03: Deterministic position-based RNG for probability and velocity** `est:30min`
  Implement position-seeded RNG: state derived from (seed, laneId, absolute step index) so output is identical regardless of block boundaries. Apply probability gating (skip hits when roll > lane.probability) and velocity spread (baseVelocity +/- spread). No accumulator state — pure function of position.
  - Files: `engine/include/poly/rng.h`, `engine/src/engine.cpp`
  - Verify: cd build && cmake --build . && ./tools/harness/poly_harness 4 120 0.05 > /tmp/a.txt && ./tools/harness/poly_harness 4 120 0.2 > /tmp/b.txt && diff <(grep -v '^#' /tmp/a.txt) <(grep -v '^#' /tmp/b.txt)

- [x] **T04: Golden test suite with transport stress scenarios** `est:45min`
  Set up Google Test in CMake. Write golden tests: (1) straight playback captures reference, (2) same patch+seed reproduces identical output, (3) loop restart produces identical sub-range, (4) different block sizes produce identical events, (5) position jump then continue matches straight-through. Store reference as checked-in .golden files.
  - Files: `tests/CMakeLists.txt`, `tests/golden_tests.cpp`, `tests/golden/`, `CMakeLists.txt`
  - Verify: cd build && cmake --build . --target poly_tests && ctest --test-dir . --output-on-failure

## Files Likely Touched

- engine/include/poly/euclidean.h
- engine/src/euclidean.cpp
- tests/euclidean_tests.cpp
- engine/src/engine.cpp
- engine/include/poly/engine.h
- engine/include/poly/rng.h
- tests/CMakeLists.txt
- tests/golden_tests.cpp
- tests/golden/
- CMakeLists.txt
