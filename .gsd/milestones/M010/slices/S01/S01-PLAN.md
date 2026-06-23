# S01: VST3 Parameter Wiring for Core Euclidean Params

**Goal:** Add 5 new per-lane VST3 parameters for core Euclidean fields and wire them end-to-end through processor, controller, presets
**Demo:** Change steps/hits/subdivision/rotation/note from DAW automation and hear the pattern change

## Must-Haves

- Complete the planned slice outcomes.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Add new param IDs and expand parameter block** `est:30min`
  Add kSteps, kSubdivision, kHits, kRotation, kMidiNote as offsets 16-20 in plugids.h. Expand kParamsPerLane to 24. Wire applyParameter in processor.cpp. Register params in controller.cpp. Add setComponentState sync.
  - Files: `plugin/source/plugids.h`, `plugin/source/processor.cpp`, `plugin/source/controller.cpp`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T02: Update preset reset and apply for new params** `est:15min`
  Update header_view.cpp applyPreset and resetToInit to include Steps, Subdivision, Hits, Rotation, MidiNote params. Each preset must set explicit values for all 5 new params.
  - Files: `plugin/source/ui/header_view.cpp`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T03: Verify build and tests pass with expanded param block** `est:10min`
  Build and run full test suite. Update golden test baseline if param block expansion changes default output.
  - Files: `tests/golden/default_patch_4bars.txt`
  - Verify: cmake --build build && ctest --test-dir build --output-on-failure

## Files Likely Touched

- plugin/source/plugids.h
- plugin/source/processor.cpp
- plugin/source/controller.cpp
- plugin/source/ui/header_view.cpp
- tests/golden/default_patch_4bars.txt
