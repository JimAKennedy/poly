# S05: VST3 Plugin Integration

**Goal:** Wire EditController parameters, ProcessContext bridge, engine-to-IEventList MIDI output with note-off scheduling, and versioned state serialization
**Demo:** Plugin generates polymetric rhythms in Cubase; save project and reload preserves complete patch; loop and transport jumps handled correctly

## Must-Haves

- Parameters automate in Cubase; ProcessContext fields map correctly to TransportContext; NoteOn/NoteOff events emit to IEventList with correct sample offsets; getState/setState round-trips with kStateVersion

## Proof Level

- This slice proves: Unit tests for sample offset calculation and state serialization; manual Cubase verification checklist

## Integration Closure

Processor bridges host to engine; Controller exposes parameters; state serialization covers full GrooveState

## Verification

- Debug logging for transport state and parameter changes (non-RT thread only)

## Tasks

- [x] **T01: Define and register EditController parameters** `est:60min`
  Define ParamID enum covering: per-lane params (probability, baseVelocity, emphasisProb, ghostFloor, velocitySpread, swingAmount, humanizeMs, noteDuration, active) x 8 lanes + macro params (complexity, density, syncopation, swing, tension, humanize) + global params (activeLaneCount, seed). Register all in EditController::initialize() with appropriate units, ranges, and default values.
  - Files: `plugin/source/plugids.h`, `plugin/source/controller.h`, `plugin/source/controller.cpp`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [x] **T02: Implement ProcessContext to TransportContext bridge** `est:30min`
  In processor::process(), extract ProcessContext fields (projectTimeMusic, tempo, sampleRate, state flags for playing/looping/cycle) and populate TransportContext. Handle the case where projectTimeMusic is not valid (kProjectTimeMusicValid flag). Detect transport jumps by comparing current ppqStart with expected position from previous block.
  - Files: `plugin/source/processor.cpp`, `plugin/source/processor.h`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [x] **T03: Implement engine output to IEventList MIDI emission** `est:60min`
  After renderRange, iterate NoteEventBuffer and emit NoteOn events to the output IEventList. Convert PPQ positions to sample offsets within the current block using tempo and sample rate. Schedule NoteOff events using note duration. Use a fixed-size pending-noteoff buffer (pre-allocated array) to track notes needing NoteOff in future blocks. All RT-safe.
  - Files: `plugin/source/processor.cpp`, `plugin/source/processor.h`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [x] **T04: Implement versioned state serialization** `est:45min`
  Implement getState() writing kStateVersion (int32) then full GrooveState fields using IBStream. Implement setState() reading version, branching on version number, and restoring GrooveState. Include all lane configs, macros, seed, and envelope assignments. Follow the portfolio convention of version-first serialization.
  - Files: `plugin/source/processor.cpp`, `plugin/source/processor.h`, `plugin/source/controller.cpp`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [x] **T05: Add integration tests for plugin bridge** `est:40min`
  Write tests for: PPQ-to-sample-offset conversion accuracy, state serialization round-trip (write then read, compare GrooveState fields), NoteOff scheduling across block boundaries. These test the bridge logic without requiring the VST3 SDK host.
  - Files: `tests/plugin_tests.cpp`, `tests/CMakeLists.txt`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

## Files Likely Touched

- plugin/source/plugids.h
- plugin/source/controller.h
- plugin/source/controller.cpp
- plugin/source/processor.cpp
- plugin/source/processor.h
- tests/plugin_tests.cpp
- tests/CMakeLists.txt
