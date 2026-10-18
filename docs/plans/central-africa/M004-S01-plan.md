---
class: gated
---

# M004/S01 — Chapter 2 reconciled

**Slice:** central-africa M004/S01 (rows CA29–CA34)
**Ledger:** `docs/plans/central-africa/ledger.md`
**Decisions consumed:** `M004-decisions.md` (2026-10-18)

## Task status

- [x] 1. Chapter 2 is scoped to West Africa and sends Central Africa onward
- [x] 2. Polymetric Stacking says what Locke supports, and no downbeat is asserted
- [x] 3. The patch table is the preset it names
- [x] 4. One rhythm at three rotations

## Definition of Done

Copied verbatim from the slice:

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

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |

## What Locke 1982 supports

Read from the owner's archived copy (`FR - Locke 1982.pdf`, page images, on
2026-10-18). These are the only claims about Ewe cycle lengths the chapter may
make, each with its page:

- p. 217: "Not only is the basic musical period established by the bell
  pattern but its distinctive rhythmic shape influences all aspects of the
  music and dance."
- p. 219 (Atsiagbeko): "The gankogui, axatse, totodzi, and kloboto patterns
  share the same phrase length and begin together." "The high-pitched kagan
  must cycle 4 times per bell pattern." "The kidi must cycle twice per bell
  pattern".
- p. 233: "the musical period of a cross-rhythm may be shorter than the time
  span of the bell pattern, making it possible for several cycles of
  cross-rhythm to be imbedded within one measure."
- p. 244: "Two cross-rhythms are common in southern Eve dance drumming, 3:2
  and 3:4. Another cross-rhythm sometimes occurs, 3:8."
- p. 225: "the continuous repetition of this pattern creates a cyclical
  effect, which makes the designation of a beginning point somewhat
  inappropriate, and indeed, field research showed that the pattern may be
  launched in various ways."

Locke spells the people Eʋe and the dance Atsiagbeko. The chapter keeps
"Ewe", the usual English spelling, as the references record already notes.
It does not claim Atsiagbeko and the preset name "Agbekor" are the same
piece, since no source on the page says so.

Locke supports a simple-ratio account: every part's period divides the bell's.
Nothing in it supports parts of 7 or 5 pulses against a 12-pulse bell.

## The three rotations

`rotateRight(bjorklund(12, 7), r)` from `site/src/audio/bjorklund.ts`:

| Where | Rotation | Pattern | Gaps |
|---|---|---|---|
| Chapter 2's diagram | 0 | `x . x x . x . x x . x .` | 2-1-2-2-1-2-2 |
| Euclidean appendix (`euclidean-appendix.json`, row k 7 n 12) | 2 | `x . x . x x . x . x x .` | 2-2-1-2-2-1-2 |
| `Sub-Saharan: Agbekor` lane 1 (`onsets` 0,2,4,5,7,9,11) | 9 | `x . x . x x . x . x . x` | 2-2-1-2-2-2-1 |

## The test file

All four tasks extend one new file, `site/tests/chapter-2-reconciled.test.mjs`.
It reads `site/src/content/docs/02-sub-saharan-africa.mdx` and registers its
prose cases with `registerClaimTests` from `site/tests/helpers/prose-claims.mjs`
(`forbidden`, `forbiddenRegex`, `present` arms, each case naming its row in
`id` and its authority in `rule`). Its structural cases are plain `node:test`
tests in the same file.

## Task 1 — Chapter 2 is scoped to West Africa and sends Central Africa onward

Rows: CA29, CA34.

**Files**

