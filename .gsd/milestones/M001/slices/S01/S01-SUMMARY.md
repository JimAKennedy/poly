---
id: S01
parent: M001
milestone: M001
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - CMakeLists.txt
  - engine/CMakeLists.txt
  - engine/include/poly/types.h
  - engine/include/poly/engine.h
  - engine/src/engine.cpp
  - plugin/CMakeLists.txt
  - plugin/source/factory.cpp
  - plugin/source/plugids.h
  - plugin/source/processor.h
  - plugin/source/processor.cpp
  - plugin/source/controller.h
  - plugin/source/controller.cpp
  - cmake/jk_warnings.cmake
key_decisions:
  - VST3 SDK fetched via FetchContent (not local path) for reproducibility
  - Engine uses pre-allocated NoteEventBuffer (std::array<NoteEvent, 256>) for RT safety
  - Processor declares stereo audio out + MIDI event out (Cubase requires audio bus even for MIDI instruments)
  - State serialization includes version int32 from day one per project convention
  - Platform entry points (macmain.cpp etc.) included directly in plugin sources since static lib linking strips unreferenced symbols
patterns_established:
  - Engine isolation: poly_engine has zero VST3 SDK dependency
  - jk_warnings.cmake suppresses SDK warnings including deprecation
  - Versioned state: kStateVersion written as first int32 in getState()
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-20T19:52:24.862Z
blocker_discovered: false
---

# S01: Build System and VST3 Skeleton

**CMake build system with VST3 SDK FetchContent, poly_engine static lib, and poly_plugin VST3 instrument skeleton producing a clean .vst3 bundle**

## What Happened

Established the full build infrastructure for Poly. The top-level CMakeLists.txt fetches the Steinberg VST3 SDK v3.7.12 via FetchContent, sets C++20, and includes the shared jk_warnings.cmake module (extended to suppress SDK deprecation warnings). poly_engine is a standalone STATIC library with no VST3 SDK dependency — it defines TransportContext, NoteEvent, NoteEventBuffer (pre-allocated for RT safety), and the Engine::renderRange() contract. poly_plugin uses smtg_add_vst3plugin with factory registration (kInstrumentSynth), PolyProcessor (AudioEffect with stereo audio out + MIDI event out, versioned state), and PolyController stub. Platform entry points (macmain.cpp/dllmain.cpp/linuxmain.cpp) are included per-platform. The build produces a valid .vst3 bundle with zero warnings from project source files.

## Verification

Clean build with zero project warnings, .vst3 bundle at build/VST3/Debug/poly_plugin.vst3, engine isolation confirmed via grep (no VST3 SDK references in engine/)

## Requirements Advanced

None.

## Requirements Validated

None.

## New Requirements Surfaced

None.

## Requirements Invalidated or Re-scoped

None.

## Operational Readiness

None.

## Deviations

None.

## Known Limitations

None.

## Follow-ups

None.

## Files Created/Modified

None.
