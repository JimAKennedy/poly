---
class: gated
---

# M001 — Review report

Status: current (2026-09-18)

Generated from `docs/plans/guide-parity/ledger.md`, git, and
`M001-decisions.md` for the review that precedes `/jk:ship`.

**Vision:** Poly's swing and humanize behave as the measured literature the
guide already cites describes, and the pages stop prescribing a fixed amount.

**Branch:** `milestone/M001-feel`, cut from `main` at `458a6d8`.

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M001/S01 | Swing widens and tracks tempo | GP01 | done |
| M001/S02 | Humanize drifts rather than jitters | GP02 | done |

## Definition of done

**M001/S01**

- [x] A lane can swing beyond the exact-triplet ceiling the fixed `/3` divisor imposes
- [x] With tempo-adaptive swing on, the effective ratio widens at slow tempi and narrows toward straight at fast ones, asserted at two tempi from one setting
- [x] With the feature off, all 45 factory presets render byte-identically, proved by a golden test
- [x] `maxTimingShift` covers the widened range, shown by a note near a block boundary still being emitted
- [x] `12-jazz` no longer tells the reader to pick a Swing value per tempo, and a `scope-framing` claim fails if that instruction returns
- [x] Both of this milestone's mode fields survive a save and reload, and a pre-bump state loads as the behaviour it played *(added after the slice first closed — see Decisions)*

**M001/S02**

- [x] Successive humanize offsets on one lane are correlated rather than independent, asserted as a measurable property of the sequence
- [x] The offsets remain a pure function of absolute step index — a locate or loop reproduces them exactly
- [x] With the correlated mode off, all 45 factory presets render byte-identically, proved by a golden test
- [x] `renderRange()` gains no allocation, lock or blocking call
- [x] `appendix-design-decisions.mdx` and `18-editors-and-views.mdx` name the correlated behaviour, each locked by a `scope-framing` claim

## Validation

Run on the current head.

