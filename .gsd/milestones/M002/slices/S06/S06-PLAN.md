# S06: Minimal VSTGUI Editor

**Goal:** Build minimal VSTGUI editor with lane overview grid, macro knob row, and per-lane velocity view
**Demo:** UI shows lane activity in real time, macro knobs control all macros, velocity view updates during playback

## Must-Haves

- Editor opens in Cubase without crash; lane grid shows which lanes are active; macro knobs send parameter changes to processor; velocity view updates during playback

## Proof Level

- This slice proves: Manual Cubase verification; no automated UI tests (first VSTGUI project)

## Integration Closure

Editor communicates via EditController parameter changes; no direct engine access

## Verification

- Visual feedback of lane activity and macro positions

## Tasks

- [x] **T01: VSTGUI spike and editor scaffold** `est:60min`
  Set up VSTGUI editor infrastructure: create .uidesc file, wire createView in controller, add editor open/close lifecycle. Start with a minimal window that opens and closes cleanly in the host. Evaluate VSTGUI custom view approach vs built-in controls for the lane grid.
  - Files: `plugin/source/controller.cpp`, `plugin/source/controller.h`, `plugin/source/ui/editor.uidesc`, `plugin/CMakeLists.txt`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [x] **T02: Implement lane overview grid view** `est:90min`
  Create a custom VSTGUI CView subclass that draws an 8-lane grid showing: lane active/inactive state, lane role/name, current Euclidean pattern visualization (filled/empty steps), and hit indicator during playback. Use CDrawContext for custom rendering. Register as a custom view in the controller.
  - Files: `plugin/source/ui/lane_grid_view.h`, `plugin/source/ui/lane_grid_view.cpp`, `plugin/source/ui/editor.uidesc`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [x] **T03: Implement macro knob row** `est:45min`
  Add a row of CKnobBase controls for the 6 macro parameters (complexity, density, syncopation, swing, tension, humanize). Wire each to its corresponding ParamID from the controller. Use VSTGUI parameter binding so knob position reflects automation state.
  - Files: `plugin/source/ui/editor.uidesc`, `plugin/source/controller.cpp`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

- [x] **T04: Implement velocity view** `est:60min`
  Create a custom view that displays per-lane velocity values during playback. Show vertical bars for each lane's most recent note velocity, updating in real time. Use message passing from processor to controller (non-RT thread) to communicate current velocity state. Ensure no RT-unsafe operations.
  - Files: `plugin/source/ui/velocity_view.h`, `plugin/source/ui/velocity_view.cpp`, `plugin/source/processor.cpp`, `plugin/source/controller.cpp`
  - Verify: cd build && cmake --build . && ctest --output-on-failure

## Files Likely Touched

- plugin/source/controller.cpp
- plugin/source/controller.h
- plugin/source/ui/editor.uidesc
- plugin/CMakeLists.txt
- plugin/source/ui/lane_grid_view.h
- plugin/source/ui/lane_grid_view.cpp
- plugin/source/ui/velocity_view.h
- plugin/source/ui/velocity_view.cpp
- plugin/source/processor.cpp
