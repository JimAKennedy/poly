---
class: gated
---

# M002/S02 — Ghosts cluster where funk puts them

**Slice:** `M002/S02` in `docs/plans/guide-parity/ledger.md`

**Design:** `M002-S01-design.md` — this slice is bounded against the mechanism
that design establishes.

## Task status

- [x] 1. A metric-position weight for ghosts
- [ ] 2. The funk chapter stops prescribing a ghost lane, and close-out

## Definition of Done

Copied verbatim from the slice.

- [ ] Ghost-add probability is higher on weak subdivisions preceding an accent than on those following one, asserted as a distribution over many seeds rather than a single roll
- [ ] The weighting scales with the Complexity macro, so low Complexity keeps grooves clean
- [ ] With the weighting neutral, all 45 factory presets render byte-identically, proved by a golden test
- [ ] `11-funk`'s dedicated ghost lane is no longer the recipe the page prescribes, and a `scope-framing` claim locks the change

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

## Task 1 — A metric-position weight for ghosts

Modifies `engine/include/poly/types.h` and `engine/src/engine.cpp`; extends
`tests/step_weight_tests.cpp`.

1. Write the failing tests first:
   - with `ghostGrammar` at zero, `ghost` is exactly `1.0f` on every step
   - with it positive, `ghost` is higher on a weak subdivision **preceding** an
     accented pattern step than on one **following** it. Both halves are needed:
     "fills toward the accent" is a gradient, and a weight that merely favoured
     all weak steps would satisfy a one-sided test.
   - the weight scales with the Complexity macro, so low Complexity keeps
     grooves clean — the row states this and it is checkable
2. Add `float ghostGrammar = 0.0f;` to `LaneConfig` and extend
   `computeStepWeights` to populate `ghost`. The accent positions come from the
   lane's own pattern, which `prepareLaneContext` already has.
3. Scale the ghost branch's threshold by `w.ghost` at the `typeRoll` site.
   Note in a comment that `typeRoll`'s thresholds are cumulative — `drop` then
   `ghost` then `add` — so scaling the ghost band moves its upper edge and the
   add band absorbs the difference. That is the intended behaviour and it is not
   obvious from the code.
4. Add the byte-identity golden with `ghostGrammar` unset.
5. Run `format`, `unit`, `engine-isolation`, `rt-safety`. Commit.

## Task 2 — The funk chapter stops prescribing a ghost lane, and close-out

Modifies the funk chapter and its theory page, `site/tests/`, the evidence, the
ledger, and this plan.

1. Read `11-funk-soul.mdx` and its theory page and quote what they say. The row
   states Chapter 11 "works around this with a dedicated high-hit-count ghost
   lane, which costs a lane and cannot respond to where the accents are". Verify
   that is what the page prescribes before writing a claim against it.
2. Write the claim, watch it fail, rewrite, watch it pass, prove by removal.
3. Run the whole validation set, append the evidence naming the discriminating
   case, tick the boxes, set `GP04` and the slice `done`, run
   `jk-standards ledger`, commit with `Slice: M002/S02` and `Rows: GP04`.
