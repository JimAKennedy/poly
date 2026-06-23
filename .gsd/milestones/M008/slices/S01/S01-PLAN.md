# S01: Additive and Aksak Cycle Mode

**Goal:** Support additive/aksak cycle definitions where steps have unequal durations (e.g. [2+2+3] for 7/8 aksak). Euclidean distribution operates over the variable-width cells. Engine renderRange() computes per-step PPQ from cumulative cell widths. Custom Cell Editor view for editing cell sequences, with lane grid indicator for additive mode.
**Demo:** Define a lane cycle as [2,2,3] for 7/8 aksak — Euclidean distribution operates over the unequal cells

## Must-Haves

- A lane with cellSizes=[2,2,3] at subdivision=8 produces hits at unequal intervals matching 7/8 aksak timing. Euclidean distribution works correctly over variable-width cells. Golden tests pass with block-size independence. State version bumped with backward-compatible defaults. Cell Editor custom view allows visual editing of cell sequences. Lane grid shows additive indicator.

## Proof Level

- This slice proves: Unit tests for cell PPQ computation, Euclidean over additive cells, golden tests for aksak patterns with block-size independence. UI builds and renders.

## Integration Closure

Additive cells compose with all M007 features (phrase gating, mutation, drift, kotekan). Existing equal-cell patterns produce identical output (regression). Cell Editor view integrates with existing lane edit panel.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Extend Cycle and LaneConfig with additive cell array** `est:30min`
  Add cellSizes array and cellCount to LaneConfig. When cellCount > 0, the cycle uses variable-width cells instead of equal steps. Add helper function to compute cumulative PPQ offsets and total cycle length from cell sizes.
  - Files: `engine/include/poly/types.h`
  - Verify: cmake --build build && ctest --test-dir build (existing tests still pass)

- [x] **T02: Engine renderRange additive step timing** `est:1h`
  Modify renderRange() to use cumulative cell PPQ offsets when cellCount > 0. For each absolute step, compute PPQ as cycleIndex * cyclePpqLength + cumPpq[localStep] instead of absStep * sPpq. Step duration for note length also varies per cell. Equal-cell path (cellCount=0) unchanged.
  - Files: `engine/src/engine.cpp`
  - Verify: cmake --build build && ctest --test-dir build (existing golden tests pass unchanged)

- [x] **T03: State serialization for additive cells (version 10)** `est:30min`
  Bump kStateVersion to 10. Serialize cellCount and cellSizes array. Version 9 load path defaults cellCount=0 (equal cells, backward compatible).
  - Files: `plugin/source/processor.cpp`, `plugin/source/plugids.h`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T04: Unit and golden tests for additive/aksak patterns** `est:45min`
  Add tests: (1) 7/8 aksak [2+2+3] produces correct PPQ positions, (2) 9/8 [2+2+2+3] works, (3) Euclidean E(2,3) over [2+2+3] distributes correctly, (4) block-size independence for aksak, (5) additive + phrase gating composition, (6) additive + drift composition. Add golden test for aksak pattern.
  - Files: `tests/euclidean_tests.cpp`, `tests/golden_tests.cpp`
  - Verify: cmake --build build && ctest --test-dir build (all new + existing tests pass)

- [x] **T05: Expose cellCount parameter to controller** `est:20min`
  Add VST3 parameter for cellCount only (per lane) to controller.cpp. cellSizes array is NOT exposed as individual VST3 params — it is state-serialized only, edited through the Cell Editor view (T06). Wire cellCount so Cubase generic editor can enable/disable additive mode (0 = equal cells).
  - Files: `plugin/source/controller.cpp`, `plugin/source/plugids.h`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T06: Cell Editor View and lane grid additive indicator** `est:2h`
  Create a VSTGUI custom view (CellEditorView) for editing additive cell sequences per lane. Shows cell widths as proportional rectangles (e.g. [2|2|3] for 7/8). Click to select a cell, drag to resize, +/- buttons to add/remove cells. Updates cellSizes and cellCount on LaneConfig via state serialization (not VST3 params). Also add an additive mode indicator to LaneGridView — show the cell pattern shorthand (e.g. '2+2+3') when cellCount > 0. View reads current lane state from processor via bridge.
  - Files: `plugin/source/ui/cell_editor_view.cpp`, `plugin/source/ui/cell_editor_view.h`, `plugin/source/ui/lane_grid_view.cpp`, `plugin/source/ui/lane_grid_view.h`, `plugin/resource/poly.uidesc`
  - Verify: cmake --build build && ctest --test-dir build (build passes, UI tests pass)

## Files Likely Touched

- engine/include/poly/types.h
- engine/src/engine.cpp
- plugin/source/processor.cpp
- plugin/source/plugids.h
- tests/euclidean_tests.cpp
- tests/golden_tests.cpp
- plugin/source/controller.cpp
- plugin/source/ui/cell_editor_view.cpp
- plugin/source/ui/cell_editor_view.h
- plugin/source/ui/lane_grid_view.cpp
- plugin/source/ui/lane_grid_view.h
- plugin/resource/poly.uidesc
