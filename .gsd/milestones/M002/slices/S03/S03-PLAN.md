# S03: Swing and Humanization

**Goal:** Implement swing offset on even subdivisions, humanize timing jitter using deterministic RNG, and note duration control
**Demo:** Notes shift timing on even subdivisions (swing), jitter slightly (humanize), and have controllable durations

## Must-Haves

- Swing shifts even-subdivision notes by configurable amount; humanize adds bounded PPQ jitter deterministically; note duration is configurable per lane; golden tests updated

## Proof Level

- This slice proves: Unit tests for swing math + humanize bounds; golden tests for timing output

## Integration Closure

Modifies ppqPosition and duration in renderRange note emission; uses existing deterministicRand for humanize

## Verification

- Golden test output shows shifted note positions and variable durations

## Tasks

- [x] **T01: Add swing and duration fields to LaneConfig** `est:15min`
  Add float swingAmount (0..1, 0=no swing, 1=full triplet swing) and float noteDuration (in PPQ, default 0 meaning auto = stepPpq*0.5) to LaneConfig. Keep defaults backward-compatible so existing tests pass without changes.
  - Files: `engine/include/poly/types.h`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [x] **T02: Implement swing offset in renderRange** `est:30min`
  After computing the note's ppqPosition, check if the cycle-local step index is even (0-indexed, so steps 1, 3, 5... get swung). Shift ppqPosition forward by swingAmount * stepPpq * (1.0/3.0). This creates the classic swing feel. Ensure the shifted position still falls within [ppqStart, ppqEnd).
  - Files: `engine/src/engine.cpp`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [x] **T03: Implement humanize timing jitter** `est:25min`
  Apply humanizeMs as bounded PPQ jitter to each note. Convert humanizeMs to PPQ using tempo: jitterPpq = humanizeMs * tempo / 60000.0. Use deterministicRand(seed, laneId, absStep, channel=3) to generate a random offset in [-jitterPpq, +jitterPpq]. Apply after swing. Clamp to keep note within the current block boundaries.
  - Files: `engine/src/engine.cpp`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [x] **T04: Implement configurable note duration** `est:10min`
  Replace the hardcoded ev.duration = sPpq * 0.5 with cfg.noteDuration when noteDuration > 0, otherwise fall back to sPpq * 0.5 as default. This allows per-lane control of note length.
  - Files: `engine/src/engine.cpp`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [x] **T05: Add swing and humanize tests plus golden updates** `est:25min`
  Write unit tests for: swing offset calculation at various swingAmount values, humanize jitter bounds, note duration configuration. Update the default golden test reference if needed. Add a new golden test with swing=0.5 and humanize=5ms to verify deterministic timing.
  - Files: `tests/euclidean_tests.cpp`, `tests/golden_tests.cpp`, `tests/golden/default_patch_4bars.txt`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

## Files Likely Touched

- engine/include/poly/types.h
- engine/src/engine.cpp
- tests/euclidean_tests.cpp
- tests/golden_tests.cpp
- tests/golden/default_patch_4bars.txt
