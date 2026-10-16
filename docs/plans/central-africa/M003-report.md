---
class: gated
---

# M003 — The Central Africa chapter

> A published chapter teaches Arom's strict polyrhythm as rules, norms and
> eleven playable formulae, and its grids cannot disagree with the presets
> that play them.

Ledger: `docs/plans/central-africa/ledger.md` · branch
`milestone/M003-central-africa-chapter` · 8 commits, **none untraced**

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M003/S01 | The chapter's teaching text | CA02, CA19–CA24 | done |
| M003/S02 | The chapter plays | CA25–CA28 | done |

## What changed

- **A new chapter, "2b. Central Africa: Strict Polyrhythm"**
  (`site/src/content/docs/02b-central-africa.mdx`), after Chapter 2 in the
  sidebar. In the vision's order:
  - what strict polyrhythm is, and how to read box notation and a `(count)` row;
  - rules R1–R12 and norms N1–N10, each paraphrased with one quoted phrase and
    its page, and what it means in Poly;
  - the eleven formulae, each with grid, count row, lane table and a card;
  - the tempo mapping, and the mould as Rotation values with the knob-turn;
  - the build procedure, what breaks the idiom, and the book's three printing
    inconsistencies.
- **The research note's Poly errors are corrected, not carried:**
  - a lane that carries marks needs Mutation 0 and Drift Rate 0, because
    timeline mode stops only the global macros;
  - there is no per-step accent mask, so accented strokes get their own lane;
  - mò.mbénzélé's dì.kétɔ̀ sits four pulsations after mò.nzòlì's, not one.
- **`fr-arom-1991` is in the published references appendix**, under a new
  "Central Africa (Chapter 2b)" section. The page's first citation of it is
  the one FR20 needs.
- **Two new site tests, both in the doc-conformance runner:**
  - `central-africa-oddity.test.mjs` redoes the oddity table's arithmetic;
  - `central-africa-grids.test.mjs` compares every titled grid on the page
    with its preset in `presets.json`, position by position, and checks each
    count row.
- **`PRESET_BPM` has an entry for each of the eleven presets.** Each one is
  derived from the tempo printed on Arom's figure.
- **The e2e audio gate plays Aka: Ngbolu and Banda-Linda: Aga-torumo.**
- The drift map declares the page under `cannot_drift`.

## Definition of done

**M003/S01**

- [x] A published page "Central Africa: Strict Polyrhythm" sits directly after
      Chapter 2 in the sidebar
- [x] The published `appendix-references.mdx` has an `fr-arom-1991` entry,
      added with the chapter's first citation of it
- [x] The page states rules R1–R12 and norms N1–N10, each with a page
      reference, and what each means in Poly
- [x] Every quote on the page is one phrase long and was found on the page it
      cites in the scan
- [x] The page's Poly guidance tells the reader to keep Mutation and Drift at 0
      on any lane that carries marks, and says the presets are marks-only
      reductions
- [x] The build procedure, what breaks the idiom (including the global macros)
      and the book's three printing inconsistencies are on the page

**M003/S02**

- [x] Each of the eleven formulae has a section with its grid, count row, lane
      recipe and a `PolyPreviewCard` that plays its preset and opens Try it
- [x] Every card plays at a tempo taken from Arom's figure, and the page says
      how that tempo maps to the host's quarter-note BPM
- [x] The page tells the reader to open Aka: Ngbolu and turn the dì.kétɔ̀
      lane's Rotation to place the mould of each other formula
- [x] A site test fails if any grid on the page differs from the preset that
      plays it, or if any count row is not the sum of its grid

## Validation

From `evidence/M003-S01.md` and `evidence/M003-S02.md`, each on the slice's
final commit:

| Token | Command | M003/S01 | M003/S02 |
|---|---|---|---|
| format | `pre-commit run --all-files` | pass | pass |
| site-unit | `npm --prefix site test` | pass, 409 | pass, 422 |
| doc-conformance | `bash scripts/check-doc-conformance.sh` | pass | pass |
| doc-discipline | `bash scripts/check-doc-discipline.sh` | pass | pass |
| e2e | `bash scripts/site-verify-local.sh` | — | pass, all 13 stages |

Each new test was seen to fail first, and was mutation-tested against the page:
- the oddity test caught a reordered bòndò cell sequence;
- the grid test caught one flipped ngúé mark and one wrong count digit.

## Look at twice

- **Quotes were checked by eye, not OCR.** No OCR tool is installed. Each of
  the 30 quoted phrases was found on its cited page by reading rendered scan
  pages, rotated where they are printed upside down. The evidence lists each
  phrase with what was seen.
- **Three tempi are judgment calls, all recorded:**
  - Aka: Ngbolu Drums takes ex. 104's tempo, because ex. 90 has none (your
    answer).
  - Àgà-tɔ̀rúmɔ̃ reads ex. 100's faint first note as a dotted quaver, which
    ex. 123 confirms.
  - Zɔ̀bɔ̀kɔ̀ follows ex. 115's dotted crotchet over p. 297's undotted prose.
- **Host tempi run high, from 207 to 240 BPM for the Aka formulae.** That is
  the mapping working as stated, not an error.
- **The chapter is "2b", by your choice.** `counts.json` counts only
  two-digit chapter files, so its chapter count stays 18.
