# S03: Phase Drift

**Goal:** Add a per-lane drift rate that gradually rotates the pattern over time, derived from absolute PPQ position, creating Reich-style phasing between lanes
**Demo:** Two lanes with identical patterns gradually phase against each other over bars, creating Reich-style emergent rhythms

## Must-Haves

- Drift rate 0 = no change; drift rate > 0 creates gradual rotation; transport jump snaps to correct drift state; golden tests verify drift at specific PPQ positions

## Proof Level

- This slice proves: Golden tests with drift across transport jump boundary

## Integration Closure

Drift computed as additional rotation from absolute PPQ position; combines with existing rotation parameter; no accumulation across process blocks

## Verification

- Phase alignment view shows drift trajectories

## Tasks

- [x] **T01: Add driftRate to LaneConfig** `est:15min`
  Add driftRate (float, steps per bar, default 0.0) to LaneConfig. At 1.0, the pattern rotates by 1 step every bar. Fractional values produce gradual sub-step drift. Negative values drift backward.
  - Files: `engine/include/poly/types.h`
  - Verify: Build compiles; existing tests pass unchanged

- [x] **T02: Implement drift in renderRange** `est:1h`
  In renderRange(), after computing cycleStep, apply drift:
  - Compute bar position: barPos = ppq / 4.0
  - driftSteps = floor(barPos * driftRate) (integer steps of drift)
  - effectiveRotation = (cfg.rotation + driftSteps) % stepsInCycle
  - Re-generate or re-index Euclidean pattern with effectiveRotation
  Key: driftSteps derived from absolute PPQ, not accumulated. Transport jump to any position gets the correct drift for that bar.
  - Files: `engine/src/engine.cpp`
  - Verify: Build + existing tests pass; drift rate 0 produces identical output

- [x] **T03: State serialization for drift params** `est:20min`
  Update processor.cpp to serialize driftRate per lane.
  - Files: `plugin/source/processor.cpp`
  - Verify: Build compiles; RT safety check passes

- [x] **T04: Golden tests for drift including transport jump** `est:1h`
  Add golden test scenarios:
  1. driftRate=0 produces identical output to baseline
  2. driftRate=1.0 rotates pattern by 1 step per bar
  3. Two lanes with identical patterns but different driftRates produce phasing
  4. Transport jump to bar 8 with driftRate=0.5 produces correct 4-step rotation
  5. Loop restart correctly resets drift to bar-start drift state
  - Files: `tests/golden_tests.cpp`, `tests/golden/`
  - Verify: ctest --test-dir build -R golden passes

- [x] **T05: Phase alignment view shows drift trajectories** `est:45min`
  Update phase_alignment_view to show drift direction and rate as animated rotation indicators or trajectory lines per lane.
  - Files: `plugin/source/ui/phase_alignment_view.cpp`, `plugin/source/ui/phase_alignment_view.h`
  - Verify: Build compiles; visual smoke test passes

## Files Likely Touched

- engine/include/poly/types.h
- engine/src/engine.cpp
- plugin/source/processor.cpp
- tests/golden_tests.cpp
- tests/golden/
- plugin/source/ui/phase_alignment_view.cpp
- plugin/source/ui/phase_alignment_view.h
