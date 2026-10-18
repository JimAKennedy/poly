---
class: gated
---

# M005/S01 — Appendices

**Slice:** central-africa M005/S01 (rows CA35–CA37)
**Ledger:** `docs/plans/central-africa/ledger.md`
**Decisions consumed:** `M005-decisions.md` (2026-10-18)

## Task status

- [x] 1. The Euclidean Reference has a 24-step section and the bòndò row
- [x] 2. The Timing Model explains composite subdivision
- [x] 3. Chapter 1 says co-prime cycles are Poly's, not the Central African norm

## Definition of Done

Copied verbatim from the slice:

- [x] The Euclidean Reference has a 24-step section with E(13,24) and the five
      mould rotations, and names bòndò's dì.kétɔ̀ as E(7,12) at rotation 0
- [x] The generator data no longer describes the gankogui bell at a rotation
      the table does not show
- [x] The Timing Model explains composite subdivision as 1/16 with Tempo Mult
      1.25
- [x] Chapter 1's Cycle Independence says co-prime cycles are not the Central
      African norm and links to the chapter

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |

`generated-freshness`, which CA35's verification names, runs inside
`doc-discipline` (`jk-standards all`), against the `generated:` entry for
`appendix-euclidean-reference.mdx` in `jk-standards.yaml`.

## Reference values

`rotateRight(bjorklund(24, 13), r)` from `site/src/audio/bjorklund.ts`
(re-exported by `site/src/lib/euclidean-claims.mjs` as `rotate`), checked on
2026-10-18. Rotation 4 equals the ngbɔ̀lù dì.kétɔ̀ grid printed in Chapter 2b.

| Rotation | Formulae (Chapter 2b mould table) | Gaps |
|---|---|---|
| 4 | ngbɔ̀lù | 2+2+2+1+2+2+2+2+2+1+2+2+2 |
| 22 | mò.nzòlì | 1+2+2+2+2+2+1+2+2+2+2+2+2 |
| 19 | yómbè, zɔ̀bɔ̀kɔ̀ | 2+2+2+2+1+2+2+2+2+2+2+1+2 |
| 13 | mò.kóndí | 2+1+2+2+2+2+2+2+1+2+2+2+2 |
| 10 | mò.mbénzélé | 2+2+2+2+2+2+1+2+2+2+2+2+1 |

E(7,12): rotation 0 is `x . x x . x . x x . x .` (2+1+2+2+1+2+2), the bòndò
dì.kétɔ̀; rotation 2 is `x . x . x x . x . x x .` (2+2+1+2+2+1+2), the
appendix's gankogui row; rotation 1 reads 2+1+2+2+1+2+2 from its first onset,
so the fixture note's "rotation 1 for grouping 2+2+1+2+2+1+2" is wrong.

The dì.kétɔ̀ lane in `site/src/generated/presets.json` (category
`Central African`) carries `cycleSteps: 24`, `hits: 13` and
these rotations: Aka: Ngbolu 4, Aka: Monzoli 22, Aka: Yombe 19, Aka: Zoboko 19,
Aka: Mokondi 13, Aka: Mombenzele 10. Aka: Bondo's dì.kétɔ̀ lane is `hits: 7`,
rotation 0.

## Task 1 — The Euclidean Reference has a 24-step section and the bòndò row

Rows: CA35.

**Files**

- Create `site/tests/appendices-catch-up.test.mjs`
- Modify `site/src/data/euclidean-appendix.json`
- Modify `site/src/content/docs/appendix-euclidean-reference.mdx` (new
  `## 24 Steps` section with empty `euclidean-24` markers, then regenerated)
- Modify `scripts/check-doc-conformance.sh` (add the test to `TESTS`)
- Modify `site/tests/doc-conformance-wiring.test.mjs` (add it to `REQUIRED`)
- Create `docs/plans/central-africa/evidence/M005-S01.md`

**Consumes:** nothing. **Produces:** the test file Tasks 2 and 3 extend.

**Steps**

