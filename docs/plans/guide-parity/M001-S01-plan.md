---
class: gated
---

# M001/S01 — Swing widens and tracks tempo

**Slice:** `M001/S01` in `docs/plans/guide-parity/ledger.md`
**Classification:** bounded — a change to swing, which `applyTimingShifts`
already applies. Design approved in chat 2026-09-18; see `M001-decisions.md`.

## Task status

- [x] 1. The mode field and the tempo curve
- [x] 2. Swing reads the curve, and the lookahead follows
- [x] 3. `12-jazz` stops prescribing a per-tempo value
- [x] 4. Evidence and close-out

## Definition of Done

Copied verbatim from the slice.

- [ ] A lane can swing beyond the exact-triplet ceiling the fixed `/3` divisor imposes
- [ ] With tempo-adaptive swing on, the effective ratio widens at slow tempi and narrows toward straight at fast ones, asserted at two tempi from one setting
- [ ] With the feature off, all 45 factory presets render byte-identically, proved by a golden test
- [ ] `maxTimingShift` covers the widened range, shown by a note near a block boundary still being emitted
- [ ] `12-jazz` no longer tells the reader to pick a Swing value per tempo, and a `scope-framing` claim fails if that instruction returns

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

## Task 1 — The mode field and the tempo curve

Modifies `engine/include/poly/types.h`; creates
`tests/swing_curve_tests.cpp` and registers it in `tests/CMakeLists.txt`.

1. Write the failing tests first, against a pure `swingRatioAt(tempo, amount)`
   helper that does not exist yet. Four cases, each pinning a claim the curve
   must make rather than a number it happens to produce:
   - at a ballad tempo the reachable ratio at `amount == 1.0` exceeds the exact
     triplet the fixed `/3` divisor imposes, and reaches about 3.5:1
   - the ratio **narrows monotonically** as tempo rises across at least four
     sampled tempi — this is the property, and a single pair of points would not
     catch a curve that wiggles
   - approaching 300 BPM the ratio is close to straight
   - `amount == 0.0` yields no displacement at any tempo, so the parameter still
     means what it meant
2. Add `SwingMode { Fixed = 0, TempoAdaptive = 1 }` and a
   `SwingMode swingMode = SwingMode::Fixed;` field to `LaneConfig`, beside
   `swingAmount`, with a comment naming M001/S01 and GP01 and stating that
   `Fixed` reproduces the pre-M001 mapping exactly.
3. Implement `swingRatioAt` in `types.h` beside the other inline helpers. It is
   pure arithmetic — no allocation — because `applyTimingShifts` runs on the
   audio thread.
4. **Comment the coefficients as ours.** State that the curve reproduces the
   shape Friberg & Sundström (2002) report — wide at slow tempi, narrowing
   toward straight near 300 BPM — and that the coefficients are Poly's, so
   refining them against a source in hand is a data change needing no code.
   This is the framing M003 used for the jembe profile; claiming these are the
   measured numbers would be a false citation.
5. Run `format`, `unit`, `engine-isolation`. Commit; `GP01` stays open.

## Task 2 — Swing reads the curve, and the lookahead follows

Modifies `engine/src/engine.cpp` and `tests/swing_curve_tests.cpp`.

1. Write the failing render-level cases first:
   - a lane with `swingMode == TempoAdaptive` emits different onsets at 90 and
     at 200 BPM from one `swingAmount`, while a `Fixed` lane emits the same
     relative displacement at both
   - **mutation-prove them.** A case that passes because both lanes happen to
     agree proves nothing; each must fail under a probe that forces
     `swingMode` to `Fixed`. M003/S01 shipped two vacuous tests that only a
     mutation round caught, in this same file.
   - a `Fixed` lane's onsets are byte-identical to the pre-change engine, which
     is the back-compatibility claim stated as a test
2. In `applyTimingShifts`, branch on `cfg.swingMode`: `Fixed` keeps
   `swingAmount * stepDurPpq / kSwingSyncopationDivisor` unchanged;
   `TempoAdaptive` derives the divisor from `swingRatioAt(tc.tempo, ...)`.
   `applyTimingShifts` already receives `tc`, so no new plumbing is needed —
   confirm that rather than assuming it.
3. **Update `maxTimingShift`.** Issue #149 names this itself: the lookahead
   bound must track the widened range, or a note displaced beyond the old
   ceiling is dropped at a block boundary. Take the widest displacement the
   curve can produce at the current tempo, not the fixed `/3` figure.
4. Add a split-rendering case: rendering a range in two halves produces exactly
   what rendering it whole produces, swept across several split points. M004/S03
   used this shape; note in a comment that it passed there both before and after
   a bound fix, so it is a regression guard rather than a proof of the bound.
5. Add the byte-identity golden over the factory presets with the mode off.
6. Run `format`, `unit`, `engine-isolation`, `rt-safety`. Commit.

## Task 3 — `12-jazz` stops prescribing a per-tempo value

Modifies `site/src/content/docs/12-jazz.mdx` and
`site/tests/scope-framing.test.mjs`.

1. Write the failing claim first, in `scope-framing.test.mjs` beside the
   existing entries: `12-jazz.mdx` must no longer instruct the reader to set a
   swing value for a given tempo, and must name the mode that now does it.
2. Rewrite the sentence at `12-jazz.mdx` line 25 — "Set the ride cymbal's swing
   to 0.4-0.5 for a medium-tempo…" — so the page describes the mode rather than
   the workaround. Keep the page's existing and correct claim that swing "varies
   continuously with tempo": that sentence is why this slice exists and it stays.
3. Watch the claim fail with the old sentence restored, then restore the new one
   **by an explicit edit, not `git checkout`** — in M007/S03 a checkout restored
   more than the probe and discarded uncommitted work.
4. Run `format`, `site-unit`, `doc-conformance`, `doc-discipline`. Commit.

## Task 4 — Evidence and close-out

1. Append `evidence/M001-S01.md`: each token's result with headline counts, the
   mutation-round outcomes from task 2, and the byte-identity golden's result.
2. Tick the four task boxes and the definition-of-done boxes, set `GP01` and the
   slice `done`.
3. Run `jk-standards ledger`, then the whole validation set once more on the
   final tree. Commit with `Slice: M001/S01` and `Rows: GP01`.
