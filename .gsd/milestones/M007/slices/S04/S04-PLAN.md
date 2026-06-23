# S04: Lane Timing Offset

**Goal:** Add a fixed millisecond timing offset per lane for systematic groove shaping — kick at +5ms, hi-hat at -3ms creates human pocket feel that swing alone cannot achieve
**Demo:** Kick channel offset +5ms, hi-hat -3ms creating a human pocket feel that swing alone cannot achieve

## Must-Haves

- Offset applied to all note events in the lane; offset interacts correctly with swing and humanize; golden tests updated; no RT safety issues

## Proof Level

- This slice proves: Golden tests with timing offsets + Cubase UAT for feel

## Integration Closure

Offset applied at NoteEvent emission in renderRange; converts ms to samples using sample rate and tempo; clamps to prevent negative PPQ positions

## Verification

- None — offset is sub-perceptual in visualization

## Tasks

- [x] **T01: Add timingOffsetMs to LaneConfig** `est:15min`
  Add timingOffsetMs (float, milliseconds, default 0.0) to LaneConfig. Positive = late (behind the beat), negative = early (ahead of the beat). Range: -20.0 to +20.0 ms.
  - Files: `engine/include/poly/types.h`
  - Verify: Build compiles; existing tests pass unchanged

- [x] **T02: Apply timing offset in renderRange** `est:30min`
  In renderRange(), after swing and humanize adjustments (lines 157-168), add timing offset:
  - offsetPpq = timingOffsetMs * tempo / 60000.0
  - ppq += offsetPpq
  - Clamp: if ppq < tc.ppqStart, clamp to tc.ppqStart (don't go before block start)
  Applied after swing and humanize so all three timing modifications stack.
  - Files: `engine/src/engine.cpp`
  - Verify: Build + existing tests pass; offset 0 produces identical output

- [x] **T03: State serialization and golden tests for timing offset** `est:45min`
  Serialize timingOffsetMs in processor.cpp. Add golden tests:
  1. timingOffsetMs=0 matches baseline
  2. timingOffsetMs=5.0 shifts all events forward by 5ms equivalent PPQ
  3. Negative offset shifts events earlier
  4. Offset interacts correctly with swing (both applied)
  - Files: `plugin/source/processor.cpp`, `tests/golden_tests.cpp`
  - Verify: ctest --test-dir build -R golden passes; RT safety check passes

## Files Likely Touched

- engine/include/poly/types.h
- engine/src/engine.cpp
- plugin/source/processor.cpp
- tests/golden_tests.cpp
