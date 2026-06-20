---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T04: Implement versioned state serialization

Implement getState() writing kStateVersion (int32) then full GrooveState fields using IBStream. Implement setState() reading version, branching on version number, and restoring GrooveState. Include all lane configs, macros, seed, and envelope assignments. Follow the portfolio convention of version-first serialization.

## Inputs

- `engine/include/poly/types.h`
- `plugin/source/processor.h`
- `plugin/source/processor.cpp`
- `plugin/source/controller.cpp`

## Expected Output

- `plugin/source/processor.cpp`
- `plugin/source/controller.cpp`

## Verification

cd build && cmake --build . && ctest --output-on-failure
