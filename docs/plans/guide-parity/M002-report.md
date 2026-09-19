---
class: gated
---

# M002 — Review report

Status: current (2026-09-19)

Generated from `docs/plans/guide-parity/ledger.md`, git, and
`M002-decisions.md` for the review that precedes `/jk:ship`.

**Vision:** Mutation, ghost, drop and fill decisions are weighted by where the step sits — in the meter, against the timeline, and within the phrase — and the chapters stop prescribing manual workarounds for them.

**Branch:** `milestone/M002-position`, cut from `main` at `00401bb`.

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M002/S01 | A lane can weight by a reference lane's timeline | GP03 | done |
| M002/S02 | Ghosts cluster where funk puts them | GP04 | done |
| M002/S03 | Fills resolve onto the phrase boundary, and a tihai lands | GP05 | done |
| M002/S04 | A response lane answers its call | GP06 | done |

## Definition of done

**M002/S01**

- [x] A lane can name a reference lane, and a mutual reference is refused rather than followed
- [x] Mutation-adds are biased toward or away from the reference lane's onsets by a signed per-lane strength, shown by the distribution of added steps changing with the sign
- [x] Drops are less likely on high-weight steps than on low-weight ones
- [x] With no reference lane named, all 45 factory presets render byte-identically, proved by a golden test
- [x] `03-afro-cuban` stops describing clave alignment as something the reader maintains by hand, and a `scope-framing` claim locks it

**M002/S02**

- [x] Ghost-add probability is higher on weak subdivisions preceding an accent than on those following one, asserted as a distribution over many seeds rather than a single roll
- [x] The weighting scales with the Complexity macro, so low Complexity keeps grooves clean
- [x] With the weighting neutral, all 45 factory presets render byte-identically, proved by a golden test
- [x] `11-funk`'s dedicated ghost lane is no longer the recipe the page prescribes, and a `scope-framing` claim locks the change

**M002/S03**

- [x] Fill probability rises toward the end of a phrase cycle, with a shape parameter controlling how sharply, asserted as a distribution across the cycle
- [x] For an ungated lane the boundary used is the composite convergence point, not silence
- [x] A tihai of a given phrase length lands its final onset exactly on the target, asserted arithmetically rather than by ear
- [x] With the weighting neutral, all 45 factory presets render byte-identically, proved by a golden test
- [x] `06-indian-classical` no longer asks the reader to do the tihai arithmetic by hand, and a `scope-framing` claim locks it

**M002/S04**

- [x] A lane's phrase gate can be defined as open exactly when a named source lane's gate is closed, with an optional lead-in or overlap in beats
- [x] Changing the source lane's phrase settings keeps the antiphony intact, which is the failure the manual recipe has
- [x] A mutual reference between two response lanes is refused rather than followed
- [x] With no response lane named, all 45 factory presets render byte-identically, proved by a golden test
- [x] `15-compositional-grammar` no longer gives interleaving offsets as the recipe for antiphony, and a `scope-framing` claim locks it

## Validation

Run on the current head.

