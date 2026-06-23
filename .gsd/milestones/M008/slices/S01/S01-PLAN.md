# S01: Additive and Aksak Cycle Mode

**Goal:** Support additive/aksak cycle definitions where steps have unequal durations (e.g. [2+2+3] for 7/8 aksak). Euclidean distribution operates over the variable-width cells. Engine renderRange() computes per-step PPQ from cumulative cell widths.
**Demo:** Define a lane cycle as [2,2,3] for 7/8 aksak — Euclidean distribution operates over the unequal cells

## Must-Haves

- A lane with cellSizes=[2,2,3] at subdivision=8 produces hits at unequal intervals matching 7/8 aksak timing. Euclidean distribution works correctly over variable-width cells. Golden tests pass with block-size independence. State version bumped with backward-compatible defaults.

## Proof Level

- This slice proves: Unit tests for cell PPQ computation, Euclidean over additive cells, and golden tests for aksak patterns with block-size independence.

## Integration Closure

Additive cells compose with all M007 features (phrase gating, mutation, drift, kotekan). Existing equal-cell patterns produce identical output (regression).

## Verification

- No new observability surfaces — engine is deterministic and testable offline.

## Tasks

- [ ] **T01: Extend Cycle and LaneConfig with additive cell array** `est:30min`
  Add cellSizes array and cellCount to LaneConfig. When cellCount > 0, the cycle uses variable-width cells instead of equal steps. Add helper function to compute cumulative PPQ offsets and total cycle length from cell sizes.
  - Files: `engine/include/poly/types.h`
  - Verify: cmake --build build && ctest --test-dir build (existing tests still pass — no engine changes yet)

- [ ] **T02: Engine renderRange additive step timing** `est:1h`
  Modify renderRange() to use cumulative cell PPQ offsets when cellCount > 0. For each absolute step, compute PPQ as cycleIndex * cyclePpqLength + cumPpq[localStep] instead of absStep * sPpq. Step duration for note length also varies per cell. Equal-cell path (cellCount=0) unchanged.
  - Files: `engine/src/engine.cpp`
  - Verify: cmake --build build && ctest --test-dir build (existing golden tests pass unchanged)

- [ ] **T03: State serialization for additive cells (version 10)** `est:30min`
  Bump kStateVersion to 10. Serialize cellCount and cellSizes array. Version 9 load path defaults cellCount=0 (equal cells, backward compatible).
  - Files: `plugin/source/processor.cpp`, `plugin/source/plugids.h`
  - Verify: cmake --build build && ctest --test-dir build

- [ ] **T04: Unit and golden tests for additive/aksak patterns** `est:45min`
  Add tests: (1) 7/8 aksak [2+2+3] produces correct PPQ positions, (2) 9/8 [2+2+2+3] works, (3) Euclidean E(2,3) over [2+2+3] distributes correctly, (4) block-size independence for aksak, (5) additive + phrase gating composition, (6) additive + drift composition. Add golden test for aksak pattern.
  - Files: `tests/euclidean_tests.cpp`, `tests/golden_tests.cpp`
  - Verify: cmake --build build && ctest --test-dir build (all new + existing tests pass)

- [ ] **T05: Expose additive cell parameters to controller** `est:30min`
  Add VST3 parameters for cellCount and cellSizes (per lane) to controller.cpp. Wire up so Cubase generic editor can set additive cells.
  - Files: `plugin/source/controller.cpp`, `plugin/source/plugids.h`
  - Verify: cmake --build build && ctest --test-dir build

## Files Likely Touched

- engine/include/poly/types.h
- engine/src/engine.cpp
- plugin/source/processor.cpp
- plugin/source/plugids.h
- tests/euclidean_tests.cpp
- tests/golden_tests.cpp
- plugin/source/controller.cpp
