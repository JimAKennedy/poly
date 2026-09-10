# M003/S03 — Simplification disclosures

**Slice:** M003/S03
**Ledger:** `docs/plans/theory-audit/ledger.md`

Five places where the guide compresses something the scholarship treats with
more nuance. None is an error. Each becomes a disclosure, so a reader can tell a
simplification from a finding — and one of them is already honest and only needs
locking.

## Task status

The executable state of this plan. `/jk:next` reads the first unchecked box
here to decide what to do; each task's own commit ticks its box.

- [x] Task 1 — Chapter 2: mark the Manding same-cycle flattening (F26)
- [x] Task 2 — Gamelan Rules 3, 4 and 5: style-dependence, and lock Rule 4 (F27, F29, F30)
- [ ] Task 3 — Chapter 6: layakari is not a subdivision change (F28)

## Definition of Done

Copied verbatim from the slice. The tasks below argue against *this* text.

- [ ] The Manding same-cycle presentation is marked as a pedagogical flattening
      and cites Charry
- [ ] Gamelan Rule 3's polos/sangsih assignment is marked style-dependent, with
      *norot* named as the reversing case
- [ ] The layakari section says plainly that Poly's subdivision change is not
      what layakari does to hit density
- [ ] Gamelan Rule 5's "choose one and keep it" is hedged to match Tenzer
- [ ] Gamelan Rule 4's existing strict-complementation honesty is locked by a
      test, so a later edit cannot drop it

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

Run `node --test site/tests/scope-framing.test.mjs` in the inner loop.

**Gate ordering.** `format` runs the `ledger` pre-commit hook, which fails while
a slice claiming `done` has no evidence file. Task 3 closes the slice, so its
order is: `site-unit` and `doc-conformance` first, then the evidence, then
`format`. Tasks 1 and 2 may run `format` at any point.