| Token | Command | Result |
|---|---|---|
| `format` | `pre-commit run --all-files` | pass |
| `unit` | `ctest --test-dir build` | pass, 677 tests |
| `engine-isolation` | `ctest --test-dir build-engine` | pass, 554 tests |
| `rt-safety` | `bash scripts/check-realtime-safety.sh` | pass |
| `site-unit` | `npm --prefix site test` | pass, 294 tests |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | pass |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` | pass |
| — | `jk-standards ledger` | 3 ledgers conform |

## Traceability

| Commit | Subject | Slice | Rows |
|---|---|---|---|
| `8fce41d` | docs(plans): front-load M002's decisions, design S01, plan all four slices | M002/S01, M002/S02, M002/S03, M002/S04 | — |
| `31951eb` | feat(engine): weight a lane's stochastic decisions by a reference timeline | M002/S01 | — |
| `67c41b4` | feat(engine): mutation and fill rolls read the timeline weights | M002/S01 | — |
| `75f0670` | docs(site): the clave page names the weighting instead of wishing for it | M002/S01 | GP03 |
| `162127e` | feat(engine): weight ghosts toward the approach to an accent | M002/S02 | — |
| `fb8f913` | docs(site): the funk page stops prescribing a ghost lane | M002/S02 | GP04 |
| `9402cc6` | feat(engine): concentrate fills toward the phrase boundary | M002/S03 | — |
| `ff7e906` | feat(engine): solve the tihai gap so the third repetition lands on sam | M002/S03 | — |
| `3bc7f81` | docs(site): the tihai section gives the gap instead of asking for it | M002/S03 | GP05 |
| `516d6c3` | feat(engine): a response lane's gate answers its call | M002/S04 | — |
| `9c93f93` | docs(site): antiphony is a coupling, not a coincidence of numbers | M002/S04 | GP06 |

11 commits, **0 untraced**.

## What a reviewer should look at twice

1. **A probe that killed nothing, and what it found.** S02's first mutation
   round — discarding the ghost weight where the sources compose — left all four
   cases green. They all exercised `computeGhostWeight` directly, so the weight
   could be computed perfectly and thrown away. Adding a render-level case
   caught a real defect immediately: the approach share was *identical* at
   0.5449 with and without the grammar, because the helper took an
   accent-pattern parameter and the engine was passing the lane's **onsets**. On
   a fully-lit lane every step then read as accented. The fix removes the
   parameter rather than correcting the argument.

2. **A weight that could not redistribute.** S03's fill weight first ran from
   `1.0` to `1 + shape`, only ever raising the probability. The render case
   measured **exactly 1800 early against 1800 late**, with and without a shape:
   once fills saturate, raising changes nothing. It now runs from `1 - shape` to
   `1 + shape`. The same figure then persisted for a *second* reason — the test
   drove `fillEveryNBars`, where `if (isFillBar) return Add;` fires before the
   roll — so the weighted probability was never consulted at all.

3. **A guard that is defensive, not load-bearing, and said so.** S04's
   mutual-reference guard can be removed and nothing fails. What actually
   prevents recursion is `responseGateOpen` reading the source's **own** phrase
   gate rather than its response gate. Rather than contrive a case that would
   fail, a case now pins the property doing the real work, so a later
   "simplification" is caught by a test rather than by a stack overflow.

4. **The composition rule deliberately contradicts M003.** That milestone ruled
   a subdivision profile takes *precedence* over `cellSizes`; here weights
   **compose multiplicatively**. The argument is in `M002-S01-design.md` and the
   decisions file: those were two competing definitions of one grid, where these
   are probabilities. It reads as an inconsistency until you see why, which is
   exactly why it is written down rather than left to be inferred.

5. **The size claim, measured.** `GrooveState` ends M002 at **18192 bytes**, up
   from 18000 — about 1%. The issues proposed storing per-step weight arrays per
   lane, which would have been roughly **8 KB** on a struct copied three times
   per block. The settings-on-the-lane, weights-in-the-context split is what
   bought that, and the per-slice guard comments record each increment.

6. **A stale build, twice more.** Two probes reported results from a binary that
   had not recompiled — once because the probe edit did not compile at all and
   `ctest` silently ran the previous build, reporting every case passing. That
   is the hazard in its most dangerous form: a probe appearing to prove tests
   cannot fail. Both plans and the decisions file carry a standing instruction
   about it, and every mutation result here was read from a build checked first.

7. **One test asserted the wrong unit's responsibility.** S01's
   `RefusesAnUnusableReference` expected `computeStepWeights` to reject an
   out-of-range lane index. It cannot: it receives an already-resolved pattern.
   The test was split along the real boundary rather than loosened.

## Decisions

Verbatim from `M002-decisions.md`.

## 2026-09-18 — planning M002 (all four slices, front-loaded)

**Classification.** `M002/S01` is **architectural**: it establishes a mechanism
the other three consume, so it carries a written design
(`M002-S01-design.md`), approved in chat before any code. `S02`, `S03` and
`S04` are **bounded** against that mechanism — each adds one weight source or,
in S04's case, reuses the reference-lane resolution for a different purpose.

- **Q:** The issues all say the per-step weights should be "precomputed per
  lane". Stored as arrays on `LaneConfig` that is roughly 8 KB per
  `GrooveState`, a 45% growth. Where should they live? — **A:** Settings on the
  lane, weights in the render context.
- **Decision:** `LaneConfig` carries ~24 bytes of scalar settings per lane; the
  weights are computed into `LaneContext`, which is per-render and never
  serialised. — **Why:** Eight kilobytes on a struct copied three times per
  block, to hold values derivable from a handful of scalars, is a poor trade.
  The reference lane's onsets resolve once per lane per block exactly as the
  kotekan source pattern already does, so this reuses a proven shape rather
  than inventing one. The issues' wording is a proposal, not a constraint.

- **Q:** A step can be metrically weak *and* against the clave *and* near a
  phrase end. How should multiple weight sources combine? — **A:**
  Multiplicatively, with `1.0` neutral.
- **Decision:** Each source contributes a factor and they multiply; an unset
  source contributes exactly `1.0`. — **Why:** The byte-identity guarantee then
  holds by construction rather than by assertion. **This deliberately differs
  from M003's ruling** that a subdivision profile takes precedence over
  `cellSizes` rather than combining with it, and the difference is the
  justification: those were two competing definitions of one grid, and combining
  them produced a placement nobody could reason about. These are probabilities,
  and composing is what probabilities do.

- **Decision:** Four named weights — `add`, `drop`, `ghost`, `fill` — rather
  than one scalar reinterpreted per decision. — **Why:** One number cannot mean
  attraction for adds, protection for drops, grammar for ghosts and shape for
  fills at once. Overloading one field with several meanings is what M003
  rejected; four named fields cost nothing in a per-render context and make each
  roll site say which weight it reads.

- **Decision:** `M002/S04` stays in this milestone rather than becoming its own.
  — **Why:** It reuses S01's reference-lane resolution and mutual-reference
  guard, reading gate state where S01 reads onsets. The shared part is small,
  and it was put to the user at the design gate as the cheap moment to split it
  back out; it was approved as grouped.

- **Decision:** Every slice owes a golden proving the 45 factory presets render
  byte-identically with its weight source unset. — **Why:** Inherited from the
  ledger, which took it from M003/S01, and reinforced by M001 — where the rule
  is what made a timing change reviewable without re-auditing 45 presets.

- **A standing instruction for every slice here, from M001's findings.** The
  discriminating case must be named in the evidence, because most cases in a
  weighting suite survive their mutation probe: determinism, range and
  preset-identity hold for the unweighted behaviour too. And a `present`-only
  site claim is proved by *removing* the correction, not by adding the old
  phrasing beside it — M001/S02's first probe did the latter and proved nothing.

- **Nothing deferred.** All four slices are planned below; none has a question
  that only becomes answerable once another has landed.
