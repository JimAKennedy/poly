# M003/S04 — Non-isochrony honesty

**Slice:** M003/S04
**Ledger:** `docs/plans/theory-audit/ledger.md`

Two traditions where the played rhythm is systematically not the notated one.
The Balkan page already says so honestly; the Sub-Saharan page admits an
approximation without ever saying how it differs. This slice states the
difference and locks both.

## Task status

The executable state of this plan. `/jk:next` reads the first unchecked box
here to decide what to do; each task's own commit ticks its box.

- [x] Task 1 — Sub-Saharan: random jitter versus systematic profile, and what Poly actually has (F31, B10)
- [ ] Task 2 — Lock the Balkan long-beat honesty (F32)

## Definition of Done

Copied verbatim from the slice. The tasks below argue against *this* text.

- [ ] `theory-sub-saharan-africa.mdx` states explicitly that Humanize is random
      jitter, against Polak's systematic style-specific profiles — not merely
      that it is an approximation
- [ ] Both non-isochrony disclosures are locked by tests, so the Balkan one
      already in the tree cannot be dropped

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

Run `node --test site/tests/scope-framing.test.mjs` in the inner loop.

**Gate ordering.** `format` runs the `ledger` pre-commit hook, which fails while
a slice claiming `done` has no evidence file. Task 2 closes the slice, so its
order is: `site-unit` and `doc-conformance` first, then the evidence, then
`format`.

