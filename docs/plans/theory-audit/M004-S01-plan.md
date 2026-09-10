# M004/S01 — Chapter 2 patch

**Slice:** M004/S01, in `docs/plans/theory-audit/ledger.md`
**Depends:** M004/S05, which is `done` — its checklist is this slice's oracle
**Decisions:** `M004-decisions.md`

The choice between carrying the missing lane and declaring a divergence was
taken up front: **carry the lane.** The milestone's Vision is that a patch
follows its page's rules or says why not, and there is no reason this one
should not follow them.

## Task status

- [x] Task 1 — Lay the dance beat and make the timeline legible
- [ ] Task 2 — Close the slice

## Definition of Done

- [ ] The Chapter 2 patch either carries the timeline-mode bell lane and the
      dance-beat lane its theory page requires, or cross-references the fuller
      theory-page construction in band
- [ ] The choice is legible from the patch table alone

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `e2e` | `bash scripts/site-verify-local.sh` |

`.jk/validations.yml` is authoritative if this table disagrees with it.

## What is already true

- The bell **already runs in timeline mode**: the `Sub-Saharan: Agbekor` preset
  carries `timeline: true` on lane 1. Rule 1 is satisfied in behaviour; what is
  missing is that the table cannot show it.
- Two `patch-divergence-ok` markers stand above the patch, `ssa-dance-beat` and
  `ssa-timeline-legible`, both naming F37. **Removing them is part of this
  slice**, not a separate tidy-up: a marker outliving the row it names is what
  M007/S03's B14 exists to catch.
- The `preset="Sub-Saharan: Agbekor"` attribute is a display caption, not a
  binding — `polypatch-preset-resolution.test.mjs` requires only that the name
  resolves to a loadable preset. The table is the authored teaching example, so
  adding a lane to it needs no engine or `presets.json` change. Do not edit the
  preset.

## Task 1 — Lay the dance beat and make the timeline legible

Closes **F37**.

**Files:** `site/src/content/docs/02-sub-saharan-africa.mdx`

`theory-sub-saharan-africa` construction step 2: *"Lay the dance beat. A low
drum at 12 steps, 4 hits (E(4,12) = every third pulse), moderate velocity. You
now have the 3:2 matrix (Rule 4)."*

1. **Confirm the two rules still fail** before changing anything, by removing
   both markers and running
   `node --test site/tests/theory-patch-conformance.test.mjs`. They must fail
   as `ssa-dance-beat` and `ssa-timeline-legible`. This is the red: the markers
   were the only thing making them pass.

2. **Add the dance-beat lane** as lane 2, directly after the bell, so the table
   reads in the construction's order — bell, dance beat, supports, lead — and
   renumber the lanes below it. 12 steps, 4 hits, rotation 0, 1/8 subdivision,
   Ghost 0, Kotekan off. Note **41**, GM Low Floor Tom: it is lower than the
   support drum's 43 and is not otherwise used in this table. Velocity **95** —
   "moderate", below the bell's 110 and above the support drum's 90, which is
   what a dance beat carrying the 3:2 matrix against the bell should sound like.

3. **Add the `Timeline` column**, `on` for the bell and `off` for every other
   lane, matching the preset's `timeline` flags rather than asserting them.
   Place it after `Subdivision`, where `PresetTable` emits its own conditional
   columns, so a later migration is a drop-in.

4. **Run and watch both rules pass.**

5. **Prove each still bites.** Remove the dance-beat lane, watch
   `ssa-dance-beat` fail; restore by inverse edit. Remove the `Timeline` column
   header, watch `ssa-timeline-legible` fail; restore. Confirm with
   `git diff --stat` that the file matches its post-step-3 state. Never
   `git checkout --` on a file carrying uncommitted work.

6. **Check.** `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`, `pre-commit run --all-files`.

7. **Commit** with `Rows: F37`, ticking Task 1, setting F37 `done` with the two
   rule ids named in its `Verification`, and appending to
   `docs/plans/theory-audit/evidence/M004-S01.md`.

## Task 2 — Close the slice

Closes no row; closes the slice.

1. **Confirm no marker names F37.** `grep -rn patch-divergence-ok site/` must
   show neither `ssa-dance-beat` nor `ssa-timeline-legible`, and the suite's
   printed count must have dropped from 6 to 4.
2. **Tick both definition-of-done boxes** in the slice and in this plan's copy,
   tick Task 2, confirm F37 is `done`, set the slice `Status` to `done`.
3. **Run every token**: `pre-commit run --all-files`, `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`, then
   `bash scripts/site-verify-local.sh` for `e2e` — Playwright, sample
   equivalence, macro-diff and first-bar parity; minutes, not seconds. Read its
   exit code.
4. **Run `jk-standards ledger`** with the slice `done`.
5. **Append the evidence**, naming each token and what it returned. No commit
   SHA — this file ships inside the commit it describes.
6. **Commit** with `Rows: —`.

## Self-review

**DoD coverage.** Item 1 (the lane, and the bell's timeline mode) → Task 1
steps 2 and 3. Item 2 (legible from the table alone) → Task 1 step 3, which is
the whole reason the column is added: the mode was already true and invisible.

**Row coverage.** F37 → Task 1, closed at step 7; its verification is the two
checklist rules `ssa-dance-beat` and `ssa-timeline-legible`, both of which S05
already built and both of which step 5 proves still bite.

**Placeholder scan.** No TBDs. Every value named — 12 steps, 4 hits, note 41,
velocity 95 — is either read from the theory page's construction step 2 or
justified against the table's existing lanes in step 2.

**Name consistency.** `ssa-dance-beat` and `ssa-timeline-legible` throughout,
matching the ids S05 defined. The column is `Timeline`.
