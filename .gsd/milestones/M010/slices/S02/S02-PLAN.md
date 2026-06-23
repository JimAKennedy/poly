# S02: Lane Edit UI View

**Goal:** Create LaneEditView UI with 10 knobs for pattern and voice params, shrink VelocityView, adjust layout
**Demo:** Open plugin UI, see new Lane Edit section with 10 knobs for pattern and voice params per lane

## Must-Haves

- Complete the planned slice outcomes.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Create LaneEditView custom view** `est:45min`
  Create lane_edit_view.h/cpp following PhraseEditView pattern. 10 knobs in two groups: Pattern (Steps, Subdiv, Hits, Rotation, Note) and Voice (Vel, Ghost, Spread, Swing, Kotekan). Lane tabs sync with kSelectedLane. Handle both core and regular param IDs.
  - Files: `plugin/source/ui/lane_edit_view.h`, `plugin/source/ui/lane_edit_view.cpp`, `plugin/CMakeLists.txt`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T02: Wire view into controller and layout** `est:20min`
  Register LaneEditView in controller.cpp createCustomView. Add to poly.uidesc layout. Shrink VelocityView from 76px to 40px. Adjust window height and all y-origins below the new section.
  - Files: `plugin/source/controller.cpp`, `plugin/resource/poly.uidesc`
  - Verify: cmake --build build && ctest --test-dir build

## Files Likely Touched

- plugin/source/ui/lane_edit_view.h
- plugin/source/ui/lane_edit_view.cpp
- plugin/CMakeLists.txt
- plugin/source/controller.cpp
- plugin/resource/poly.uidesc