**Validate after `git add`** — `jk-standards` does not enumerate untracked files
under a doc root (jk-standards#96).

## Context

**What the engine actually does, verified in the code — do not restate this
from memory.** `engine/src/engine.cpp` `applyTimingShifts` applies, in order:
swing, syncopation offset, then `cfg.microTimingMs[cycleStep]` — a per-step
array — and finally Humanize, which computes `jitterPpq` scaled by
`deterministicRand(...)`. So:

- **Humanize is random jitter.** Seeded, therefore reproducible for a given
  patch and seed, but with no systematic per-position structure. That is the
  contrast F31 asks for: Polak (2010) documents a *stable short-medium-long
  subdivision profile* — specific positions, specific ratios, persisting across
  tempi and players.
- **Per-step micro-timing is the systematic control, and it already exists.**
  `microTimingMs` is reachable from the WebUI: `web_ui_view.cpp` handles a
  `setMicroTiming` action, clamped to ±20 ms, and `ui.js` renders the
  micro-timing bars. What Poly lacks is a jembe *profile* to load, not the
  mechanism to express one.

That second point is **B10**, a row this programme added because planning found
it — the audit never named it. Construction step 5 currently says "until Poly
ships subdivision-profile support", which reads as though no systematic timing
control exists.

**F32 is already true.** `theory-balkan.mdx` Rule 8 reads "The long beat is
slightly *less* than 3:2 in practice… a systematic, style-defining tendency, not
sloppiness ([Goldberg 2015]; cf. [London 2012] on NI-meter tolerance ranges).
Until Poly exposes a long-beat ratio control, this is unreproducible on the grid
— know that the grid version is the *notated* aksak, slightly stiffer than the
played one." Nothing there changes. Its case therefore **passes on the day it is
written**, exactly as `S03-F29` did last slice, and the only way to know it is
not vacuous is to delete the sentence and watch it fail. Task 2 does that.

**The two sites for F31.** Rule 8 at line 33 of
`theory-sub-saharan-africa.mdx` states Polak's finding and never mentions
Humanize. Construction step 5 at line 45 says "a light Humanize (≤0.15) as an
admitted approximation of Rule 8" — the admission with no content. The contrast
belongs where Humanize is named, so step 5 carries it; Rule 8 is left as the
statement of what the scholarship found.

**Find both by text, not line number.** Task 1's own edit shifts everything
below it.

**The lock host** is `site/tests/scope-framing.test.mjs`, already wired into the
runner and `REQUIRED`. Append to its `CLAIMS`.

**Reverting a test mutation** while a file holds this task's uncommitted work:
use an inverse edit, not `git checkout --`.

## Task 1 — Sub-Saharan: random jitter versus systematic profile, and what Poly actually has

**Modifies:** `site/src/content/docs/theory-sub-saharan-africa.mdx`,
`site/tests/scope-framing.test.mjs`, `docs/plans/theory-audit/ledger.md`
**Creates:** `docs/plans/theory-audit/evidence/M003-S04.md`
**Rows:** F31, B10

### Steps

1. **Write two failing cases** in `scope-framing.test.mjs`, both with
   `file: 'theory-sub-saharan-africa.mdx'`:

   - `id: 'S04-F31'` — `rule`: ledger F31, the guide flagged Humanize as an
     approximation of Rule 8 without saying how it differs; Humanize applies
     random jitter while Polak documents a systematic style-specific
     subdivision profile. `present`: `['random jitter', 'systematic profile']`.
     Not bare `'systematic'` — the page already contains "systematically", and
     `containsClaim` matches substrings under normalisation, so that arm would
     pass before the edit. Verified: `systematic profile`, `random jitter` and
     `micro-timing` are all absent from the file today.
   - `id: 'S04-B10'` — `rule`: ledger B10, "until Poly ships subdivision-profile
     support" understates the engine, which has a per-step `microTimingMs` array
     exposed as the WebUI micro-timing bars; what is missing is a jembe profile,
     not the mechanism. `forbidden`:
     `['until Poly ships subdivision-profile support']`.
     `present`: `['micro-timing']`.

2. **Run them and watch both fail** — `S04-F31` on its present arm, `S04-B10`
   on its forbidden arm.

3. **Rewrite construction step 5.** Find it by the text "Apply the feel.", not
   by line number. It must keep the practical advice (no swing; small per-lane
   offsets) and then say three things plainly:

   - Humanize applies **random jitter** around each onset;
   - Rule 8's profile is **systematic** — particular positions, particular
     ratios, stable across tempi and players — so jitter approximates the
     *presence* of non-isochrony without reproducing its shape;
   - Poly's **per-step micro-timing** offsets are where such a profile would be
     entered by hand; what Poly does not ship is a measured jembe profile to
     load into them.

   Do not claim Poly can reproduce Polak's profile. It cannot, because no
   measured profile is supplied — the point is that the missing piece is data,
   not a control.

4. **Run both cases and watch them pass**, then **run `site-unit` immediately**
   and confirm the file's other cases still pass.

5. **Run** `format`, `site-unit` and `doc-conformance` staged, and read each
   exit code.

6. **Close F31 and B10** in the ledger: each `Status` to `done`, each
   `Verification` naming its case, each `Lands in` corrected. Record in B10's
   `Item` cell that the correction was found while planning F31 rather than
   inherited from the audit. Use no `|` in any cell.

7. **Append to** `docs/plans/theory-audit/evidence/M003-S04.md`, creating it.
   Name no commit SHA.

8. **Commit** with trailers `Plan: docs/plans/theory-audit/ledger.md`,
   `Slice: M003/S04`, `Rows: F31, B10`.

## Task 2 — Lock the Balkan long-beat honesty

**Modifies:** `site/tests/scope-framing.test.mjs`,
`docs/plans/theory-audit/ledger.md`,
`docs/plans/theory-audit/M003-S04-plan.md`,
`docs/plans/theory-audit/evidence/M003-S04.md`
**Rows:** F32

No prose changes. `theory-balkan.mdx` Rule 8 is already honest; this task adds
the lock that stops a later edit removing it.

### Steps

1. **Write the case.** Add to `CLAIMS`:

   - `id: 'S04-F32'`, `file: 'theory-balkan.mdx'`
   - `rule`: ledger F32 — Rule 8's non-isochrony honesty is already present and
     this case exists so a later edit cannot drop it; it passes on the day it is
     written, and the proof it is not vacuous is in the evidence
   - `present`: `['systematic, style-defining tendency', 'the grid version is the']`
   - `presentRegex`: `[/#fr-goldberg-2015/]`

   Assert `'systematic, style-defining tendency'` rather than a bare
   `'systematic'`: the word alone is too weak to prove Rule 8 in particular
   survived. Do not assert `'notated'` on its own either — check the file first
   if you are tempted to, since Rule 8 is not the only place the word appears.

2. **Run it and expect it to pass immediately.** That is correct for an
   already-true row, not a mistake.

3. **Prove it is not vacuous.** Delete the phrase "a systematic, style-defining
   tendency, not sloppiness" from Rule 8, confirm `S04-F32` fails naming it,
   then restore **with an inverse edit, not `git checkout --`**, and confirm
   `git diff --stat` shows the file unchanged.

4. **Run `site-unit` and `doc-conformance`** staged, and read their exit codes.

5. **Close F32 and the slice.** Set F32 `done`, name case `S04-F32` in its
   `Verification`, correct its `Lands in`, and record in its `Item` cell that no
   prose changed and the row added only the lock. Then tick both
   Definition-of-Done boxes in the slice and in this plan's copy, tick this
   plan's Task 2 box, and set the slice `Status` to `done`.

6. **Append the evidence**, then run `format` last per the gate ordering, and
   `jk-standards ledger`.

7. **Commit** as one unit with trailers
   `Plan: docs/plans/theory-audit/ledger.md`, `Slice: M003/S04`, `Rows: F32`.