| Token | Command | Result |
|---|---|---|
| `format` | `pre-commit run --all-files` | pass |
| `unit` | `ctest --test-dir build` | pass, 644 tests |
| `engine-isolation` | `ctest --test-dir build-engine` | pass, 521 tests |
| `rt-safety` | `bash scripts/check-realtime-safety.sh` | pass |
| `site-unit` | `npm --prefix site test` | pass, 290 tests |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | pass |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` | pass |
| — | `jk-standards ledger` | 3 ledgers conform |

## Traceability

| Commit | Subject | Slice | Rows |
|---|---|---|---|
| `961f6f8` | docs(plans): front-load M001's decisions and plan both slices | M001/S01, M001/S02 | — |
| `8b70e62` | feat(engine): a swing ratio that varies with tempo | M001/S01 | — |
| `1bacd49` | feat(engine): swing displacement follows the tempo curve | M001/S01 | — |
| `c8fa554` | docs(site): 12-jazz stops prescribing a swing value per tempo | M001/S01 | — |
| `9f13d97` | docs(plans): close M001/S01 with tempo-adaptive swing shipping | M001/S01 | GP01 |
| `7506edd` | feat(engine): a correlated fluctuation source for humanize | M001/S02 | — |
| `41d4abb` | feat(engine): humanize can drift instead of jittering | M001/S02 | — |
| `73137d5` | feat(engine): serialize both feel modes at state version 21 | M001/S01 | — |
| `d9f9aab` | docs(site): two pages name both humanize shapes | M001/S02 | — |
| `eecae9a` | docs(plans): close M001/S02 with correlated humanize shipping | M001/S02 | GP02 |

10 commits, **0 untraced**.

## What a reviewer should look at twice

1. **A planning gap shipped two slices before it was caught.** Neither
   `swingMode` nor `humanizeMode` was serialized: set tempo-adaptive swing, save,
   reopen, and it was gone. S01's definition of done never mentioned state and
   its plan had no serialization task, while S02's plan says to carry the field
   "in the same `kStateVersion` bump as M001/S01's `swingMode`" — assuming a bump
   S01 never made. The two plans disagreed with each other and nothing caught it
   until S02 reached that step. S01 was reopened rather than a third slice
   invented, because a slice whose deliverable is two other slices' unfinished
   half makes a planning error look like a deliverable.

2. **The lookahead bound is proven here, where M003/S03's was not.** Leaving
   `maxTimingShift` at the fixed `/3` figure fails
   `SplitRenderingMatchesWholeRenderingWhenAdaptive` — the shortfall is large
   enough to drop an onset at a seam. M003/S03's equivalent split test passed
   both before and after its bound fix, because the additive path enumerates
   whole cycles with a margin that swamped the shortfall there. Reading across
   from one to the other would be wrong, so both are recorded.

3. **A mutation round that proved nothing, caught and redone.** S02 task 3's
   first probe re-inserted the old phrasing *alongside* the new one. Those claims
   are `present`-only, so adding text breaks nothing and the probe passed. The
   regression that matters is the correction being removed; probing that fails
   both claims by name. A mutation round that cannot fail is exactly as
   misleading as none, which is this programme's oldest lesson.

4. **Which test carries the claim, named rather than assumed.** In both slices
   most cases survive their mutation probe — determinism, range, preset identity
   and lane independence are true of the old behaviour too. Exactly one case per
   feature discriminates, and the evidence says which, so nobody later mistakes
   a green suite for four independent proofs.

5. **The curve's coefficients are ours and say so.** `swingRatioAt` reproduces
   the *shape* Friberg & Sundström report; the values are Poly's, commented as
   such, so refining them against a source in hand is a data change needing no
   code. Same framing M003 used for the jembe profile.

6. **A prediction that held.** M001/S01's size-guard comment said `humanizeMode`
   was expected to fit the padding `swingMode` opened. It did — `GrooveState` is
   unchanged at 18000 bytes across S02.

7. **What the prose change deliberately kept.** `12-jazz`'s *per-lane
   independence* is not the workaround — different voices genuinely swing at
   different ratios, which the mode does not address — and the claim's `present`
   list requires it to survive, so a later edit cannot drop it while satisfying
   the rest.

## Decisions

Verbatim from `M001-decisions.md`.

nning M001 (both slices, front-loaded)

**Classification.** Both slices are **bounded**: well-scoped changes to flows
that already exist in this repo. Swing and humanize are both applied today in
`applyTimingShifts`; neither slice introduces a subsystem. The design below was
presented in chat and approved before any code was written.

- **Decision:** Both features sit behind a **per-lane, state-only mode field**,
  following M002's `kotekanMode`. — **Why:** The per-lane expression parameter
  family is full at `kParamsPerLane == 16` and the core family is at 14, so
  neither has room; and a feel-mode is a style choice rather than something
  anyone automates. One `kStateVersion` bump covers both fields, both defaulting
  off.

- **Decision:** The `Swing` VST3 parameter keeps its normalised 0–1 range; what
  changes is what `1.0` *means*, and only with the mode on. — **Why:** `Swing` is
  expression slot 5, `Kind::Unit01`. Widening the mapping unconditionally would
  move every existing preset that sets a swing value, which the milestone's
  byte-identity rule forbids. Off: `swingAmount * stepDurPpq / 3`, untouched.
  On: a tempo-dependent divisor with the ceiling widened to about 0.43 step
  (≈3.5:1), which is the figure issue #149 itself gives.

- **Q:** The swing curve has no published coefficients to copy — Friberg &
  Sundström report a shape and two anchor figures. How should the shipped curve
  be framed? — **A:** Ship the shape, state it is ours.
- **Decision:** Encode a curve reproducing the reported behaviour — about 3.5:1
  reachable at ballad tempi, narrowing toward straight near 300 BPM — with
  coefficients that are Poly's, said so in the comment beside them. — **Why:**
  This is exactly the framing M003 used for the jembe profile: a rendering of a
  published *finding*, not a transcription of a published table. Refining the
  values against a source in hand is then a data change needing no code, and
  claiming they are the measured numbers would be the kind of citation this
  repo's first programme existed to remove.

- **Q:** `M001/S02`'s definition of done says the Humanize pages "no longer
  describe it as jitter where they now mean drift". Only two pages mention
  randomness at all, and neither becomes false — they become imprecise. What
  should that item require? — **A:** Rescope to the two pages and lock the
  correction.
- **Decision:** `appendix-design-decisions.mdx`'s "humanize jitter" and
  `18-editors-and-views.mdx`'s "a different random displacement on every cycle"
  are corrected to name the correlated behaviour, each locked by a
  `scope-framing` claim. The slice's DoD item is rewritten from "the pages" to
  those two. — **Why:** Fourteen pages mention Humanize; two describe its
  mechanism. Editing the other twelve to satisfy a checklist would be prose
  written for a gate rather than for a reader. The finding that neither page was
  actually *wrong* is recorded here so a later reader does not mistake the small
  edit for an oversight.

- **Decision:** Every engine slice in this milestone owes a golden proving the
  45 factory presets render byte-identically with its feature off. — **Why:**
  Inherited from the ledger, which took it from M003/S01. It is what makes a
  timing change reviewable without re-auditing 45 presets by hand.

- **Nothing deferred.** Both slices are planned in full below; neither has a
  question that only becomes answerable once the other has landed.

## 2026-09-18 — a planning gap found during M001/S02 task 2

- **Finding:** Neither `swingMode` nor `humanizeMode` was serialized. A user
  setting tempo-adaptive swing, saving and reopening would find it gone.
  `CLAUDE.md` calls unversioned state "a preset compatibility time bomb"; this is
  the adjacent failure — state that does not persist at all.
- **The gap was in the planning, and it was mine.** `M001/S01`'s definition of
  done never mentioned state and its plan had no serialization task, while
  `M001/S02`'s plan task 2 says to carry the field "in the same `kStateVersion`
  bump as `M001/S01`'s `swingMode`" — assuming a bump S01 never made. The two
  plans disagreed, and the ledger's S01 definition of done was the wrong one.
  Every prior milestone here bumped the version as a matter of course: M002 for
  the kotekan fields, M003 for the subdivision profile.
- **Q:** S01 is already `done` and its DoD never mentioned state. How should
  this be corrected? — **A:** Reopen S01 and add a shared state task.
- **Decision:** `M001/S01` returns to `in-progress` with a new definition-of-done
  item and a task 5 covering **both** fields in one `kStateVersion` bump, 20 →
  21. — **Why:** One version for the milestone is what S02's plan already
  assumes, and it is how M002 and M003 handled theirs. A third slice would have
  made a planning error look like a deliverable; shipping unsaved would have
  made a feel setting that vanishes on reload, which is close to not shipping it.
- **`GP01` stays `done`.** The swing capability is delivered; what was missing is
  persistence, which the new DoD item covers for the milestone rather than for
  that row.