**Validate after `git add`.** `jk-standards` does not enumerate untracked files
under a doc root (jk-standards#96), so a check run before staging proves nothing
about the tree you are about to push.

## Context

**The lock host** is `site/tests/scope-framing.test.mjs`, created by M003/S01
and already wired into `scripts/check-doc-conformance.sh` and the `REQUIRED`
array of `doc-conformance-wiring.test.mjs`. Append to its `CLAIMS`; no wiring
work is needed.

**Charry is named in prose, deliberately unlinked.** F26's disclosure must cite
Charry (2000), *Mande Music* — but the appendix entry for it does not exist. It
is added by **F44 in M005/S01**, whose own verification reads "Cited inline at
the F26 disclosure", and which `Depends: M003/S03`. So the ledger's sequencing
is: this slice writes the disclosure naming Charry, and M005/S01 later adds the
entry and turns the name into a linked citation.

Write `Charry (2000), *Mande Music*` as plain text with **no**
`/appendix-references/#…` link. Do not invent an anchor: `research-provenance`
requires every citation anchor to resolve, so a link to an entry that does not
exist yet fails the build. Do not add the appendix entry either — that is F44's
row, and doing it here leaves that row with nothing to close.

**F29 is already true.** `theory-gamelan.mdx` Rule 4 already reads "Strict
complementation is only the textbook case. In practice both players strike
together at cadence points, phrase joins, and *angsel* (break) figures — the
overlap marks structure ([Tenzer 2000](/appendix-references/#fr-tenzer-2000))."
Its disposition is `accept`: the row exists to lock honesty that is already
there, not to add any. Its case therefore **passes the moment it is written**,
so the only way to know it is not vacuous is to delete the sentence and watch it
fail. Task 2 does exactly that, and the plan expects no red-first for this one
case.

**The five sites.**

| Row | File | Where |
|---|---|---|
| F26 | `02-sub-saharan-africa.mdx` | the `## Manding Traditions: Djembe Ensembles` paragraph, which says Manding patterns "typically lock to a shared cycle length" |
| F27 | `theory-gamelan.mdx` | Rule 3, "Polos leans onbeat, sangsih leans offbeat" |
| F29 | `theory-gamelan.mdx` | Rule 4, "The parts overlap at structural tones" — already honest |
| F30 | `theory-gamelan.mdx` | Rule 5, "choose one and keep it; mixing interlock styles mid-phrase is not idiomatic" |
| F28 | `06-indian-classical.mdx` | the layakari paragraph, "maps directly to Poly's subdivision parameter" |

**Rule 5 already names *norot*, and Rule 4 already cites Tenzer.** That governs
how the cases below are written: a `present` arm naming either would be
satisfied before any edit. Checked against the file rather than assumed —
`norot`, `tenzer` and `kebyar` are present today; `style-dependent`, `reliable
default`, `stylistic mixing` and `in norot the relationship is effectively
reversed` are not, so those are what the cases assert.

F27's caveat on Rule 3 names the same style Rule 5 lists, so the two rules
agree — match Rule 5's spelling and italicisation rather than introducing a
variant.

**Chapter 6 carries five locks already** — `S02-F01`, `S02-F12`,
`S06-F16-indian`, `S04-F20` and `S02-F25`. `theory-gamelan.mdx` carries
`S06-F16-gamelan`. Tasks 2 and 3 run `site-unit` immediately after their edits
and name those cases, because a slice that quietly broke an earlier milestone's
lock would be the worst outcome here.

**Chapter 6's line numbers moved.** M003/S02 inserted a `:::note[Scope]`
admonition near the top, so the layakari paragraph is lower than any older note
records. Find it by its text — "maps directly to Poly's subdivision parameter" —
not by line number.

**Reverting a test mutation** while a file holds this task's uncommitted work:
use an inverse edit, not `git checkout --`. M002/S03 lost three edits that way.

## Task 1 — Chapter 2: mark the Manding same-cycle flattening

**Modifies:** `site/src/content/docs/02-sub-saharan-africa.mdx`,
`site/tests/scope-framing.test.mjs`, `docs/plans/theory-audit/ledger.md`
**Creates:** `docs/plans/theory-audit/evidence/M003-S03.md`
**Rows:** F26

### Steps

1. **Write the failing case.** Add to the `CLAIMS` array in
   `site/tests/scope-framing.test.mjs`:

   - `id: 'S03-F26'`, `file: '02-sub-saharan-africa.mdx'`
   - `rule`: ledger F26 — Manding dunun ensembles do use distinct cycle lengths
     in many contexts, so the chapter's shared-cycle presentation is a
     pedagogical flattening rather than an error, and is marked as one; Charry
     (2000) is the authority that corrects it
   - `present`: `['pedagogical simplification', 'Charry', 'distinct cycle lengths']`

2. **Run it and watch it fail on the present side** —
   `node --test site/tests/scope-framing.test.mjs` reports
   `corrective phrase went missing`. There is no forbidden arm: the existing
   sentence is not wrong and is not being removed, only qualified.

3. **Add the disclosure** to `02-sub-saharan-africa.mdx`, immediately after the
   paragraph beginning "West of the Ewe heartland". It must say that the
   shared-cycle presentation is a pedagogical simplification; that Manding dunun
   ensembles do in many contexts use distinct cycle lengths; and that Charry
   (2000), *Mande Music*, is the authority on that. Name Charry as plain text —
   **no anchor link**, per the Context note above.

4. **Run the case and watch it pass**, then run `site-unit` and confirm the
   chapter's other cases are unaffected.

5. **Run** `format`, `site-unit` and `doc-conformance` with everything staged,
   and read each exit code.

6. **Close F26** in the ledger: `Status` to `done`, name case `S03-F26` in its
   `Verification` cell, correct its `Lands in` cell, and record in the `Item`
   cell that Charry is named unlinked pending F44. Use no `|` in any cell.

7. **Append to** `docs/plans/theory-audit/evidence/M003-S03.md`, creating it.
   Name no commit SHA.

8. **Commit** with trailers `Plan: docs/plans/theory-audit/ledger.md`,
   `Slice: M003/S03`, `Rows: F26`.

## Task 2 — Gamelan Rules 3, 4 and 5: style-dependence, and lock Rule 4

**Modifies:** `site/src/content/docs/theory-gamelan.mdx`,
`site/tests/scope-framing.test.mjs`, `docs/plans/theory-audit/ledger.md`,
`docs/plans/theory-audit/evidence/M003-S03.md`
**Rows:** F27, F29, F30

Three rows in one file, and they interact: Rule 3's caveat names the same style
Rule 5 lists, and Rule 4 sits between them untouched but locked.

### Steps

1. **Write three failing cases** in `scope-framing.test.mjs`, all with
   `file: 'theory-gamelan.mdx'`:

   - `id: 'S03-F27'` — `rule`: ledger F27, Rule 3 presents the polos-onbeat /
     sangsih-offbeat division as general when in *norot* it is effectively
     reversed. `present`:
     `['style-dependent', 'in norot the relationship is effectively reversed']`.
     Do **not** assert bare `'norot'` — Rule 5 already names the style, so that
     arm would be satisfied before the edit and prove nothing about Rule 3.
   - `id: 'S03-F30'` — `rule`: ledger F30, Rule 5's absolute prohibition is
     stronger than Tenzer, who allows stylistic mixing within a kebyar
     performance. `forbidden`: `['mixing interlock styles mid-phrase is not idiomatic']`.
     `present`: `['reliable default', 'stylistic mixing']`.
     Do **not** assert bare `'Tenzer'` or the `#fr-tenzer-2000` anchor — Rule 4
     already carries both, so either arm would pass before the edit.
   - `id: 'S03-F29'` — `rule`: ledger F29, Rule 4's strict-complementation
     honesty already exists and this case exists to stop a later edit dropping
     it. `present`: `['Strict complementation is only the textbook case',
     'the overlap marks structure']`, `presentRegex`: `[/#fr-tenzer-2000/]`.

2. **Run them and read which fail.** `S03-F27` and `S03-F30` must fail —
   `S03-F30` on its forbidden arm, since the absolute phrasing is still there.
   **`S03-F29` will pass immediately**, because Rule 4 is already honest. That
   is expected and is not a mistake; step 6 proves it is not vacuous.

3. **Edit Rule 3.** Extend it with a caveat: the onbeat/offbeat division is
   style-dependent rather than a general law, and in *norot* the relationship is
   effectively reversed. Match Rule 5's spelling and italicisation of *norot*.

4. **Edit Rule 5.** Replace "choose one and keep it; mixing interlock styles
   mid-phrase is not idiomatic" with phrasing that keeps the practical advice
   while matching Tenzer: choosing one style and staying with it is the reliable
   default, and Tenzer documents stylistic mixing within a single kebyar
   performance, so the prohibition is a starting discipline rather than a rule
   of the tradition. Cite Tenzer with the existing anchor
   `([Tenzer 2000](/appendix-references/#fr-tenzer-2000))`, which Rule 4 already
   uses and which therefore resolves.

5. **Run the cases and watch all three pass**, then **run `site-unit`
   immediately** and confirm `S06-F16-gamelan` is among the passing cases — it
   is M001's lock on this file.

6. **Prove `S03-F29` is not vacuous.** Delete the sentence "Strict
   complementation is only the textbook case." from Rule 4, confirm `S03-F29`
   fails naming that missing phrase, then restore it **with an inverse edit, not
   `git checkout --`**. A case written against text that already exists is
   worthless until it has been seen to fail.

7. **Run** `format`, `site-unit` and `doc-conformance` staged, and read each
   exit code.

8. **Close F27, F29 and F30** in the ledger: each `Status` to `done`, each
   `Verification` naming its case, each `Lands in` corrected to
   `theory-gamelan.mdx`. Record in F29's `Item` cell that the honesty was
   already present and this row added only the lock. Use no `|` in any cell.

9. **Append the evidence**, then **commit** with trailers
   `Plan: docs/plans/theory-audit/ledger.md`, `Slice: M003/S03`,
   `Rows: F27, F29, F30`.

## Task 3 — Chapter 6: layakari is not a subdivision change

**Modifies:** `site/src/content/docs/06-indian-classical.mdx`,
`site/tests/scope-framing.test.mjs`, `docs/plans/theory-audit/ledger.md`,
`docs/plans/theory-audit/M003-S03-plan.md`,
`docs/plans/theory-audit/evidence/M003-S03.md`
**Rows:** F28

### Steps

1. **Write the failing case.** Add to `CLAIMS`:

   - `id: 'S03-F28'`, `file: '06-indian-classical.mdx'`
   - `rule`: ledger F28 — true layakari performs the same compositional phrase
     at 2× or 3× speed, while changing a lane's subdivision changes hit density;
     the mapping is a useful Poly workflow and a conceptual simplification, and
     the chapter now says so
   - `present`: `['hit density', 'the same phrase', 'simplification']`

2. **Run it and watch it fail on the present side.** No forbidden arm: the
   existing mapping is a legitimate workflow and stays.

3. **Add the disclosure** to `06-indian-classical.mdx`, immediately after the
   paragraph containing "maps directly to Poly's subdivision parameter". Find
   that paragraph **by its text, not by line number** — M003/S02 inserted a
   Scope note above it. The disclosure must say that true layakari is the same
   phrase performed at a multiple of the base speed, that changing a lane's
   subdivision instead changes how many hits fall in the cycle, and that the
   mapping is a practical approximation rather than the thing itself.

4. **Run the case and watch it pass**, then **run `site-unit` immediately** and
   confirm `S02-F01`, `S02-F12`, `S06-F16-indian`, `S04-F20` and `S02-F25` are
   all among the passing cases — five earlier locks live in this chapter.

5. **Run `site-unit` and `doc-conformance`** staged, and read their exit codes.

6. **Close F28 and the slice.** Set F28 `done`, name case `S03-F28` in its
   `Verification`, correct its `Lands in`. Then tick all five
   Definition-of-Done boxes in the slice and in this plan's copy, tick this
   plan's Task 3 box, and set the slice `Status` to `done`.

7. **Append the evidence**, then run `format` last per the gate ordering, and
   `jk-standards ledger`.

8. **Commit** as one unit with trailers
   `Plan: docs/plans/theory-audit/ledger.md`, `Slice: M003/S03`, `Rows: F28`.
