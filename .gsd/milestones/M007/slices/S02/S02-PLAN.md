# S02: Pattern Mutation

**Goal:** Add a per-lane mutation rate that introduces controlled, deterministic variation to the Euclidean pattern each cycle — displaced hits, ghost notes, occasional substitutions
**Demo:** With mutation rate > 0, each cycle of a pattern has subtle variations — displaced hits, ghost notes, occasional substitutions

## Must-Haves

- Mutation rate 0 = no change (backward compatible); mutation rate > 0 produces per-cycle variations; mutations are deterministic given same seed + absolute position; golden tests pass

## Proof Level

- This slice proves: Golden tests with mutation rate > 0 across multiple cycles

## Integration Closure

Mutation applied after Euclidean generation, before envelope/constraint processing; RNG seeded from lane seed + cycle index for determinism

## Verification

- Lane grid view could optionally highlight mutated steps in a different shade

## Tasks

- [x] **T01: Add mutationRate to LaneConfig** `est:15min`
  Add mutationRate (float, 0.0-1.0, default 0.0) to LaneConfig. 0.0 = no mutation (backward compatible). Controls probability of per-step variation each cycle.
  - Files: `engine/include/poly/types.h`
  - Verify: Build compiles; existing tests pass unchanged

- [x] **T02: Implement deterministic pattern mutation in renderRange** `est:1.5h`
  After generating Euclidean pattern and computing cycleStep, apply mutation:
  - Compute absolute cycle index: cycleIndex = absStep / stepsInCycle
  - For each step, roll deterministicRand(seed, laneId, cycleIndex, stepInCycle + mutation_salt)
  - If roll < mutationRate, apply one of: toggle step (add/remove hit), displace step (+/- 1 position), reduce velocity to ghost level
  - Mutation type selected deterministically from hash of cycleIndex + stepInCycle
  - Must not mutate anchor steps (constraint system)
  Key invariant: same (seed, laneId, cycleIndex) always produces same mutations.
  - Files: `engine/src/engine.cpp`
  - Verify: Build + existing tests pass; mutation rate 0 produces identical output to baseline

- [x] **T03: State serialization for mutation params** `est:20min`
  Update processor.cpp to serialize mutationRate per lane. Bump state version if not already bumped by S01.
  - Files: `plugin/source/processor.cpp`
  - Verify: Build compiles; RT safety check passes

- [x] **T04: Golden tests for mutation** `est:45min`
  Add golden test scenarios:
  1. mutationRate=0 produces byte-identical output to baseline
  2. mutationRate=0.3 produces consistent variations across runs (determinism)
  3. Mutation respects anchor steps (no mutation on anchored positions)
  4. Transport jump + mutation produces correct output at new position
  - Files: `tests/golden_tests.cpp`, `tests/golden/`
  - Verify: ctest --test-dir build -R golden passes

## Files Likely Touched

- engine/include/poly/types.h
- engine/src/engine.cpp
- plugin/source/processor.cpp
- tests/golden_tests.cpp
- tests/golden/