- **"Arom calls the differing ngúé parts the only means of distinguishing
  them" (p. 291) is quoted as he wrote it.** The page also says the dì.kétɔ̀
  mould sits at a different offset in each, per p. 298.
- **Chapter 2 still frames Central Africa as it did.** M004 reconciles it.

## Traceability

Commits are listed by subject, not SHA: the branch is squash-merged, and a
branch SHA in a durable document stops resolving once it is.

| Commit | Slice | Rows |
|---|---|---|
| docs(plans): plan central-africa M003 | M003/S01 | — |
| docs(site): a Central Africa chapter that teaches box notation | M003/S01 | CA02, CA19, CA20 |
| docs(site): Arom's twelve rules and ten norms, with their Poly settings | M003/S01 | CA21, CA22 |
| docs(site): build a formula, what breaks it, and the book's three misprints | M003/S01 | CA23, CA24 |
| test(site): chapter 2b's grids are what their presets play | M003/S02 | CA25, CA28 |
| docs(site): the eleven formulae play, at Arom's tempi, and the mould turns | M003/S02 | CA25, CA26, CA27 |
| test(e2e): the audio gate plays two Central African cards | M003/S02 | CA25 |
| docs(plans): write the M003 report | M003/S02 | — |

Untraced commits: none.

## Decisions

The contents of `M003-decisions.md`, verbatim:


## 2026-10-16 — planning M003/S01 and M003/S02

- **Q:** The chapter goes directly after Chapter 2, and every chapter title
  carries its number. Number it "2b." with no renumbering, renumber chapters
  3–18, or leave it unnumbered? — **A:** "2b." — no renumber
- **Q:** Ex. 90 (ngbɔ̀lù drums alone, p. 281) has no tempo marking. Use ex.
  104's ♩. = 144, or leave the card at the site's default? — **A:** use ex.
  104's ♩. = 144
- **Q:** Should a copy of the owner's research note be committed to the repo?
  — **A:** no; the chapter cites Arom only, and every quote and grid is
  re-checked against the scan
- **Q:** Approve the design: both slices bounded; one page,
  `02b-central-africa.mdx`, in the vision's CA3 order; every quote checked
  against the scan image; an oddity-cell test in S01; eleven formula sections
  with cards, grids tagged by preset name, and a grid-against-`presets.json`
  test in S02; host BPM = pulsation tempo × values per pulsation × step length
  in beats? — **A:** yes
- **Decision:** host quarter-note BPM = Arom's pulsation tempo × values per
  pulsation × the step length in beats — **Why:** a pulsation is that many
  Poly steps, so the host must run that many beats per pulsation; for the Aka
  formulae at 1/8 the factor is 1.5, for gàzà (8 values at 1/16) 2, for
  kpóníngbó (4 at 1/16) 1, for àgà-tɔ̀rúmɔ̃ (5 at 1/16 with Tempo Mult 1.25) 1
- **Decision:** àgà-tɔ̀rúmɔ̃ takes ex. 100's marking (two tied quavers = 110,
  p. 285) and zɔ̀bɔ̀kɔ̀ takes ex. 115's ♩. = 160 (p. 294) — **Why:** ex. 99 and
  the zɔ̀bɔ̀kɔ̀ grids carry no marking; exx. 100 and 115 are the same music in
  staff notation, read from the scan on 2026-10-16
- **Decision:** quotes are checked by reading rendered scan pages, rotated
  where printed upside down — **Why:** no OCR tool is installed; the note's
  own OCR check is the owner's, and the DoD asks that each quote be found on
  its page in the scan

## 2026-10-16 — judgment calls in M003/S01 and M003/S02 task 1

- **Decision:** the planning entry's "two tied quavers = 110" for ex. 100 is
  corrected to a dotted quaver tied to a quaver = 110, and bòndò's ex. 102 is
  on p. 287, not p. 286 — **Why:** a close reading of the scan at 1200 dpi
  found a faint dot on the first note, matching ex. 123 (p. 301), and found
  ex. 102 on p. 287; the àgà marking is still five values to one pulsation,
  and neither page change moves a BPM
- **Decision:** zɔ̀bɔ̀kɔ̀ takes ex. 115's ♩. = 160, not p. 297's prose "♩ = 160"
  — **Why:** the zɔ̀bɔ̀kɔ̀ subdivision is ternary (p. 297 itself), so the
  pulsation is a dotted crotchet; the undotted prose is a misprint, and the
  page says which it follows
- **Decision:** the formulae sit between the norms and the build procedure —
  **Why:** the vision's CA3 order puts the formulae (item 5) and the mould
  (item 6) before the procedure (item 7)

## 2026-10-16 — judgment calls in M003/S02 task 2

- **Decision:** the page says mò.mbénzélé's dì.kétɔ̀ sits four pulsations after
  mò.nzòlì's, where the research note says one — **Why:** p. 298's offsets
  (2 and 6 pulsations) and the presets' rotations (22 and 10) both give four
- **Decision:** the lane tables are read from presets.json, so the bòndò
  rattle and the kpóníngbó pellet bells appear as drawn lanes — **Why:** the
  page describes the preset the card plays; M002 built those two lanes drawn
