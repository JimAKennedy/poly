# S05: Kotekan Pairs

**Goal:** Add lane linking mode where a paired lane automatically generates the complement of its source lane Euclidean pattern, creating Balinese kotekan-style interlocking on two MIDI notes
**Demo:** Mark two lanes as a kotekan pair — one gets the Euclidean hits, the other fills the gaps, creating interlocking Balinese-style patterns on two MIDI notes

## Must-Haves

- Paired lane pattern is exact complement of source; different MIDI notes and velocity scaling per voice; complement updates when source pattern changes; golden tests cover pair scenarios

## Proof Level

- This slice proves: Golden tests with kotekan pair + Cubase UAT for musical quality

## Integration Closure

Complement computed from source lane Euclidean output; paired lane skips its own Euclidean generation; constraint system applies to both independently

## Verification

- Lane grid view shows linked pairs with visual indicator

## Tasks

- [ ] **T01: Add kotekan linking fields to LaneConfig** `est:15min`
  Add kotekanSourceLane (int, -1=none, 0-7=source lane index) to LaneConfig. When set, this lane's pattern is the complement of the source lane's Euclidean pattern (all gaps filled, all hits removed). The lane keeps its own midiNote, baseVelocity, and other params.
  - Files: `engine/include/poly/types.h`
  - Verify: Build compiles; existing tests pass unchanged

- [ ] **T02: Implement kotekan complement generation in renderRange** `est:1h`
  In renderRange(), when processing a lane with kotekanSourceLane >= 0:
  - Look up source lane config
  - Generate source lane Euclidean pattern
  - Invert: complement[i] = !source[i]
  - Use complement pattern instead of own Euclidean generation
  - All other processing (envelopes, constraints, velocity, timing) uses this lane's own config
  - Handle edge cases: source lane inactive, circular reference (A->B->A), source out of range
  - Files: `engine/src/engine.cpp`
  - Verify: Build + existing tests pass; kotekanSourceLane=-1 produces identical output

- [ ] **T03: State serialization and golden tests for kotekan** `est:45min`
  Serialize kotekanSourceLane. Add golden tests:
  1. Kotekan pair: lane A plays E(3,8), lane B is complement (plays the 5 gaps)
  2. Source + complement notes together fill all 8 positions exactly
  3. Kotekan with different velocities per voice
  4. kotekanSourceLane=-1 matches baseline
  - Files: `plugin/source/processor.cpp`, `tests/golden_tests.cpp`
  - Verify: ctest --test-dir build -R golden passes

- [ ] **T04: Lane grid view shows kotekan pairs** `est:45min`
  Update lane_grid_view to visually indicate kotekan-linked lanes — show a link icon or connecting line between paired lanes, and render complement steps in a distinct color.
  - Files: `plugin/source/ui/lane_grid_view.cpp`, `plugin/source/ui/lane_grid_view.h`
  - Verify: Build compiles; visual smoke test passes

## Files Likely Touched

- engine/include/poly/types.h
- engine/src/engine.cpp
- plugin/source/processor.cpp
- tests/golden_tests.cpp
- plugin/source/ui/lane_grid_view.cpp
- plugin/source/ui/lane_grid_view.h
