---
estimated_steps: 7
estimated_files: 7
skills_used: []
---

# T03: poly_plugin VST3 instrument skeleton

Create plugin/ directory with:
1. plugin/CMakeLists.txt — smtg_add_vst3plugin(poly_plugin ...) linking poly_engine
2. plugin/source/factory.cpp — BEGIN_FACTORY_DEF / DEF_CLASS2 for processor + controller
3. plugin/source/plugids.h — processor/controller class IDs and plugin constants
4. plugin/source/processor.h/.cpp — PolyProcessor inheriting AudioEffect with empty process()
5. plugin/source/controller.h/.cpp — PolyController inheriting EditController
Apply jk_target_warnings. Declare a silent audio output bus (Cubase requirement for instruments).

## Inputs

- `engine/include/poly/engine.h`
- `cmake/jk_warnings.cmake`

## Expected Output

- `plugin/CMakeLists.txt`
- `plugin/source/factory.cpp`
- `plugin/source/plugids.h`
- `plugin/source/processor.h`
- `plugin/source/processor.cpp`
- `plugin/source/controller.h`
- `plugin/source/controller.cpp`

## Verification

cmake --build build produces a .vst3 bundle; find build -name '*.vst3' returns a result
