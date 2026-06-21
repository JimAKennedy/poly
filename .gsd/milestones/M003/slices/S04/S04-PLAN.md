# S04: Cubase Automation Polish

**Goal:** Organize VST3 parameters with proper units, display formatting, and unit hierarchy for clean Cubase automation lanes
**Demo:** Cubase automation lanes show well-organized, clearly named parameters; high-value controls are easy to find and automate

## Must-Haves

- All parameters display with correct units in Cubase (%, ms, beats). Parameters grouped into named VST3 Units (per-lane, macros, global). Automation lane names are concise and scannable.

## Proof Level

- This slice proves: Build succeeds; plugin_tests pass; manual Cubase verification of automation lane display

## Integration Closure

Controller-only changes. No engine or state format changes.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: VST3 Unit hierarchy** `est:1.5h`
  Implement IUnitInfo in PolyController. Create unit hierarchy: root -> Lane 1..8 (each with their params), Macros unit, Global unit. Register units in initialize(). Assign each parameter unitId to the correct unit.
  - Files: `plugin/source/controller.h`, `plugin/source/controller.cpp`, `plugin/source/plugids.h`
  - Verify: cmake --build build && ctest --test-dir build -R plugin

- [x] **T02: Parameter display formatting** `est:1h`
  Add proper units strings to all parameters: Probability -> '%', Humanize -> 'ms', Note Duration -> 'beats', Velocity -> '0-127', Swing -> '%'. Set appropriate stepCount for discrete params. Add getParamStringByValue/getParamValueByString overrides for formatted display where needed.
  - Files: `plugin/source/controller.cpp`
  - Verify: cmake --build build && ctest --test-dir build -R plugin

- [x] **T03: Parameter naming cleanup** `est:45m`
  Rename automation lane labels for scannability. Macros get 'Macro | Complexity' format. Ensure names are unique and under the VST3 128-char title limit. Verify no ParamID collisions with any new params from S02/S03.
  - Files: `plugin/source/controller.cpp`
  - Verify: cmake --build build && ctest --test-dir build -R plugin

## Files Likely Touched

- plugin/source/controller.h
- plugin/source/controller.cpp
- plugin/source/plugids.h
