---
class: gated
---

# M004 — Chapter 2 says what the tradition does

> Chapter 2 describes West African practice only as far as its cited
> sources support, presents co-prime layering as Poly's own, and agrees with the
> Central Africa chapter.

Ledger: `docs/plans/central-africa/ledger.md` · branch
`milestone/M004-chapter-2-reconciled` · 6 commits including this report, **none untraced**

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M004/S01 | Chapter 2 reconciled | CA29–CA34 | done |

## What changed

- **Scope (CA29, CA34).** The description and opening say West Africa. The
  opening describes one bell-defined cycle rather than "independent rhythmic
  cycles — each with its own length". A bridge paragraph sends the Aka, Banda
  and Zande formulae to [Chapter 2b](/02b-central-africa/).
- **Polymetric Stacking (CA30).** Locke 1982 was read from the owner's
  archive, as page images, before the section was written. It supports a
  simple-ratio account only: the bell sets the period (p. 217), the kagan
  cycles 4 times and the kidi twice per bell pattern (p. 219), and the common
  cross-rhythms are 3:2 and 3:4, with 3:8 sometimes (p. 244). The section
  states that, then presents 12/7/5 layering, 420 pulses before it repeats,
  as Poly's extension. Two other places made the same uncited claim, and both
  were reworded: the Manding comparison ("Where Ewe drumming stacks many
  independent cycles") and the closing paragraph.
- **No downbeat (CA31).** The gankogui paragraph now describes the bell
  against Locke's four beats of three pulses (pp. 224–225), not "pulling
  forward toward the next downbeat". Listen for calls the convergences an
  effect of Poly's layering. The word "downbeat" is locked out of the page.
- **The patch table (CA32).** It named "Sub-Saharan: Agbekor" and matched it
  on one lane. It now names "Ewe Polymetric Ensemble", the preset its card
  plays, and is that preset's four lanes, compared cell by cell with
  `presets.json`. The bell's Ghost cell moves from 0 to 30, the preset's
  value. The dance beat theory-audit M004 had inserted as lane 2 is now an
  instruction to add a fifth lane, which `ssa-dance-beat` accepts. The prose's
  "Lane 4 uses the Kotekan setting `L1`" is now true.
- **Three rotations (CA33).** One sentence names the diagram's rotation 0,
  the appendix's rotation 2 and the Agbekor preset bell's rotation 9, each
  with its gap sequence. It cites Locke on the bell having no fixed beginning
  (p. 225).
- **Tests.** `site/tests/chapter-2-reconciled.test.mjs` holds 9 cases, each
  red before its edit and red again under a recorded mutation. It is wired
  into `scripts/check-doc-conformance.sh` and into `REQUIRED` in
  `doc-conformance-wiring.test.mjs`. The site suite went from 422 tests to
  431.

## Definition of done

- [x] Chapter 2's description and opening speak of West Africa, not West and
      Central Africa
- [x] Polymetric Stacking claims nothing about Ewe cycle lengths that Locke
      1982 does not support, and presents 12/7/5 layering as Poly's extension
- [x] No passage in Chapter 2 asserts a downbeat as part of the tradition
- [x] The `PolyPatch` table matches the preset it names, and the prose names
      the lane that actually carries the Kotekan setting
- [x] A reader can see that the chapter's diagram, the Euclidean appendix and
      the Agbekor bell are one rhythm at three rotations
- [x] Chapter 2 links to the Central Africa chapter for Central African music

## Validation

From `evidence/M004-S01.md`, on the slice's last task:

| Token | Command | Result |
|---|---|---|
| `format` | `pre-commit run --all-files` | exit 0 |
| `site-unit` | `npm --prefix site test` | exit 0, 431 tests, 0 failures |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | exit 0, 333 tests, 0 failures |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` | exit 0 |

## Look at twice

- **`theory-patch-conformance.test.mjs` `ssa-dance-beat` was loosened.** It
  used to require a 12-step, 4-hit table row. It now also accepts an
  instruction inside the patch to add one. This was your decision at
  planning; it is the only existing lock this milestone weakened.
- **Two existing locks had their anchors moved**, both recorded as judgment
  calls. `literature-enrichment.test.mjs` F45 now binds the Kubik citation to
  the rewritten opening sentence. The `theory-patch-conformance` checklist key
  follows the patch's new title.
- **Prose that interprets Locke.** "Seven strokes cannot sit evenly on four
  beats, so the bell keeps landing between them as well as on them" is
  arithmetic, not a quote. "Atsiagbeko, the piece Locke draws most of his
  examples from" summarises the article, and its note 1 says the article
  comes from his Atsiagbeko dissertation.
- **Not touched:** the frontmatter `patternClaims` `reference` label says
  "line 33" for the gap-sequence sentence. That sentence was on line 30 before
  this milestone and is on line 32 now. The label was already wrong, and no
  test reads it.

## Traceability

- `e7daeb7` docs(plans): plan M004/S01, Chapter 2 reconciled — Slice: M004/S01
- `f1e3032` docs(site): Chapter 2 is about West Africa, and sends Central Africa to 2b — Slice: M004/S01; Rows: CA29, CA34
- `fd8b391` docs(site): Chapter 2's stacking says what Locke 1982 supports — Slice: M004/S01; Rows: CA30, CA31
- `0859556` docs(site): Chapter 2's patch table is the Ewe Polymetric Ensemble preset — Slice: M004/S01; Rows: CA32
- `d960a6e` docs(site): Chapter 2 says its three gankogui bells are one rhythm — Slice: M004/S01; Rows: CA33

This report's own commit carries `Slice: M004/S01`.

Untraced commits: none.

## Decisions

Verbatim from `M004-decisions.md`, with headings demoted one level.

### 2026-10-18 — planning M004/S01

- **Q:** The Chapter 2 patch table is the "Ewe Polymetric Ensemble" preset
  plus a 12/4 dance-beat lane that theory-audit M004 inserted as lane 2, which
  no preset carries, and it names "Sub-Saharan: Agbekor", which it matches on
  lane 1 only. Make it the Ewe preset exactly (dance beat becomes an
  add-a-lane instruction, `ssa-dance-beat` rewritten to require it), keep five
  lanes under a `Custom:` label, or add the lane to the preset in the engine?
  — **A:** the Ewe preset exactly
- **Q:** Approve the design: bounded; CA29 scopes the description and opening
  to West Africa; CA30 rewrites Polymetric Stacking from Locke 1982 (bell sets
  the period, kagan 4× and kidi 2× per bell pattern, cross-rhythms 3:2 and 3:4,
  sometimes 3:8) and presents 12/7/5 as Poly's extension, also rewording the
  Manding "Ewe stacks many independent cycles" sentence and the closing
  paragraph; CA31 rewrites both downbeat passages; CA33 names rotations 0, 2
  and 9 and cites Locke p. 225; CA34 links `/02b-central-africa/`? — **A:**
  approve
- **Decision:** classify M004/S01 as bounded — **Why:** prose edits to one
  existing chapter plus site tests in an existing harness; no engine,
  interface or preset change.
- **Decision:** Locke 1982 was read from the owner's archived copy, as page
  images, before planning; the quotes the plan permits are listed in the plan
  with pages — **Why:** CA30's verification requires Locke checked first, and
  `pdftotext` clips line ends on this scan, so only the images are a reliable
  reading.
- **Decision:** the Manding sentence "Where Ewe drumming stacks many
  independent cycles" and the closing paragraph are in CA30's scope — **Why:**
  both repeat the uncited cycle-length claim the DoD forbids, and the vision
  names the closing paragraph as building on it.
- **Decision:** the Agbekor preset's bell is at rotation 9 (onsets
  0,2,4,5,7,9,11), derived with `rotateRight(bjorklund(12, 7), 9)` — **Why:**
  CA33 needs a third rotation the page can name, and the preset is the
  record of what that bell plays.
- **Decision:** the chapter keeps the spelling "Ewe" and makes no claim that
  Locke's Atsiagbeko and the preset name "Agbekor" are one piece — **Why:** the
  references record already notes Eʋe/Ewe; the equivalence is not on the page
  of any source the chapter cites.
- **Decision:** the CA31 lock forbids the word "downbeat" anywhere in the
  chapter — **Why:** a narrower lock on the two phrases would let a third
  downbeat assertion in; the rewritten text has no need of the word.

### 2026-10-18 — executing M004/S01

- **Judgment call:** `literature-enrichment.test.mjs` F45 bound the Kubik
  citation to the phrase "it is the polymetric drumming of West and Central
  Africa", which CA29 removes; its `near` now names the rewritten sentence,
  and its `why` no longer says the opening claims the Central Sudanic belt —
  **Why:** the lock's purpose is that the citation stays attached to the
  opening sentence; the anchor follows the sentence, the binding is unchanged.
- **Judgment call:** `chapter-2-reconciled.test.mjs` is added to `REQUIRED`
  with its own comment rather than "beside the central-africa entries" as the
  plan says — **Why:** M003 put its two tests in the runner only, so there
  were no central-africa entries in `REQUIRED` to sit beside.
- **Judgment call:** the `theory-patch-conformance.test.mjs` checklist entry
  for Chapter 2 is keyed by patch title, so retitling the patch "Ewe
  Polymetric Ensemble" (plan Task 3 step 5) also changes that key from
  "Ewe-Inspired Polymetric Ensemble" — **Why:** the harness fails closed on a
  renamed patch; the key follows the title, and both rules still run against
  the table.
- **Judgment call:** `docs/audits/poly_theory_audit.md` still names the
  "Ewe-Inspired Polymetric Ensemble" patch and is left as it is — **Why:** it
  is a dated audit record describing the page as it was, not a live claim.
