---
class: gated
---

# M002/S03 — Fills resolve onto the phrase boundary, and a tihai lands

**Slice:** `M002/S03` in `docs/plans/guide-parity/ledger.md`

**Design:** `M002-S01-design.md` — this slice is bounded against the mechanism
that design establishes.

## Task status

- [x] 1. A phrase-proximity weight for fills
- [x] 2. A tihai lands on the target
- [ ] 3. `06-indian-classical` stops asking for hand arithmetic, and close-out

## Definition of Done

Copied verbatim from the slice.

- [ ] Fill probability rises toward the end of a phrase cycle, with a shape parameter controlling how sharply, asserted as a distribution across the cycle
- [ ] For an ungated lane the boundary used is the composite convergence point, not silence
- [ ] A tihai of a given phrase length lands its final onset exactly on the target, asserted arithmetically rather than by ear
- [ ] With the weighting neutral, all 45 factory presets render byte-identically, proved by a golden test
- [ ] `06-indian-classical` no longer asks the reader to do the tihai arithmetic by hand, and a `scope-framing` claim locks it

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

## Task 1 — A phrase-proximity weight for fills

Modifies `engine/include/poly/types.h` and `engine/src/engine.cpp`; extends
`tests/step_weight_tests.cpp`.

1. Write the failing tests first:
   - with `fillPhraseShape` at zero, `fill` is exactly `1.0f` everywhere
   - with it positive, `fill` rises toward the end of the phrase-gate cycle,
     asserted across several positions in the cycle rather than at two points —
     a weight that spikes only at the final step would satisfy two samples
   - the shape parameter controls how sharply: a higher value concentrates the
     rise later, asserted by comparing two shapes at a mid-cycle position
   - **for an ungated lane** the boundary is the composite convergence point,
     not silence. The row says so, and a lane with `phraseLength == 0` must
     still get a meaningful weight rather than a division by zero.
2. Add `float fillPhraseShape = 0.0f;` to `LaneConfig` and extend
   `computeStepWeights`. The phrase position is already computed in
   `prepareLaneContext` as `phraseCyclePpq`/`phraseOffPpq`; reuse it rather than
   recomputing, and confirm that by reading the code rather than assuming.
3. Scale the fill threshold by `w.fill` at the `fillRoll` site.
4. Add the byte-identity golden with `fillPhraseShape` unset.
5. Run `format`, `unit`, `engine-isolation`, `rt-safety`. Commit.

## Task 2 — A tihai lands on the target

Modifies `engine/include/poly/types.h` (or a new helper header) and
`tests/step_weight_tests.cpp`.

1. Write the failing tests first, against a pure `tihaiGap(phraseLength,
   remaining)` helper: given a phrase length P and a distance to the target,
   solve `3P + 2g = remaining` for the gap, and assert the final onset lands
   exactly on the target for several (P, remaining) pairs.
2. Assert the **unsolvable** cases are reported rather than silently rounded: a
   remaining distance that yields a negative gap has no tihai, and returning one
   anyway would place the third repetition past the target.
3. Implement it. This is arithmetic, not an engine change: the row allows the
   helper to be engine-side or UI-side, and engine-side keeps it testable
   without a browser.
4. Run `format`, `unit`, `engine-isolation`. Commit.

## Task 3 — `06-indian-classical` stops asking for hand arithmetic, and close-out

1. Read the page's "Tihai: Landing on Sam" section and quote it in the claim.
2. Write the claim, watch it fail, rewrite, prove by removal.
3. Run the whole validation set, append the evidence, tick the boxes, set `GP05`
   and the slice `done`, run `jk-standards ledger`, commit with
   `Slice: M002/S03` and `Rows: GP05`.
