---
id: T03
parent: S01
milestone: M001
key_files:
  - plugin/CMakeLists.txt
  - plugin/source/factory.cpp
  - plugin/source/plugids.h
  - plugin/source/processor.h
  - plugin/source/processor.cpp
  - plugin/source/controller.h
  - plugin/source/controller.cpp
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-20T19:51:50.353Z
blocker_discovered: false
---

# T03: poly_plugin VST3 instrument skeleton with factory, processor, controller producing .vst3 bundle

**poly_plugin VST3 instrument skeleton with factory, processor, controller producing .vst3 bundle**

## What Happened

Created plugin/ directory with: CMakeLists.txt using smtg_add_vst3plugin + sdk link + platform entry points (macmain.cpp/dllmain.cpp/linuxmain.cpp); factory.cpp with BEGIN_FACTORY_DEF registering PolyProcessor as kInstrumentSynth and PolyController; processor.h/.cpp with AudioEffect subclass implementing initialize (stereo audio out + MIDI event out), process (silence output), and versioned getState/setState; controller.h/.cpp with EditController stub; plugids.h with unique FUIDs. Cubase requires an audio output bus even for MIDI-only instruments — declared as stereo.

## Verification

cmake --build produces .vst3 bundle at build/VST3/Debug/poly_plugin.vst3

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build --config Debug` | 0 | pass | 5000ms |
| 2 | `find build -name '*.vst3' -type d` | 0 | pass - build/VST3/Debug/poly_plugin.vst3 | 100ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `plugin/CMakeLists.txt`
- `plugin/source/factory.cpp`
- `plugin/source/plugids.h`
- `plugin/source/processor.h`
- `plugin/source/processor.cpp`
- `plugin/source/controller.h`
- `plugin/source/controller.cpp`
