---
estimated_steps: 6
estimated_files: 4
skills_used: []
---

# T02: poly_engine static library scaffold

Create engine/ directory with:
1. engine/CMakeLists.txt — add_library(poly_engine STATIC) with public include path
2. engine/include/poly/engine.h — Engine class declaration with renderRange stub
3. engine/include/poly/types.h — NoteEvent, TransportContext, basic type declarations
4. engine/src/engine.cpp — empty renderRange implementation
Must NOT link or include any VST3 SDK headers. Apply jk_target_warnings.

## Inputs

- `IMPLEMENTATION_PLAN.md section 3 (domain model)`

## Expected Output

- `engine/CMakeLists.txt`
- `engine/include/poly/engine.h`
- `engine/include/poly/types.h`
- `engine/src/engine.cpp`

## Verification

poly_engine target compiles as standalone static lib; grep confirms no VST3 includes
