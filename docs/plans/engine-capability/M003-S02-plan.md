---
class: gated
---

# M003/S02 — Cell-aware aksak swing

**Slice:** `M003/S02` in `docs/plans/engine-capability/ledger.md`
**Depends:** `M003/S01` — see `M003-decisions.md` for why this dependency was
added after the ledger denied it.

## Task status

- [x] 1. A lane's steps can be grouped into cells
- [x] 2. Swing displaces within a cell, not across the bar
- [x] 3. The Balkan preset's long beat, and Rule 8 catches up

## Definition of Done

Copied verbatim from the slice.

- [ ] Swing on an additive meter applies per cell rather than to alternate notes
      across the bar
- [ ] A Balkan preset's long beat carries the feel the guide describes
- [ ] `renderRange()` gains no allocation, lock or blocking call

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `unit` | `cmake --build build --config Release --parallel && ctest --test-dir build --build-config Release --output-on-failure` |
| `engine-isolation` | `cmake -S . -B build-engine -DCMAKE_BUILD_TYPE=Release -DPOLY_ENGINE_ONLY=ON && cmake --build build-engine --parallel && ctest --test-dir build-engine --output-on-failure` |
| `rt-safety` | `bash scripts/check-realtime-safety.sh` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |

## Task 1 — A lane's steps can be grouped into cells

Creates `tests/aksak_swing_tests.cpp`; modifies `engine/include/poly/types.h`
and `tests/CMakeLists.txt`.

The gap S01's design names: `cellCount > 0` today collapses a lane to one step
per cell, so there is nothing inside a cell to swing. This task lets a lane keep
one step per *unit* while knowing its cell boundaries.

1. Write the failing test first. Against a helper that maps a step index to its
   cell index and its position within that cell:
   - a lane with `cycle = {7, 8}`, `swingCellCount = 3`,
     `swingCellSizes = {2, 2, 3}` maps steps 0–6 to cells 0,0,1,1,2,2,2 and to
     within-cell positions 0,1,0,1,0,1,2
   - `swingCellCount == 0` leaves every step in its own cell at position 0, so
     nothing downstream changes
   - cell sizes that do not sum to the step count are rejected — the helper
     reports no grouping rather than reading past the end
2. Add `int swingCellCount = 0` and `std::array<int, kMaxSteps> swingCellSizes{}`
   to `LaneConfig`, with a comment stating that this is a **grouping over
   existing steps**, distinct from `cellCount`/`cellSizes`, which replace the
   steps with one per cell. Name M003/S02 and EC09.
3. Implement the helper in `engine/include/poly/types.h` beside
   `computeAdditiveCells`, returning cell index and within-cell position for a
   step. Pure, allocation-free, header-inline like its neighbour.
4. Watch the cases pass. Run `unit` and `engine-isolation`. Commit.

## Task 2 — Swing displaces within a cell, not across the bar

Modifies `engine/src/engine.cpp` and `tests/aksak_swing_tests.cpp`.

1. Write the failing cases first, rendering through the engine:
   - with `swingCellSizes = {2,2,3}` and `swingAmount = 0.2f`, the displaced
     steps are the odd *within-cell* positions — steps 1, 3, 5 and 6 of the
     seven — not the odd absolute steps 1, 3, 5
   - the three-unit cell's internal inter-onset intervals differ from the
     two-unit cells', which is the slice's second definition-of-done clause
     stated as a measurement
   - `swingCellCount == 0` reproduces today's output byte-identically for a
     swung lane, so no existing patch moves
2. Watch them fail against the current `(cycleStep % 2) == 1` predicate in
   `applyTimingShifts` — `engine/src/engine.cpp:329`.
3. Change the predicate to use the within-cell position when a grouping is
   present, falling back to `cycleStep % 2` when it is not. `applyTimingShifts`
   already receives `cycleStep`; pass the `LaneConfig` grouping through the same
   call rather than recomputing it per step.
4. Confirm `maxTimingShift` still bounds the displacement — swing is scaled by
   `stepDurPpq`, which task 1 does not change, so the existing bound holds. State
   that in the evidence rather than assuming it: assert a note near a block
   boundary is still emitted.
5. Run `unit`, `engine-isolation`, `rt-safety`. Commit.

## Task 3 — The Balkan preset's long beat, and Rule 8 catches up

**Repaired mid-slice.** This task previously said to give the zurna "the swing
the guide describes". `theory-balkan` Rule 6 is "No swing", and its stated
reason is the mechanism task 2 built. The feel the guide describes is Rule 8's
long beat, which is the second half of issue #157. Tasks 1 and 2 are unaffected
and their boxes stand.

Modifies `engine/src/presets.cpp`, `engine/include/poly/types.h`,
`site/src/content/docs/theory-balkan.mdx`, `site/tests/scope-framing.test.mjs`,
`site/tests/presets-json-schema.test.mjs`, `docs/engine-spec.md`, the evidence
file, the ledger, and this plan.

1. Compose cells and profile: when a lane declares the same number of cells as
   profile entries, normalise the profile to the cells' total rather than to
   `profileCount`, so the bar keeps its notated length. Mismatched counts leave
   the profile governing alone, which is M003/S01's precedence case unchanged.
   Two cases pin both branches.
2. Give the Balkan davul and rim a `{2.0, 2.0, 2.85}` profile over their
   `{2,2,3}` cells. Leave `swingAmount` at 0 on every Balkan lane: Rule 6.
3. Replace Rule 8's "until Poly exposes a long-beat ratio control" disclaimer
   with what the engine now does and what the preset plays, keeping the rule's
   cited claim intact.
4. Fix the two guards this trips: `presets-json-schema.test.mjs` still expects
   `schemaVersion === 5`, and `scope-framing`'s `S04-F32` locks a phrase from
   the deleted disclaimer. Update the lock to the ratio the preset plays rather
   than loosening it — the claim F32 protects is unchanged.
5. Document the new fields in `docs/engine-spec.md`; `doc-drift` requires it for
   any change to `types.h`. The preset-taxonomy and testing-strategy rules are
   answered with `Docs-Not-Affected:` trailers, because no preset was added or
   recategorised and no new test binary was introduced.
6. Run the whole validation set, append the evidence, tick the boxes, set `EC09`
   and the slice `done`, run `jk-standards ledger`, commit with
   `Slice: M003/S02` and `Rows: EC09`.
