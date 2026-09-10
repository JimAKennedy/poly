# M003 — Scope and Repositioning: review report

The guide states plainly what it is, what it is not, and exactly where it
simplifies, so a reader can calibrate every claim it makes.

Ledger: `docs/plans/theory-audit/ledger.md`
Branch: `milestone/M003-scope-repositioning`
Generated at the `/jk:auto` review gate. `/jk:ship` is a separate, deliberate
act taken after reading this.

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M003/S01 | About This Guide | F24, F36 | done |
| M003/S02 | Chapter 6 scope | F25 | done |
| M003/S03 | Simplification disclosures | F26, F27, F28, F29, F30 | done |
| M003/S04 | Non-isochrony honesty | F31, B10, F32 | done |
| M003/S05 | Remaining framing items | F33, F34, F35 | done |

Fourteen rows closed: thirteen audit findings and one defect the programme
found in itself (B10). Every slice is `done`, which is the stricter ledger
claim — every definition-of-done box ticked, evidence file present, every row
closed.

## Definition of done

All fifteen boxes across the five slices are checked. By slice:

- **S01** (3) — About This Guide exists with its declared exclusions; every
  theory page and the introduction link to it; the page count is asserted, not
  assumed
- **S02** (2) — Chapter 6 is scoped to Hindustani practice and says what is
  absent; the counterpoint overview agrees with it
- **S03** (5) — the Manding same-cycle presentation, gamelan Rules 3 and 5,
  layakari, and Chapter 6's tala framing each disclose their simplification at
  the point of use
- **S04** (2) — Humanize is distinguished from a measured subdivision profile;
  the Balkan long beat is stated as a style-defining tendency, not sloppiness
- **S05** (3) — Rule 5 admits a third part doubling the pokok; the Rachenitsa
  table carries a `Note` column matching `presets.json` with its stand-ins
  named; Chapter 5's cyclic-time opening is cited

## Validation

Every token each slice declared, as its evidence file recorded it. The final
row is the state of the branch head, and is the one that matters for shipping.

| Token | Command | Result | Where |
|---|---|---|---|
| `format` | `pre-commit run --all-files` | exit 0 | every slice |
| `site-unit` | `npm --prefix site test` | exit 0, 198 tests | S05 close |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | exit 0, 177 tests | S05 close |
| `doc-discipline` | jk-standards doc checks | exit 0 | S01 |
| `gate` | `bash scripts/pre-push-check.sh` | exit 0, 7 stages, 589 ctest tests | S05 close |
| `ledger` | `jk-standards ledger` | conforms | before every commit |

The site suite grew from 184 cases to 198 and doc-conformance from 162 to 177
across the milestone; every increase is a lock this milestone added.

`gate` was not treated as subsuming the other tokens. Its clang-format stage is
not `pre-commit run --all-files`, and nothing in it runs the site suite — poly
issue #272 is that CI never runs `npm --prefix site test`, which is why the
lock hosts are also wired into `check-doc-conformance.sh`.

## Traceability

Nineteen commits, measured against `origin/main` at `8e8c36d`. **Every one
carries a `Slice:` trailer; there are no untraced commits.**

