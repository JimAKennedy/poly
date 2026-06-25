# M001 Architecture Decision Review

## Summary

Six significant architecture decisions were made during M001. All were validated through implementation and testing. No decisions require revisiting for Phase 1.

---

## D1: Engine Isolation (poly_engine as pure C++ static library)

**Decision:** Separate the groove engine from the VST3 plugin as a standalone C++ static library with zero SDK dependencies.

**Rationale:** Enables off-host unit testing, golden test determinism verification, and clean separation of musical logic from host integration. The engine is testable without a DAW.

**Alternatives considered:**
- Engine embedded in the plugin — simpler build but untestable without a host
- Header-only engine — avoids the static lib but leaks implementation details

**Confidence:** High. Proven in S01–S03. The engine compiles and passes 18 tests without the VST3 SDK on the link path. The `tools/harness/` CLI also exercises the engine independently.

**Phase 1 impact:** None. The boundary is clean and will hold as features are added.

---

## D2: PPQ-Derived Timing (No Accumulation)

**Decision:** Derive all cycle phase, step position, and envelope phase from absolute PPQ position. Never accumulate state across process blocks.

**Rationale:** Accumulator-based sequencers drift on loop restarts, tempo changes, and position jumps. PPQ-derived positioning makes the engine a pure function of `(transport, patch)` — identical inputs always produce identical output.

**Alternatives considered:**
- Accumulator with reset-on-loop — handles loops but fails on arbitrary position jumps
- Sample-based timing — introduces sample-rate dependency and buffer-size sensitivity

**Confidence:** High. Eight golden tests prove byte-identical output under block-size variation, loop restart, position jump, and tempo change. This is the strongest architectural guarantee in the project.

**Phase 1 impact:** Envelope evaluation will use the same pattern — `phase = fmod(ppq / (periodBars * 4.0), 1.0)`. No architectural change needed.

---

## D3: Euclidean Rhythm Generation (Bresenham/Bjorklund)

**Decision:** Use the Bresenham/Bjorklund algorithm for distributing pulses across steps, with rotation for pattern offset.

**Rationale:** Euclidean rhythms produce maximally even distributions that correspond to well-known musical patterns (tresillo, cinquillo, etc.). The algorithm is O(n), deterministic, and produces musically useful results with just two parameters (hits, steps).

**Alternatives considered:**
- User-drawn step patterns — more flexible but no generative capability
- Probability-only generation — loses structural regularity
- Binary tree subdivision — produces different distributions, less musically validated

**Confidence:** High. 10 unit tests verify distribution correctness, rotation preservation, edge cases (0 pulses, more pulses than steps), and maximal evenness. The algorithm is well-studied in music theory and computer science literature.

**Phase 1 impact:** Euclidean generation remains the default. User-editable pattern overrides may be added but the Euclidean function stays as a generator.

---

## D4: Position-Seeded Deterministic RNG (splitmix64)

**Decision:** Use a position-seeded hash function (`deterministicRand(seed, laneId, absStep, channel)`) instead of a sequential PRNG.

**Rationale:** A sequential PRNG (like `std::mt19937`) accumulates state — its output for step N depends on every prior call. This makes it block-size-dependent and position-jump-fragile. Position-seeded hashing makes each step's random value a pure function of its coordinates.

**Alternatives considered:**
- `std::mt19937` with reset-on-cycle — fragile across block boundaries
- No randomness — deterministic but musically flat
- External random source — breaks reproducibility

**Confidence:** High. Block-size independence tests (0.05, 0.5, 2.0 PPQ blocks) produce byte-identical results. The channel dimension ensures probability and velocity use independent random streams.

**Phase 1 impact:** Additional channels will be needed (2 = emphasis, 3 = humanize timing, 4+ = envelope-related randomness). The hash function supports this natively via the channel parameter.

---

## D5: Fixed-Capacity Buffers (No Heap in renderRange)

**Decision:** Use `std::array` with compile-time capacities for all working buffers: `NoteEventBuffer` (256 events), `AccentMask` (64 steps), `LaneConfig[8]`, etc.

**Rationale:** Real-time safety requires no heap allocation in the audio-thread call path. Fixed-capacity buffers with overflow checks (`NoteEventBuffer::push` returns false on overflow) satisfy this constraint with zero runtime cost.

**Alternatives considered:**
- `std::vector` with pre-reserved capacity — technically possible but leaks the capacity guarantee to runtime
- Ring buffers — unnecessary for single-block output that doesn't persist
- Dynamic allocation with arena — over-engineered for current needs

**Confidence:** High. The 256-event buffer is large enough for 8 lanes × 64 steps per block (theoretical max 512, but probability filtering typically reduces to <50 events). A diagnostic surface for overflow events would be valuable in Phase 1.

**Phase 1 impact:** May need to increase `kMaxEventsPerBlock` if dense patches with many lanes overflow. Adding a diagnostic counter for dropped events is recommended.

---

## D6: ParamID Layout (Range-Based, 800-Stride Per Lane)

**Decision:** Organize ParamIDs in ranges — globals (0–99), macros (100–199), per-lane (1000+ with 800-stride) — rather than sequential assignment.

**Rationale:** Range-based layout allows future parameter additions within each category without renumbering. The 800 IDs per lane leave room for envelope parameters, future per-lane controls, and backward-compatible state deserialization.

**Alternatives considered:**
- Sequential ParamIDs (0, 1, 2, ...) — fragile under parameter additions
- Hash-based IDs — harder to debug, no range guarantees

**Confidence:** Medium-high. The layout is specified but not yet implemented in the controller. It needs validation when `PolyController::initialize()` registers parameters in Phase 1.

**Phase 1 impact:** Direct. The controller must implement this layout. Any changes to the layout should happen before the first preset is saved, since ParamIDs are part of the serialized state.
