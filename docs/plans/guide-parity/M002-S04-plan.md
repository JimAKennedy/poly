---
class: gated
---

# M002/S04 — A response lane answers its call

**Slice:** `M002/S04` in `docs/plans/guide-parity/ledger.md`

**Design:** `M002-S01-design.md` — this slice is bounded against the mechanism
that design establishes.

## Task status

- [ ] 1. A lane's gate can answer another's
- [ ] 2. `15-compositional-grammar` stops giving the manual recipe, and close-out

## Definition of Done

Copied verbatim from the slice.

- [ ] A lane's phrase gate can be defined as open exactly when a named source lane's gate is closed, with an optional lead-in or overlap in beats
- [ ] Changing the source lane's phrase settings keeps the antiphony intact, which is the failure the manual recipe has
- [ ] A mutual reference between two response lanes is refused rather than followed
- [ ] With no response lane named, all 45 factory presets render byte-identically, proved by a golden test
- [ ] `15-compositional-grammar` no longer gives interleaving offsets as the recipe for antiphony, and a `scope-framing` claim locks it

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

## Standing instructions for this milestone

From `M002-decisions.md`, which took them from M001's findings:

- **Name the discriminating case in the evidence.** Most cases in a weighting
  suite survive their mutation probe — determinism, range and preset-identity
  hold for the unweighted behaviour too. Exactly which case carries the claim
  must be stated, or a green suite reads as several independent proofs when it
  is one.
- **Prove a `present`-only site claim by removing the correction**, never by
  re-inserting the old phrasing beside it. M001/S02's first probe did the latter
  and proved nothing.
- **Restore a probed file by explicit edit, not `git checkout`** — in M007/S03 a
  checkout discarded uncommitted work.
- **Rebuild before reading any mutation result.** In M003/S01 a stale build
  reported a failure against sources that were already correct.

## Task 1 — A lane's gate can answer another's

Modifies `engine/include/poly/types.h` and `engine/src/engine.cpp`; creates
`tests/response_gate_tests.cpp` and registers it in `tests/CMakeLists.txt`.

1. Write the failing tests first:
   - a response lane's gate is open exactly when the source lane's gate is
     closed, asserted across a full phrase cycle rather than at one instant
   - a lead-in of *n* beats opens the response early by exactly *n*; an overlap
     opens it late. Both signs, because one would pass by magnitude alone.
   - **changing the source lane's `phraseLength` keeps the antiphony intact.**
     This is the failure the manual recipe has and the reason the row exists, so
     it is the case that carries the claim.
   - a mutual reference between two response lanes is refused: neither lane
     gates off the other, and both fall back to their own phrase settings
   - with `responseSourceLane == -1` the gate is exactly the pre-M002 gate
2. Add `int responseSourceLane = -1;` and `float responseLeadIn = 0.0f;` to
   `LaneConfig`, reusing the resolution and guard `M002/S01` built rather than
   writing a second one — that reuse is the whole reason this slice is in this
   milestone.
3. The gate is evaluated at `engine.cpp:470-474` from `phraseCyclePpq` and
   `phraseOffPpq`. A response lane evaluates the *source's* gate from the
   source's settings and inverts it. Derive it from absolute PPQ as the existing
   gate does, so a locate reproduces it.
4. Add the byte-identity golden with no response lane set.
5. Run `format`, `unit`, `engine-isolation`, `rt-safety`. Commit.

## Task 2 — `15-compositional-grammar` stops giving the manual recipe, and close-out

1. The page says antiphony comes from "identical Length and Gap values but
   different Offsets" — verified at assessment. That is the recipe the slice
   replaces.
2. Write the claim, watch it fail, rewrite the passage, prove by removal.
3. Run the whole validation set, append the evidence naming the discriminating
   case, tick the boxes, set `GP06` and the slice `done`, run
   `jk-standards ledger`, commit with `Slice: M002/S04` and `Rows: GP06`.
