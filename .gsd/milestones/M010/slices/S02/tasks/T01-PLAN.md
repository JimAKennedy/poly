---
estimated_steps: 1
estimated_files: 3
skills_used: []
---

# T01: Create LaneEditView custom view

Create lane_edit_view.h/cpp following PhraseEditView pattern. 10 knobs in two groups: Pattern (Steps, Subdiv, Hits, Rotation, Note) and Voice (Vel, Ghost, Spread, Swing, Kotekan). Lane tabs sync with kSelectedLane. Handle both core and regular param IDs.

## Inputs

- `PhraseEditView as reference pattern`
- `plugids.h param IDs`

## Expected Output

- `lane_edit_view.h/cpp with complete draw + mouse interaction`
- `Added to CMakeLists.txt`

## Verification

cmake --build build && ctest --test-dir build