- `fbd91f7` docs: plan M003/S01 — About This Guide — M003/S01
- `bd25dd0` docs(site): add About This Guide with its declared scope exclusions — M003/S01, rows F36
- `9164565` docs(site): make About This Guide reachable from every page that needs it — M003/S01, rows F24
- `7c77992` docs: plan M003/S02 — Chapter 6 scope — M003/S02
- `aa99349` docs(site): scope Chapter 6 to Hindustani and state what is absent — M003/S02, rows F25
- `abe5bad` docs: plan M003/S03 — Simplification disclosures — M003/S03
- `9419150` docs(site): mark the Manding same-cycle presentation as a simplification — M003/S03, rows F26
- `2195af6` docs(site): mark gamelan Rule 3 style-dependent and hedge Rule 5 to Tenzer — M003/S03, rows F27, F29, F30
- `1ddc72d` docs(site): say what layakari does that a subdivision change does not — M003/S03, rows F28
- `4628f8e` docs: plan M003/S04 — Non-isochrony honesty, and fix the ledger guard's B-row bug — M003/S04
- `27df463` docs(site): say how Humanize differs from a measured subdivision profile — M003/S04, rows F31, B10
- `94232be` test(site): lock the Balkan long-beat honesty so an edit cannot drop it — M003/S04, rows F32
- `2f658f9` plan(M003/S05): plan the last framing slice and correct two audit claims — M003/S05
- `d5cc9bd` docs(site): ground Chapter 5's cyclic-time opening in Tenzer — M003/S05, rows F35
- `c63d46e` plan(M003/S05): repair Task 2 — the third-part sentence carries no citation — M003/S05
- `e35fc10` docs(site): let Gamelan Rule 5 admit a third part that doubles the pokok — M003/S05, rows F33
- `0a684c5` docs(site): say which Rachenitsa lanes are GM stand-ins, and lock it to the preset — M003/S05, rows F34
- `d9f099c` docs(plans): record M003's decisions for the auto run's report — M003/S05
- `d0a30a3` docs(plans): close M003/S05 — the last framing slice — M003/S05

A note on those SHAs: the branch was rebased onto `origin/main` between
M003/S05's task 3 and task 4, picking up `chore: adopt jk-standards v0.15.0
(#279)`. Commits made before that rebase were replayed and carry different
hashes than the ones reported while they were being made. Content is unchanged;
the full gate above ran on the rebased tree, which is what a reviewer will see.

## Left deliberately open

- **M006/S04, row B11** — whether *kotekan polos* has an attestation, and
  whether any source documents the practice Rule 5 now describes without it.
  M003/S05 wrote the practice with neither the label nor a citation, and
  forbids the label in case `S05-F33`. This is the one piece of M003's subject
  matter that leaves the milestone unresolved, and it is `open` on purpose.
- **The milestone's own `Status`** is still `planned` in the ledger while every
  slice is `done`. `jk-standards ledger` accepts this, and `/jk:close` is what
  sets a milestone `done` after its pull request merges.

## What a reviewer should look at twice

- **Two audit claims were corrected rather than implemented.** The audit asked
  Rule 5 to name *kotekan polos* citing nothing, and asked the Rachenitsa table
  to mark "tupan and kaval" stand-ins in a `Note` column that did not exist,
  calling the tupan pitched when it is a bass drum. Both corrections are
  recorded in the rows' `Item` cells with the evidence that settles them. If
  either correction is wrong, the rows are where to argue with it.
- **Three lock cases pass on the day they are written** — `S03-F29`, `S04-F32`
  and `S05-F34`'s Note column existed only after the change, but `S04-F32`
  guarded prose that was already correct. For those, the deletion test in the
  evidence file is the only proof the case is not vacuous.
- **`S05-F34` has a pre-satisfied arm.** Its `roleLabel` check for `rim` is
  already satisfied by the `PolyPreviewCard` caption within the same window.
  The `kick`, `woodblock`, `hat` and two `no GM drum` arms drive it.
- **A plan residue is unswept.** M003/S05's Task 2 step 1 sample `rule:` string
  still reads "cited and linked to Rule 6", contradicting step 3, the
  definition of done and the F33 row. Nothing executable depends on it.

## Decisions

Reproduced verbatim from `docs/plans/theory-audit/M003-decisions.md`.


Append-only. One entry per question asked, answer given, or judgment call made
on the user's behalf.

**Scope note.** This file was created during the `/jk:auto` run that executed
M003/S05's last task, so it records S05's decisions in full and does not
reconstruct S01–S04's. Those slices' reasoning is in their commit messages and
in `evidence/M003-S0{1,2,3,4}.md`, which are the contemporaneous record.

## 2026-09-09 — planning M003/S05

- **Q:** F33 asks Rule 5 to name *kotekan polos*, "a third player on structural
  pokok tones". Nothing in the repo attests the term, the audit cites no source
  for it, and Rule 3 already uses *polos* for one of the interlocking pair. How
  should it be handled? — **A:** Write the substance, drop the label.
- **Decision:** Rule 5 gains the practice without the term; the sourcing
  question becomes M006/S04 row B11 — **Why:** M002 spent a milestone on
  citation integrity, and shipping an unsourced term into the same guide would
  undo it.
- **Q:** F34's definition of done says the table should mark its "tupan and
  kaval notes" as GM stand-ins, and the audit refers to a "Note" column. The
  table has no Note column, and the preset shows tupan maps to real drums while
  kaval and gadulka are the stand-ins. Which should be written? — **A:** Add a
  GM column sourced from `presets.json`.
- **Decision:** A `Note` column for all four lanes plus a paragraph naming the
  GM sounds, with the audit's two errors recorded in F34's `Item` cell —
  **Why:** the preset makes every value verifiable, so the lock has a real
  oracle instead of hand-copied numbers.
- **Decision:** The audit's miscount of Rule 5 as listing "five styles" (it
  lists four) is recorded in F33's `Item` rather than raised as its own row —
  **Why:** no outstanding work follows from it.

