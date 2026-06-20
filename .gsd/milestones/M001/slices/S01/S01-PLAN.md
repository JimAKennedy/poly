# S01: Build System and VST3 Skeleton

**Goal:** Stand up CMake build system with VST3 SDK integration, create poly_engine static library scaffold and poly_plugin VST3 instrument skeleton. An empty VST3 plugin builds cleanly with all warnings enabled and produces a valid .vst3 bundle.
**Demo:** Empty VST3 plugin instantiates in Cubase without error; clean build with all warnings enabled

## Must-Haves

- 1. cmake --build produces a .vst3 bundle without errors or warnings\n2. poly_engine compiles as a standalone static library without VST3 SDK headers\n3. poly_plugin links poly_engine and the VST3 SDK\n4. Factory registers processor + controller with correct class IDs\n5. jk_warnings.cmake applied to both targets

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Top-level CMake with VST3 SDK via FetchContent** `est:15min`
  Create the top-level CMakeLists.txt that:
  1. Sets project(poly) with C++20 standard
  2. Fetches the Steinberg VST3 SDK via FetchContent from GitHub
  3. Includes cmake/jk_warnings.cmake
  4. Adds engine/ and plugin/ subdirectories
  5. Adds VST3 SDK build output paths
  - Files: `CMakeLists.txt`
  - Verify: cmake -B build -DCMAKE_BUILD_TYPE=Debug completes without error

- [x] **T02: poly_engine static library scaffold** `est:15min`
  Create engine/ directory with:
  1. engine/CMakeLists.txt — add_library(poly_engine STATIC) with public include path
  2. engine/include/poly/engine.h — Engine class declaration with renderRange stub
  3. engine/include/poly/types.h — NoteEvent, TransportContext, basic type declarations
  4. engine/src/engine.cpp — empty renderRange implementation
  Must NOT link or include any VST3 SDK headers. Apply jk_target_warnings.
  - Files: `engine/CMakeLists.txt`, `engine/include/poly/engine.h`, `engine/include/poly/types.h`, `engine/src/engine.cpp`
  - Verify: poly_engine target compiles as standalone static lib; grep confirms no VST3 includes

- [x] **T03: poly_plugin VST3 instrument skeleton** `est:25min`
  Create plugin/ directory with:
  1. plugin/CMakeLists.txt — smtg_add_vst3plugin(poly_plugin ...) linking poly_engine
  2. plugin/source/factory.cpp — BEGIN_FACTORY_DEF / DEF_CLASS2 for processor + controller
  3. plugin/source/plugids.h — processor/controller class IDs and plugin constants
  4. plugin/source/processor.h/.cpp — PolyProcessor inheriting AudioEffect with empty process()
  5. plugin/source/controller.h/.cpp — PolyController inheriting EditController
  Apply jk_target_warnings. Declare a silent audio output bus (Cubase requirement for instruments).
  - Files: `plugin/CMakeLists.txt`, `plugin/source/factory.cpp`, `plugin/source/plugids.h`, `plugin/source/processor.h`, `plugin/source/processor.cpp`, `plugin/source/controller.h`, `plugin/source/controller.cpp`
  - Verify: cmake --build build produces a .vst3 bundle; find build -name '*.vst3' returns a result

- [x] **T04: Build verification and warning cleanup** `est:10min`
  1. Run full cmake configure + build from clean state
  2. Verify zero warnings with -Wall -Wextra
  3. Verify .vst3 bundle exists in build output
  4. Verify poly_engine compiles independently (no VST3 SDK leakage)
  5. Update .gitignore if needed for SDK artifacts
  - Files: `.gitignore`
  - Verify: cmake --build build --clean-first 2>&1 | grep -c 'warning:' returns 0; find build -name '*.vst3' returns a path

## Files Likely Touched

- CMakeLists.txt
- engine/CMakeLists.txt
- engine/include/poly/engine.h
- engine/include/poly/types.h
- engine/src/engine.cpp
- plugin/CMakeLists.txt
- plugin/source/factory.cpp
- plugin/source/plugids.h
- plugin/source/processor.h
- plugin/source/processor.cpp
- plugin/source/controller.h
- plugin/source/controller.cpp
- .gitignore
