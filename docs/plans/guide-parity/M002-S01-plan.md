---
class: gated
---

# M002/S01 — A lane can weight by a reference lane's timeline

**Slice:** `M002/S01` in `docs/plans/guide-parity/ledger.md`

**Design:** `M002-S01-design.md` — approved 2026-09-18

## Task status

- [x] 1. The weight mechanism and the timeline source
- [x] 2. The rolls read the weights
- [ ] 3. `03-afro-cuban` stops prescribing manual clave maintenance
- [ ] 4. Evidence and close-out

## Definition of Done

Copied verbatim from the slice.

- [ ] A lane can name a reference lane, and a mutual reference is refused rather than followed
- [ ] Mutation-adds are biased toward or away from the reference lane's onsets by a signed per-lane strength, shown by the distribution of added steps changing with the sign
- [ ] Drops are less likely on high-weight steps than on low-weight ones
- [ ] With no reference lane named, all 45 factory presets render byte-identically, proved by a golden test
- [ ] `03-afro-cuban` stops describing clave alignment as something the reader maintains by hand, and a `scope-framing` claim locks it

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

## Task 1 — The weight mechanism and the timeline source

Modifies `engine/include/poly/types.h` and `engine/src/engine.cpp`; creates
`tests/step_weight_tests.cpp` and registers it in `tests/CMakeLists.txt`.

1. Write the failing tests first, against a `StepWeights` struct and a
   `computeStepWeights(cfg, sourcePattern, stepsInCycle, step)` helper that do
   not exist yet:
   - with no reference lane, every weight is exactly `1.0f` — this is the
     byte-identity guarantee stated as arithmetic rather than as a promise
   - with a positive `timelineStrength`, `add` exceeds `1.0` on a step the
     source strikes and falls below it on one the source does not
   - with a **negative** strength the comparison inverts. This is the case that
     proves the sign is read; without it "adds prefer aligned steps" could pass
     by reading the magnitude alone.
   - `drop` moves opposite to `add` on the same step: protecting what attraction
     favours is the row's stated behaviour
   - a self-reference, an out-of-range index, and a source pointing back all
     yield the neutral weights
2. Add the settings to `LaneConfig` beside `kotekanSourceLane`:
   `int timelineSourceLane = -1;` and `float timelineStrength = 0.0f;`, with a
   comment naming M002/S01 and GP03 and stating that the defaults are neutral.
3. Add `StepWeights` and `computeStepWeights` to `types.h` as inline pure
   functions — they run on the audio thread.
4. Resolve the reference lane's onsets once per lane per block in
   `prepareLaneContext`, following the kotekan resolution at `engine.cpp:99-110`
   including its mutual-reference guard. Store the pattern in `LaneContext`, not
   in `LaneConfig`: it is per-render and must not reach serialised state.
5. Run `format`, `unit`, `engine-isolation`. Commit; `GP03` stays open.

## Task 2 — The rolls read the weights

Modifies `engine/src/engine.cpp` and `tests/step_weight_tests.cpp`.

1. Write the failing render-level cases first, each measured as a **distribution
   over many seeds** rather than a single roll — one roll proves nothing about a
   probability:
   - with a positive strength, mutation-adds land on source-struck steps more
     often than on steps the source leaves empty
   - with a negative strength that relationship inverts
   - drops occur less often on source-struck steps than on empty ones
   - a lane with no reference lane produces exactly the pre-M002 distribution
2. Scale the thresholds at the roll sites: `mutRoll < cfg.mutationRate * w.add`
   for the add path, and the drop path by `w.drop`. **Scale the threshold, never
   the roll** — perturbing the random value would break the determinism the
   engine's whole contract rests on.
3. Mutation-prove each case with a probe forcing the weights neutral, and record
   which cases survive: the neutral-lane case will, because it is about the
   unweighted path, and saying so is the point.
4. Add the byte-identity golden over the 45 factory presets.
5. Run `format`, `unit`, `engine-isolation`, `rt-safety`. Commit.

## Task 3 — `03-afro-cuban` stops prescribing manual clave maintenance

Modifies `site/src/content/docs/03-afro-cuban.mdx` and
`site/tests/scope-framing.test.mjs`.

1. Read the page first and quote what it actually says in the claim's `rule`.
   The ledger's row says it teaches that parts ignoring the clave "sound wrong";
   the instruction to be removed is whatever the page tells the reader to *do*
   about it by hand. If the page turns out to prescribe nothing — as two of
   M001/S02's pages did — say so and make the claim assert the new capability is
   described, recording the finding rather than editing prose to satisfy a
   checklist.
2. Write the claim, watch it fail, rewrite the page, watch it pass.
3. Prove it by **removing** the correction.
4. Run `format`, `site-unit`, `doc-conformance`, `doc-discipline`. Commit.

## Task 4 — Evidence and close-out

1. Append `evidence/M002-S01.md`: token results with headline counts, every
   mutation outcome including which cases survived and why, and the golden.
2. Tick the task boxes and the definition-of-done boxes, set `GP03` and the
   slice `done`.
3. Run `jk-standards ledger`, then the whole validation set on the final tree.
   Commit with `Slice: M002/S01` and `Rows: GP03`.