1. Write the test file with a header comment naming the slice and rows, in the
   style of `chapter-2-reconciled.test.mjs`. Import `presets.json`,
   `euclidean-appendix.json`, `bjorklund` and `rotateRight` from
   `../src/audio/bjorklund.ts`, and `registerClaimTests` from
   `./helpers/prose-claims.mjs`. Add these cases:
   - `CA35: the fixture has a 24-step section whose rows are E(13,24) at the
     five mould rotations`: find `sections` entry `n === 24`; assert every row
     has `k === 13`, and the set of its `rotation` values equals the set of
     Rotation cells parsed from the `| … | N pulsations | R |` rows under
     `## The mould` in `02b-central-africa.mdx`.
   - `CA35: each mould rotation is what the presets' dì.kétɔ̀ lanes play`:
     for every preset in `presets.json` whose `category` is `Central African`
     and that has a lane with `cycleSteps === 24` and `hits === 13`, assert that
     lane's `rotation` is in the fixture's 24-step rotation set.
   - `CA35: every generated 24-step row is bjorklund at its rotation`: parse
     the rows between the `euclidean-24` markers of the appendix (the
     `| E(k,n) | \`pattern\` | grouping |` shape); for each, assert the
     pattern equals `rotateRight(bjorklund(24, 13), r)` rendered as `x`/`.`
     for the fixture row in the same order, and the grouping equals its gap
     sequence joined with `+`.
   - `CA35: the 12-step section names bòndò's dì.kétɔ̀ as E(7,12) rotation 0`:
     the fixture's `n === 12` section has a row with `k === 7`, no rotation or
     rotation 0, whose `association` contains `bòndò`; the appendix's
     generated 12-step region has a row `| E(7,12) | \`x . x x . x . x x . x .\``;
     and the Aka: Bondo preset's `hits === 7` lane has rotation 0.
   - `CA35: the fixture note describes the gankogui at the rotation the table
     prints`: read the gankogui row's `rotation` from the fixture (k 7, n 12,
     association containing "gankogui"); assert `$note` contains
     `` `gankogui bell at rotation ${rotation}` `` and does not contain
     `gankogui bell at rotation 1`.
   - A prose claim through `registerClaimTests` for the appendix: `present:
     ['/02b-central-africa/#the-mould']`.
2. Run `node --test site/tests/appendices-catch-up.test.mjs` and watch every
   CA35 case fail: there is no 24-step section, no bòndò row, the note says
   rotation 1, and the appendix has no link to 2b.
3. Edit the fixture:
   - In `$note`, change "E(7,12) Ewe gankogui bell at rotation 1" to
     "E(7,12) Ewe gankogui bell at rotation 2".
   - In the `n: 12` section, after the gankogui row, add
     `{ "k": 7, "association": "Aka bòndò dì.kétɔ̀ (Central Africa)", "chapter": "2b" }`.
   - After the `n: 16` section, add an `n: 24` section with five rows,
     `k: 13`, `chapter: "2b"`, in the mould table's order:
     rotation 4 "Aka dì.kétɔ̀ mould: ngbɔ̀lù",
     rotation 22 "Aka dì.kétɔ̀ mould: mò.nzòlì",
     rotation 19 "Aka dì.kétɔ̀ mould: yómbè, zɔ̀bɔ̀kɔ̀",
     rotation 13 "Aka dì.kétɔ̀ mould: mò.kóndí",
     rotation 10 "Aka dì.kétɔ̀ mould: mò.mbénzélé".
4. Edit the appendix:
   - After the 12-step section's E(5,12) paragraph, add one paragraph: the
     same seven onsets at rotation 0 are the dì.kétɔ̀ of the Aka bòndò, where
     Arom reads them not as a timeline but as a split of the oddity model the
     drums play; link `[Chapter 2b](/02b-central-africa/)`.
   - After the 16-step section and before `## Further Reading`, add
     `## 24 Steps` with empty `{/* BEGIN GENERATED: euclidean-24 */}` and
     `{/* END GENERATED: euclidean-24 */}` markers, then
     `<EuclideanDiagram steps={24} hits={13} rotation={4} />`, then a
     paragraph: E(13,24) is the Aka dì.kétɔ̀ figure, which never changes inside
     and "works like a mould" (Arom p. 298), placed at a different offset in
     each formula, so each row is one Rotation of one lane; link
     `[the mould](/02b-central-africa/#the-mould)` in Chapter 2b.
5. Run `node scripts/generate-euclidean-appendix.mjs` and confirm it reports
   `euclidean-24 (5 rows)` and `euclidean-12 (6 rows)`.
6. Run the test and watch it pass. Mutate: change the fixture's rotation 22
   row to 21, regenerate, run, watch the mould-set and preset cases fail;
   restore and regenerate. Record the red runs in the evidence file.
7. Add the test to `TESTS` in `scripts/check-doc-conformance.sh` after
   `chapter-2-reconciled.test.mjs`, and to `REQUIRED` in
   `doc-conformance-wiring.test.mjs` with a comment naming central-africa
   M005/S01.
8. Run format, site-unit, doc-conformance and doc-discipline (which runs
   generated-freshness). Write the evidence. Tick this task. Commit with
   `Rows: CA35`.

## Task 2 — The Timing Model explains composite subdivision

Rows: CA36.

**Files**

- Modify `site/tests/appendices-catch-up.test.mjs`
- Modify `site/src/content/docs/appendix-timing-model.mdx`
- Append to `docs/plans/central-africa/evidence/M005-S01.md`

**Consumes:** the test file from Task 1. **Produces:** nothing later tasks use.

**Steps**

1. Add cases:
   - `CA36: the Timing Model has a composite-subdivision section`: the page has
     a `## Composite subdivision` heading and the strings `1/16`,
     `Tempo Mult 1.25`, `0.25 ÷ 1.25 = 0.2` and `/02b-central-africa/`.
   - `CA36: the worked arithmetic holds`: from the page text, parse the
     `0.25 ÷ 1.25 = 0.2` expression with a regex capturing the three numbers;
     assert `4 / 16 === a` (the page's own `stepPpq = 4.0 / subdivision`
     rule), `a / b === c` within 1e-12, and `1 / c === 5` (five steps per
     quarter). Also parse the page's stated step count per quarter ("five
     steps") and assert it equals `1 / c`.
