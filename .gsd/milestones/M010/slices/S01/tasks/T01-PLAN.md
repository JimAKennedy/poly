---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T01: Add new param IDs and expand parameter block

Add kSteps, kSubdivision, kHits, kRotation, kMidiNote as offsets 16-20 in plugids.h. Expand kParamsPerLane to 24. Wire applyParameter in processor.cpp. Register params in controller.cpp. Add setComponentState sync.

## Inputs

- `Current plugids.h param layout`
- `Current processor.cpp applyParameter switch`
- `Current controller.cpp param registration`

## Expected Output

- `Updated plugids.h with 5 new param offsets`
- `processor.cpp applies new params to LaneConfig`
- `controller.cpp registers and syncs new params`

## Verification

cmake --build build && ctest --test-dir build