- Create `site/tests/chapter-2-reconciled.test.mjs`
- Modify `site/src/content/docs/02-sub-saharan-africa.mdx`
- Modify `.github/docs-drift-map.yml` (the chapter's `reason`)
- Modify `scripts/check-doc-conformance.sh` (add the test to `TESTS`)
- Modify `site/tests/doc-conformance-wiring.test.mjs` (add it to `REQUIRED`)
- Create `docs/plans/central-africa/evidence/M004-S01.md`

**Steps**

1. Write the test with two prose cases and one structural case:
   - `CA29`: `forbidden: ['West and Central Africa', 'West and Central African']`;
     `present: ['West African drumming']` (the description) and
     `present: ['West Africa']`.
   - `CA34`: `present: ['/02b-central-africa/']`.
   - `CA34: the link resolves`: for every `](/<slug>/` link on the page whose
     slug is `02b-central-africa`, assert
     `site/src/content/docs/<slug>.mdx` exists. Starlight routes a page by its
     file name, so this is the build's own resolution.
2. Run `node --test site/tests/chapter-2-reconciled.test.mjs` and watch all three
   fail: the description says "West and Central African drumming", and no
   link to the new chapter exists.
3. Edit the page:
   - `description`: "The rhythmic foundations of West African drumming —
     timeline patterns, interlocking parts, and the polymetric layering Poly
     adds to them." (the layering is Poly's, per Task 2).
   - Opening paragraph: the tradition is "the drumming of West Africa"; Kubik
     1999 "traces rhythmic practice across this region". The sentence "The
     principle of layering independent rhythmic cycles — each with its own
     length…" becomes a statement about the bell-defined cycle against which
     every part is placed. The Jones and Locke citations stay word for word:
     `theory-audit-claims.test.mjs` S05-F07 pins them.
   - Add a bridge paragraph at the end of the opening: the chapter is about
     West Africa, the Ewe of southern Ghana and the Manding; the strict
     polyrhythm of the Aka, Banda and Zande of Central Africa follows other
     rules and has its own chapter, linked as
     `[Chapter 2b](/02b-central-africa/)`.
4. Run the test and watch it pass. Mutate: restore "West and Central African"
   in the description, run, watch CA29 fail naming the forbidden phrase;
   restore. Record the red runs in the evidence file.
5. Change the drift-map `reason` to "Musicology of West African bell patterns;
   the polymetric layering is Poly's. No repo source surface."
6. Add the test to `TESTS` in the runner and to `REQUIRED` in the wiring test,
   beside the central-africa entries.
7. Run format, site-unit, doc-conformance and doc-discipline. Write the
   evidence. Commit with `Rows: CA29, CA34`.

## Task 2 — Polymetric Stacking says what Locke supports, and no downbeat is asserted

Rows: CA30, CA31.

**Files**

- Modify `site/tests/chapter-2-reconciled.test.mjs`
- Modify `site/src/content/docs/02-sub-saharan-africa.mdx`
- Modify `docs/plans/central-africa/evidence/M004-S01.md`

**Steps**

1. Add cases:
   - `CA30-tradition`: `forbidden: ['often run at different cycle lengths',
     'Where Ewe drumming stacks many independent cycles',
     'The power of the Ewe approach is that support drums']`;
     `present: ['4 times per bell pattern', 'twice per bell pattern',
     '3:2 and 3:4', 'fr-locke-1982']`.
   - `CA30-extension`: `present: ["Poly's extension", 'Ewe-inspired',
     'Ewe Polymetric Ensemble']`. The 420-pulse arithmetic may stay, but only
     in the paragraph these phrases frame.
   - `CA31`: `forbiddenRegex: [/\bdownbeats?\b/i]` over the whole page. The
     rewritten text never uses the word, not even to deny it.
2. Run, and watch all three fail.
3. Rewrite the page:
   - **Gankogui section**: replace "the sense that it is always pulling
     forward toward the next downbeat without ever quite resolving" with
     Locke's account: the pattern is felt against four beats of three pulses
     (p. 225), and its long and short strokes sit across those beats.
   - **Polymetric Stacking**: first paragraph, the tradition per Locke. The
     bell sets the period (p. 217). In Atsiagbeko the kagan cycles 4 times per
     bell pattern and the kidi twice (p. 219). The cross-rhythms are 3:2 and
     3:4, with 3:8 sometimes (p. 244), and a cross-rhythm's period can be
     shorter than the bell's, so several fit inside one cycle (p. 233). Every
     claim links `fr-locke-1982` with its page. Second paragraph: what Poly
     adds. Lanes of 12, 7 and 5 steps that repeat together only after 420
     pulses are Poly's extension, Ewe-inspired, and not a claim about the
     tradition. "Ewe Polymetric Ensemble" is named as the example.
   - Keep the two `EuclideanDiagram`s and their sentences. Remove "deep
     structural pulse", which is a downbeat in other words.
   - **Listen for**: the convergences of the 12-, 5- and 7-step lanes are
     what Poly's layering produces. They are rare unison points between
     lanes, not an accent the tradition places. The tension described is
     between the lanes.
   - **Manding**: "Where Ewe drumming stacks many independent cycles"
     becomes a comparison that claims nothing about Ewe cycle lengths.
     The shared-cycle `:::note` stays word for word: scope-framing S03-F26
     pins it.
   - **Closing paragraph**: "emerges from the interaction of independent
     Euclidean cycles" is presented as Poly's way of making the conversation,
     not the tradition's.
4. Run and watch all cases pass. Mutate: restore "structural downbeats" in
   Listen for, run, watch CA31 fail; restore. Record it.
5. Run format, site-unit, doc-conformance and doc-discipline. Commit with
   `Rows: CA30, CA31`.

## Task 3 — The patch table is the preset it names

Rows: CA32.

**Files**

- Modify `site/tests/chapter-2-reconciled.test.mjs`
- Modify `site/tests/theory-patch-conformance.test.mjs` (`ssa-dance-beat`)
- Modify `site/src/content/docs/02-sub-saharan-africa.mdx`
- Modify `docs/plans/central-africa/evidence/M004-S01.md`

**Steps**

1. Add a structural test `CA32: the Ewe table is the preset it names`. Find
   the `<PolyPatch>` in the Polymetric Stacking section. Read its `preset`
   attribute and assert it equals `Ewe Polymetric Ensemble`. Load that preset
   from `site/src/generated/presets.json`. Parse the table, then assert the
   row count equals `lanes.length`. For each row, assert these cells against
   the lane: `Steps` = `cycleSteps`, `Hits` = `hits`, `Rotation` = `rotation`,
   `Subdivision` = `1/${subdivision}`, `Timeline` = `timeline ? 'on' : 'off'`,
   `Note` = `noteNumber`, `Velocity` = `velocity`, `Ghost` = `ghostFloor`,
   `Kotekan` = `kotekanSourceLane === -1 ? 'off' : 'L' + (kotekanSourceLane + 1)`.
   The mapping matches `site/src/lib/preset-table-data.mjs`. A failure names
   the row, the column and both values.
2. Add a second structural test, `CA32: the prose names the Kotekan lane`.
   Find the sentence matching `/Lane (\d+) uses the Kotekan setting `L(\d+)`/`.
   Assert the table row with that lane number has that Kotekan cell, and is
   the only row whose Kotekan cell is not `off`.
3. Rewrite `ssa-dance-beat` in `theory-patch-conformance.test.mjs`: the check
   passes when the table has a 12-step, 4-hit lane, *or* the block (the
   `<PolyPatch>` contents) carries a sentence holding `dance beat`,
   `12 steps` and `4 hits`. Update its description and failure message.
4. Run all three and watch them fail. The preset attribute reads
   `Sub-Saharan: Agbekor` and the table has five rows. The prose says Lane 4
   while the table's Kotekan row is 5.
5. Edit the page: set `preset="Ewe Polymetric Ensemble"` and title "Ewe
   Polymetric Ensemble". Make the table the preset's four lanes:

   | Lane | Role | Steps | Hits | Rotation | Subdivision | Timeline | Note | Velocity | Ghost | Kotekan |
   |---|---|---|---|---|---|---|---|---|---|---|
   | 1 | Bell timeline | 12 | 7 | 0 | 1/8 | on | 56 | 110 | 30 | off |
   | 2 | Support drum | 12 | 3 | 0 | 1/8 | off | 43 | 90 | 40 | off |
   | 3 | Responding drum | 5 | 3 | 0 | 1/8 | off | 45 | 85 | 50 | off |
   | 4 | Lead complement | 7 | 5 | 2 | 1/8 | off | 47 | 75 | 60 | L1 |

   Below the table, inside the `<PolyPatch>`, add the dance-beat
   instruction: "To lay the dance beat under it, add a fifth lane: a low
   drum, 12 steps, 4 hits, rotation 0, 1/8." Keep "Lane 4 uses the Kotekan
   setting `L1`" and fix its mutation advice to name lane 4.
6. Run and watch everything pass, including the full theory-patch-conformance
   file. Mutate: change one Velocity cell, run, watch CA32 name the row and
   column; restore. Record it.
7. Run format, site-unit, doc-conformance and doc-discipline. Commit with
   `Rows: CA32`.

## Task 4 — One rhythm at three rotations

Rows: CA33. Closes the slice.

**Files**

- Modify `site/tests/chapter-2-reconciled.test.mjs`
- Modify `site/src/content/docs/02-sub-saharan-africa.mdx`
- Modify `docs/plans/central-africa/evidence/M004-S01.md`
- Modify `docs/plans/central-africa/ledger.md` (rows, DoD, slice status)
- Modify `docs/plans/central-africa/M004-S01-plan.md` (DoD boxes)

**Steps**

1. Add a structural test, `CA33: the three rotations are one rhythm`. Find
   the page's sentence naming rotations. For each `rotation N` it prints
   with a gap sequence `a-b-…`, derive
   `rotateRight(bjorklund(12, 7), N)` from `site/src/audio/bjorklund.ts` and
   assert the gaps match. Assert the sentence names rotations 0, 2 and 9.
   Cross-check the sources. The appendix's k 7 n 12 row in
   `site/src/data/euclidean-appendix.json` has `rotation` 2. The
   `Sub-Saharan: Agbekor` lane 1 `onsets` equal the onset set of rotation 9.
   The page links `/appendix-euclidean-reference/` (the appendix's route) and
   `fr-locke-1982`.
2. Run and watch it fail: there is no such sentence.
3. Add the sentence after the gankogui diagram paragraph. The diagram starts
   the bell at rotation 0 (2-1-2-2-1-2-2). The Euclidean appendix shows it at
   rotation 2 (2-2-1-2-2-1-2). The bell lane of the Sub-Saharan: Agbekor
   preset starts it at rotation 9 (2-2-1-2-2-2-1). They are one rhythm begun
   at three places, and Locke observes that the cycle makes a beginning point
   "somewhat inappropriate" and that players launch it in various ways
   (p. 225).
4. Run and watch it pass. Mutate: change rotation 9's gaps to 2-2-1-2-2-1-2,
   run, watch it fail naming the derived gaps; restore. Record it.
5. Tick CA29–CA34 `done` in the ledger, tick the slice's and this plan's DoD
   boxes, and set the slice `done`. Run `jk-standards ledger`.
6. Run format, site-unit, doc-conformance and doc-discipline. Commit with
   `Rows: CA33`.

## Self-review

- **DoD coverage:** West Africa → Task 1. Locke-bounded stacking → Task 2.
  No downbeat → Task 2. Table and Kotekan lane → Task 3. Three rotations →
  Task 4. Link to Central Africa → Task 1.
- **Row coverage:** CA29 → T1 step 1 (the scope lock asserts the narrowed
  wording). CA30 → T2 (Locke checked first, above, and in the evidence; T2
  step 1 locks out the uncited sentence). CA31 → T2 step 1. CA32 → T3 steps 1
  and 3 (preset resolution plus the new cell-by-cell lock). CA33 → T4 step 1,
  against `bjorklund.ts`. CA34 → T1 step 1, whose structural case resolves the link
  to the page file the site builds it from.
- **Placeholders:** none. Every phrase a test asserts is written above.
- **Names:** `chapter-2-reconciled.test.mjs`, `Ewe Polymetric Ensemble`,
  `Sub-Saharan: Agbekor` and `ssa-dance-beat` are spelled the same throughout.