2. Run the test and watch both CA36 cases fail: the section does not exist.
3. Edit the page: after `## The one true speed knob: \`Tempo Mult\`` and before
   `## Additive (aksak) cell mode`, add `## Composite subdivision`. It says:
   the Subdivision values 1/4, 1/8, 1/12 and 1/16, give one,
   two, three or four steps to a quarter note, but some music divides a beat into five, Arom's composite
   subdivision. Set the lane to 1/16 and Tempo Mult to 1.25:
   `stepPpq = 4.0 / 16 = 0.25`, and `0.25 ÷ 1.25 = 0.2` quarter notes, so
   five steps fill one quarter note and a 10-step lane spans two beats. The
   Banda-Linda: Aga-torumo preset does this; link
   `[Chapter 2b](/02b-central-africa/)`. Keep the step-count arithmetic in a
   `text` block or table consistent with the section's existing formulas.
4. Run the test and watch it pass. Mutate: change `0.2` to `0.25` in the
   expression, run, watch the arithmetic case fail; restore. Record it.
5. Run all four tokens. Append the evidence. Tick this task. Commit with
   `Rows: CA36`.

## Task 3 — Chapter 1 says co-prime cycles are Poly's, not the Central African norm

Rows: CA37.

**Files**

- Modify `site/tests/appendices-catch-up.test.mjs`
- Modify `site/src/content/docs/01-foundations.mdx`
- Append to `docs/plans/central-africa/evidence/M005-S01.md`
- Modify `docs/plans/central-africa/M005-S01-plan.md` (tick the DoD boxes)
- Modify `docs/plans/central-africa/ledger.md` (rows and slice status)

**Consumes:** the test file from Task 1. **Produces:** the slice's close.

**Steps**

1. Add a prose claim and a structural case:
   - `CA37` prose claim on `01-foundations.mdx`: `forbidden: ['exactly as it
     does in the interlocking percussion ensembles of West Africa and
     Southeast Asia']`; `present: ['/02b-central-africa/',
     '/appendix-references/#fr-arom-1991', 'p. 277']`.
   - `CA37: the Chapter 1 aside sits in Cycle Independence and its link
     resolves`: slice the page from `## Cycle Independence` to the next `## `;
     assert the slice contains `/02b-central-africa/` and `p. 277`, and that
     `site/src/content/docs/02b-central-africa.mdx` exists.
2. Run the test and watch both fail: the clause is present, and the section has
   no link or page.
3. Edit the page's Cycle Independence:
   - Replace "Polymetric interaction emerges automatically from independent
     cycle lengths, exactly as it does in the interlocking percussion
     ensembles of West Africa and Southeast Asia." with a sentence saying the
     interaction emerges automatically from independent cycle lengths, and
     that this layering of co-prime cycles is Poly's own default rather than
     a description of any tradition.
   - Add an aside after that paragraph: in the Central African strict
     polyrhythm Simha Arom transcribed, parts differ in length only in simple
     ratios such as 2:1 or 3:2, and the formula's period is that of its
     longest figure ([Arom 1991](/appendix-references/#fr-arom-1991),
     p. 277); co-prime lengths such as 7 against 12 do not belong to that
     idiom. Link `[Chapter 2b](/02b-central-africa/)` for building in it, and
     note that Chapter 2 gives the Ewe case from Locke.
   - Check `prose-conformance-claims.test.mjs` S03-F1/F2,
     `theory-audit-claims.test.mjs` S06-F16-foundations and
     `literature-enrichment.test.mjs` B01 still pass: none pins the replaced
     sentence.
4. Run the test and watch it pass. Mutate: restore the old clause, run, watch
   CA37 fail on the forbidden phrase; restore. Record it.
5. Tick the four DoD boxes in this plan and in the ledger slice; set CA35,
   CA36 and CA37 to `done` and the slice to `done`.
6. Run all four tokens and `jk-standards ledger`. Append the evidence. Tick
   this task. Commit with `Rows: CA37`.

## Self-review

- **DoD coverage.** 24-step section and bòndò row → Task 1 (steps 3–5).
  Fixture note → Task 1 (step 3). Composite subdivision → Task 2. Chapter 1
  aside and link → Task 3.
- **Row coverage.** CA35 → Task 1; generated-freshness via doc-discipline in
  step 8, the bjorklund check in step 1's third case. CA36 → Task 2; the
  arithmetic case in step 1. CA37 → Task 3; the link-resolves case and the
  `p. 277` citation in step 1.
- **Placeholders.** None: every row value and rotation is given above.
- **Names.** `appendices-catch-up.test.mjs`, `euclidean-24`, `rotateRight`,
  `## Composite subdivision`, `## The mould` are used identically throughout.