## 2026-09-09 — executing M003/S05

- **Decision:** F35 cites Tenzer (2000), not Geertz's *Negara* (1980) — **Why:**
  the audit mentions Geertz as the framing's origin but recommends Tenzer, and
  Geertz has no appendix entry; adding one is M006's scope.
- **Decision:** `S05-F35`'s pattern binds the citation to the opening paragraph
  with a bounded span — **Why:** `05-gamelan.mdx` cites Vitale further down, so
  an unbounded match would pass on a page whose opening was still uncited.
- **Q:** Task 2's "cite `fr-tenzer-2000`" could not be followed honestly: no
  source in the repo attests that a third part doubles the pokok rather than
  interlocking. How should the plan be repaired? — **A:** Ground it internally,
  fold the attribution into B11.
- **Decision:** Rule 5's sentence carries no citation and cross-references Rule
  6 and Construction's Pokok lane; B11 grew to own the attribution as well as
  the term — **Why:** citing on the strength of an appendix annotation would
  have swapped an unsourced term for an unsourced attribution, the same defect
  B11 exists for.
- **Decision:** `S05-F33` matches `third part`, not `third (?:part|lane)` —
  **Why:** Construction step 4 already says "a sparse third lane" and the patch
  names a "Pokok melody" lane, so the `lane` alternative would have passed on an
  unchanged Rule 5.
- **Decision:** Rule 6 is cross-referenced in plain text, `(Rule 6)` — **Why:**
  the plan said to use "the anchor convention the page already uses", and the
  page defines no rule anchors; plain-text parentheticals are its convention.
- **Decision:** `S05-F34` reads `site/src/generated/presets.json`, not the
  `site/public/webui/presets.json` the plan named — **Why:** the two are
  byte-identical, and `src/generated` is the copy
  `preset-table-conformance.test.mjs` already imports.
- **Decision:** `S05-F34`'s prose window is bounded to the region between the
  table and the next heading — **Why:** the first draft ran to end of file,
  which would have let a mention anywhere later in the chapter satisfy it.
- **Decision:** the `Note` header sits between `Subdivision` and `Velocity` with
  bare note numbers — **Why:** that is what `PresetTable` emits for the column,
  so a later migration off the hand-written table is a drop-in.
- **Judgment call:** Task 2's step-1 sample `rule:` string still read "cited and
  linked to Rule 6" after the repair. The rule string was written to match step
  3, the definition of done and the F33 row, all of which say the sentence
  carries no citation — **Why:** the intent was unambiguous and a stale
  adjective in an illustrative snippet is not worth a halt. Recorded in
  `evidence/M003-S05.md` and still unswept in the plan.
- **Judgment call:** `site-unit` was run as `npm --prefix site test` rather than
  the plan's `node --test site/tests/`, which collects one file from the repo
  root and exits 1 — **Why:** the plan itself instructs that
  `.jk/validations.yml` is authoritative on disagreement. Corrected in the plan
  by the repair commit.
