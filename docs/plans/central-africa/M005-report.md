---
class: gated
---

# M005 — The reference appendices catch up

> The Euclidean Reference, the Timing Model and Chapter 1 carry what the
> Central Africa chapter relies on.

Ledger: `docs/plans/central-africa/ledger.md` · branch
`milestone/M005-appendices` · 5 commits including this report, **none untraced**

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M005/S01 | Appendices | CA35–CA37 | done |

## What changed

- **Euclidean Reference (CA35).** A new `24 Steps` section, generated from
  `euclidean-appendix.json`, lists E(13,24) at the five rotations of Chapter
  2b's mould table (4, 22, 19, 13, 10), each row naming its Aka formulae. It
  has a diagram at rotation 4 and a paragraph that cites Arom p. 298 ("works
  like a mould") and links to `/02b-central-africa/#the-mould`. The 12-step
  table gains E(7,12) at rotation 0 as the bòndò dì.kétɔ̀, with a paragraph
  linking to Chapter 2b. The fixture's `$note` said the gankogui bell sits at
  rotation 1. Rotation 1 does not give the grouping the note claims, and the
  row is at rotation 2, so the note now says rotation 2.
- **Timing Model (CA36).** A new `Composite subdivision` section, between
  Tempo Mult and additive cell mode. It works 1/16 at Tempo Mult 1.25 through
  as `0.25 ÷ 1.25 = 0.2` quarter notes per step, so five steps fill one
  quarter note, and a 10-step lane spans two beats. Banda-Linda: Aga-torumo is
  the example, with a link to Chapter 2b.
- **Chapter 1 (CA37).** Cycle Independence said co-prime polymetre happens
  "exactly as it does in the interlocking percussion ensembles of West Africa
  and Southeast Asia". It now says that layering is Poly's own default. A
  `:::note` cites Arom's simple-ratio rule (Arom 1991, p. 277), says co-prime
  lengths do not belong to that idiom, links to Chapter 2b, and points to
  Chapter 2's Ewe case.
- **Tests.** `site/tests/appendices-catch-up.test.mjs` holds 10 cases. Each
  was red before its edit and red again under a recorded mutation. The
  rotations are checked against `bjorklund.ts`, against Chapter 2b's mould
  table and against the dì.kétɔ̀ lanes in `presets.json`, so the appendix
  cannot drift from either. The file is wired into
  `scripts/check-doc-conformance.sh` and `REQUIRED`. The site suite went from
  431 tests to 441.

## Definition of done

- [x] The Euclidean Reference has a 24-step section with E(13,24) and the five
      mould rotations, and names bòndò's dì.kétɔ̀ as E(7,12) at rotation 0
- [x] The generator data no longer describes the gankogui bell at a rotation
      the table does not show
- [x] The Timing Model explains composite subdivision as 1/16 with Tempo Mult
      1.25
- [x] Chapter 1's Cycle Independence says co-prime cycles are not the Central
      African norm and links to the chapter

## Validation

From `evidence/M005-S01.md`, on the slice's last task:

| Token | Command | Result |
|---|---|---|
| `format` | `pre-commit run --all-files` | exit 0 |
| `site-unit` | `npm --prefix site test` | exit 0, 441 tests, 0 failures |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | exit 0, 343 tests, 0 failures |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` | exit 0, generated-freshness: 4 generated doc(s) fresh |

## Look at twice

- **CA37 went past the ledger row's wording, by your decision.** The row
  asked only for an aside. The aside would have contradicted the sentence
  beside it, so that sentence was reworded as well. "Southeast Asia" went with
  it, because no source cited on the page supports co-prime cycles in gamelan.
- **New citations of Arom on two pages.** p. 298 on the Euclidean Reference
  and p. 277 on Chapter 1. Both quote or paraphrase what Chapter 2b already
  states from those pages. Neither was re-read against the scan for this
  milestone.
- **The Chapter column now carries `2b`**, the first value in that column
  that is not a number. Nothing parses the column.
- **A presence check that cannot catch a mutation on its own.** CA36's
  presence case looks for the substring `0.25 ÷ 1.25 = 0.2`, which also
  matches `= 0.25`. The worked-arithmetic case is what went red on that
  mutation.
- **Not touched:** the `ListenFor` box in Chapter 1 still calls the 84-step
  convergence "the structural downbeat of the polymetric phrase". It speaks
  of Poly's layering, not of a tradition, so it is out of this row's scope.

## Traceability

- `e0398c3` docs(plans): plan M005/S01, the reference appendices catch up — Slice: M005/S01
- `74c7a24` docs(site): the Euclidean Reference has the dì.kétɔ̀ mould and the bòndò bell — Slice: M005/S01; Rows: CA35
- `788e115` docs(site): the Timing Model explains five steps to a beat — Slice: M005/S01; Rows: CA36
- `391148b` docs(site): Chapter 1 says co-prime cycles are Poly's, not a tradition's — Slice: M005/S01; Rows: CA37

This report's own commit carries `Slice: M005/S01`.

Untraced commits: none.

## Decisions

Verbatim from `M005-decisions.md`, with headings demoted one level.

### 2026-10-18 — planning M005/S01

- **Q:** Chapter 1's Cycle Independence says co-prime polymetre emerges
  "exactly as it does in the interlocking percussion ensembles of West Africa
  and Southeast Asia". M004 found Locke 1982 supports only simple ratios for
  Ewe drumming, and R7 says the same for Central Africa, so the CA37 aside
  would sit beside a claim it contradicts. Reword the clause in CA37, add the
  aside only and leave the clause, or halt and add a ledger row? — **A:**
  reword it in CA37
- **Q:** Approve the bounded design: CA35 adds a 24-step section to the
  Euclidean fixture with the five E(13,24) mould rotations, an E(7,12)
  rotation 0 row for the bòndò dì.kétɔ̀, and corrects the fixture note's
  gankogui rotation; CA36 adds a composite-subdivision section to the Timing
  Model; CA37 adds the Chapter 1 aside? — **A:** approve
- **Decision:** classify M005/S01 as bounded — **Why:** prose and fixture
  edits to three existing pages through the existing generator, plus site
  tests in the existing harness; no engine, interface or preset change.
- **Decision:** the fixture note's gankogui rotation becomes 2 — **Why:**
  `rotateRight(bjorklund(12, 7), 1)` has gaps 2+1+2+2+1+2+2 read from its
  first onset, not the 2+2+1+2+2+1+2 the note claims; rotation 2 gives that
  sequence, and rotation 2 is what the table row already prints.
- **Decision:** the five mould rows carry chapter `2b` in the Chapter column
  — **Why:** the column is free text to the generator and to
  `appendix-euclidean-claims.test.mjs`, and 2b is the chapter's own number.
- **Decision:** the Chapter 1 rewording drops "Southeast Asia" as well as
  "West Africa" rather than substituting another tradition — **Why:** no
  source cited on the page supports co-prime cycles in any named tradition,
  and the gamelan chapter does not claim it either; the sentence says the
  layering is Poly's own.
