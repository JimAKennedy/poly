# S05: Cross-rhythm Visualization

**Goal:** Add a cross-rhythm visualization view showing how all active lanes' cycles align and diverge over time. Display polyrhythmic convergence points where multiple lanes' downbeats coincide.
**Demo:** UI view showing how all lanes align and cross over time — polyrhythmic convergence points visible

## Must-Haves

- VSTGUI custom view renders concentric arcs or linear timeline showing each lane's cycle boundaries. Convergence points (where 2+ lanes' cycles align) are highlighted. View updates when lane parameters change. Works with both equal and additive cells.

## Proof Level

- This slice proves: Interaction smoke tests for the new view. Visual verification in Cubase.

## Integration Closure

View reads lane configs from controller parameters. Works alongside existing phase alignment view. Registered in poly.uidesc.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Design cross-rhythm view layout and rendering** `est:30min`
  Design a linear timeline view that shows N bars worth of lane cycle boundaries. Each lane gets a horizontal row with markers at cell/step boundaries. Convergence points (where 2+ lanes share a boundary) get highlighted markers. Compute convergence from LCM of cycle lengths.
  - Files: `plugin/source/ui/`
  - Verify: Design review

- [x] **T02: Implement cross-rhythm view (VSTGUI CView)** `est:1.5h`
  Create cross_rhythm_view.h/cpp as a CView subclass. Draws horizontal lanes with step markers at computed PPQ positions. Highlights convergence points. Reads lane configs via tag-based parameter access. Handles both equal and additive cell widths.
  - Files: `plugin/source/ui/cross_rhythm_view.h`, `plugin/source/ui/cross_rhythm_view.cpp`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T03: Register view in uidesc and controller** `est:30min`
  Add cross-rhythm view to poly.uidesc layout. Register the view factory in controller.cpp createView(). Wire parameter tags for lane configs.
  - Files: `plugin/resource/poly.uidesc`, `plugin/source/controller.cpp`, `plugin/CMakeLists.txt`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T04: Interaction smoke tests for cross-rhythm view** `est:30min`
  Add interaction smoke test verifying the cross-rhythm view renders without crash when loaded and when lane parameters change.
  - Files: `tests/ui/interaction/interaction_smoke_tests.cpp`
  - Verify: cmake --build build && ctest --test-dir build

## Files Likely Touched

- plugin/source/ui/
- plugin/source/ui/cross_rhythm_view.h
- plugin/source/ui/cross_rhythm_view.cpp
- plugin/resource/poly.uidesc
- plugin/source/controller.cpp
- plugin/CMakeLists.txt
- tests/ui/interaction/interaction_smoke_tests.cpp
