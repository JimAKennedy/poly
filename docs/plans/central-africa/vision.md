---
class: gated
---

# Central Africa — vision

**Input document for `/jk:assess`.** Not a plan: the outcome, the current state
measured rather than assumed, and the decisions a plan will have to take. The
milestone boundaries below are arguments; the ledger that comes out of the
assessment is the plan of record.

## The observation

The guide's African material is one chapter, `02-sub-saharan-africa.mdx`, about
the Ewe and Manding traditions of West Africa. Its description claims "West and
Central African drumming", but nothing on the site describes Central African
music, and no factory preset plays it.

The owner's research note *Arom's Strict Polyrhythm — Rules & Norms for Poly*
(2026-10-15) distils Simha Arom, *African Polyphony and Polyrhythm* (Cambridge
University Press, 1991), Book VI, Chapter 3 "Strict polyrhythmics", pp. 229–306,
into twelve rules, ten norms and eleven worked formulae, transcribed as box
notation with Poly lane recipes. It was reviewed against the repository and the
scan before this vision was written. The review found two things.

**The rhythmic content holds.** Every grid was run through a port of
`engine/src/euclidean.cpp`:

| Check | Result |
|---|---|
| Each formula's per-position count of marking parts against its `(count)` row | 11 of 11 match |
| Each Euclidean recipe against the grid row it claims to produce | every recipe matches, including E(5,12) rotation 7 for the bòndò drums and E(3,16) rotation 8 for the gàzà mother drum |
| The dì.kétɔ̀ "mould" as E(13,24) | rotations 4, 22, 19, 13 and 10 each match their formula's row, and each is the only rotation that does |
| Rhythmic oddity, half−1/half+1 | all seven cell sequences avoid the halfway cut |
| Gàzà against the scan, p. 278 | accents at 7, 12, 15 (husband) and 3, 9, 14 (mother), as transcribed |

**The note's claims about Poly do not all hold,** and four of them change the
work:

1. **It proposes a page where nobody can read it.** `site/src/content/theory/`
   holds the unpublished theory deep dives, outside the site's content
   collection. A companion page there would not ship.
2. **Timeline mode does not make a lane immune to Mutation.** The global macros
   skip timeline lanes, but a lane's own Mutation and Drift still apply. The
   note's advice to put marks on a timeline lane "which ignores Mutation" would
   move the marks; the lane needs Mutation 0 and Drift 0 as well.
3. **The site cannot play a pattern defined on a page.** `PolyPatch` renders a
   table and nothing else. Play and Try it take only a factory preset name, so a
   formula the reader can hear is a formula the engine ships as a preset.
4. **Composite subdivision is reachable but invisible.** Tempo Mult is
   continuous over 0.25–4.0, so the 1.25 that àgà-tɔ̀rúmɔ̃ needs is valid and a
   preset can carry it, but the WebUI has no control for it.

The review also read Chapter 2 against the rules, and it does not agree with
them:

- **Its central premise contradicts rule R7.** Arom's periods always stand in
  simple ratios, and the formula's period is that of its longest figure (p. 231,
  p. 277). Chapter 2's "Polymetric Stacking" says Ewe support drums "often run at
  different cycle lengths from the bell", with a 12-against-7-against-5 example
  that repeats after 420 pulses. That claim carries no citation, and its "Listen
  for" box and closing paragraph build on it.
- **Two passages assume a downbeat** ("pulling forward toward the next
  downbeat", "structural downbeats"), where rule R6 says there is no measure and
  no strong beat.
- **Its role model is offered for Central Africa too.** Timeline, support and
  lead are presented for "West and Central Africa"; the Aka formulae have no
  timeline in the Ewe sense, and their drums barely vary (norm N10).
- **Its patch table does not match its preset.** The `PolyPatch` labelled
  "Sub-Saharan: Agbekor" lists five lanes. That preset has four, and the card
  below plays "Ewe Polymetric Ensemble", which matches neither. The prose puts
  the Kotekan setting on lane 4; the table puts it on lane 5.
- **Three rotations of one bell go unexplained.** The chapter's E(7,12) diagram
  is rotation 0, the Euclidean appendix shows rotation 2, and the Agbekor
  preset's drawn bell is a third rotation of the same rhythm.

And the reference record is behind the reading: `fr-arom-1991` in
`site/src/data/references.json` still says "The text was not read", with
`description: "unverified"`. The book is absent from the published
`appendix-references.mdx`, although the theory pages link to an anchor there.

**Vision.** A reader can hear Central African strict polyrhythm in Poly, formula
by formula, and learn to build it. Every grid the guide prints is one the engine
plays, and a test fails if they part. Chapter 2 says what the West African
traditions do, on cited evidence, and says plainly which layering is Poly's own.

## What success looks like

- A published chapter, "Central Africa: Strict Polyrhythm", directly after
  Chapter 2, where each of the eleven formulae can be played and opened in Try
  it.
- Opening `Aka: Ngbolu` and turning the dì.kétɔ̀ lane's Rotation from 4 to 22,
  19, 13 or 10 places the mould where mò.nzòlì, yómbè and zɔ̀bɔ̀kɔ̀, mò.kóndí or
  mò.mbénzélé put it.
- A reader can follow the chapter's build procedure and produce a formula that
  passes its audit.
- Chapter 2 makes no claim about a tradition that a cited source does not
  support, and presents co-prime layering as Poly's extension.
- `fr-arom-1991` is recorded as read, for the pages that were read.
- CI fails if a grid on the page, a preset's lanes, or an appendix entry drifts
  from Arom's transcription.

## Decisions taken with the owner

Taken on 2026-10-15, before this document was written.

| Question | Decision |
|---|---|
| Where the material lives | A new published chapter in `site/src/content/docs/`, after Chapter 2. Chapter 2 is reconciled with it in the same programme. |
| How Chapter 2 treats co-prime stacking | As Poly's extension, not the tradition. The chapter is scoped to West Africa; its stacking section describes the tradition only as far as a cited source supports, then presents 12/7/5 layering as Ewe-inspired. "Ewe Polymetric Ensemble" stays, honestly labelled. |
| Which formulae become presets | All eleven worked examples, in a new "Central African" category; "African" becomes "West African". |
| How the lanes are built | Euclidean where the note's recipe is Euclidean, so Rotation demonstrates the mould; drawn timeline lanes elsewhere. A test proves the preset's own macro settings leave every lane on Arom's grid. |

## Proposed milestones

### CA1 — The source is recorded as read

Every later milestone cites Arom, so the record comes first.

- `fr-arom-1991` in `references.json` moves to a verified description for
  pp. 229–306. Its evidence names the owner's scan of the chapter and the checks
  above. The scan is not added to the reference archive: it is a copyrighted
  book.
- The published `appendix-references.mdx` gains the Arom entry, so the anchor
  the theory pages already link to exists.
- The two Arom attributions in the unpublished theory bundle are corrected to
  what pp. 229–306 support. The chapter names the *pulsation*, usually implicit,
  as the referent (p. 230), and does not use the word "timeline". Row F52 of the
  theory-audit ledger closed by citing Arom at the dance-beat claim, and the test
  that pins that citation moves with the correction.

### CA2 — The eleven formulae are factory presets

| Preset | Formula | Source | Parts | Positions |
|---|---|---|---|---|
| Aka: Ngbolu | ngbɔ̀lù | exx. 104–105, p. 288 | 3 | 24 |
| Aka: Mokondi | mò.kóndí | exx. 106–107, p. 289 | 4 | 24 |
| Aka: Yombe | yómbè | exx. 108–109, p. 290 | 4 | 24 |
| Aka: Monzoli | mò.nzòlì | ex. 111, p. 292 | 4 | 24 |
| Aka: Mombenzele | mò.mbénzélé | ex. 112, pp. 292–293 | 4 | 24 |
| Aka: Zoboko | zɔ̀bɔ̀kɔ̀ | exx. 113–120, pp. 294–297 | 3 | 24 |
| Aka: Bondo | bòndò | exx. 101–103, pp. 286–287 | 5 | 12 |
| Aka: Ngbolu Drums | ngbɔ̀lù, drums only | ex. 90, p. 281 | 2 | 12 |
| Banda-Dakpa: Gaza | gàzà | exx. 83–84, p. 278 | 2 | 16 |
| Zande: Kponingbo | kpóníngbó | ex. 89, p. 281 | 2 | 8 |
| Banda-Linda: Aga-torumo | àgà-tɔ̀rúmɔ̃ | ex. 99, p. 285 | 2 | 10 |

The decisions every preset follows:

- **ASCII names.** Preset and lane names are ASCII, because a name crosses VST3
  strings, the WebUI and the site's preset lookup. Lane names are the part names
  (endomba, ngue, dikpakpa, diketo). The page carries the full orthography.
- **Lanes.** One lane per part, each on its own MIDI note, using the note's
  General MIDI choices. Bòndò's three drums share one rhythm but keep three
  lanes, because Arom's count of 3 at those positions is three tone colours
  sounding together.
- **Marks only.** Every hit is a mark, which is Arom's own reduction (p. 253).
  Each part plays at one fixed velocity, different per part.
- **Nothing moves.** Mutation, Drift, Swing and Humanize are 0, Probability is
  1, there is no phrase gating, and the seed is fixed.
- **Àgà-tɔ̀rúmɔ̃** runs at 1/16 with Tempo Mult 1.25, giving five steps to a
  quarter note.
- **Preview tempi** go in the site's `PRESET_BPM`, taken from Arom's figures; a
  dotted quarter of 144 is 216 quarter-note BPM. The note gives no tempo for
  àgà-tɔ̀rúmɔ̃, the ngbɔ̀lù drums alone or zɔ̀bɔ̀kɔ̀, and gàzà's figure gives
  ♩ = 112 for a pulsation of eight values. These four are read from the scan,
  and how each maps to a host tempo is stated, not assumed.

**The test is the point of the milestone.** The transcribed grids become engine
test fixtures. For every preset, each lane's rendered pattern, tiled to the
formula's period, must equal its part's row position by position. The
per-position count of marking parts must equal Arom's `(count)` row. Both are
checked at the preset's own macro settings, which is what makes the Euclidean
lanes safe to ship.

The factory count moves from 45 to 56 and the category count from 10 to 11.
Every place that hard-codes either moves in the same slice:

- `presets.h` and `presets.cpp`
- `kWebPresetLaneNames`
- the per-index tables in `webui/mock-host.js`
- `docs/preset-taxonomy.md` and its conformance test
- the Cubase nightly's "all 45 presets" and the preset-recall spec and contract
  behind it
- the site e2e test that expects 43 preset rows, already stale against 45
- the stale comment in `emit_presets.cpp`

The tables in `webui/mock-host.js` and `kWebPresetLaneNames` are kept per preset
index. The last time one of them fell out of step with the preset list, changing
preset crashed Cubase.

### CA3 — The Central Africa chapter

A published page in `site/src/content/docs/`, wired into the sidebar directly
after Chapter 2. In order:

1. What strict polyrhythm is, and its source.
2. How to read box notation and a `(count)` row, which is Arom's three-step
   method (p. 277).
3. Rules R1–R12, each as the rule, a short quote with its page, and what it
   means in Poly.
4. Norms N1–N10 in the same form, with the rhythmic-oddity table and the
   hemiola.
5. The eleven formulae, each with grid, count row, lane recipe and a
   `PolyPreviewCard`, and the comparisons that teach: mò.nzòlì against
   mò.mbénzélé, where ngúé alone tells them apart (p. 291), and gàzà as Arom's
   only strictly interwoven block.
6. The mould as Rotation values, with the knob-turn above.
7. The build procedure and its audit targets (norm N5).
8. What breaks the idiom, including the global macros, which matter here
   because some lanes are Euclidean.
9. The three printing inconsistencies in the book, with the reading the chapter
   follows for each.

The note's corrections are built in, not carried over: Mutation and Drift at 0
on any lane that carries marks, and the presets described as marks-only
reductions. Quotes stay at one phrase each. Everything else is paraphrase, and
the grids are transcriptions of rhythm.

A site test parses each grid on the page and compares it with the preset of the
same formula in `presets.json`. It also checks that every `(count)` row on the
page is the sum of its grid.

### CA4 — Chapter 2 is reconciled

- The description and opening are scoped to West Africa, Ewe and Manding.
- **Polymetric Stacking is rewritten.** A check against Locke 1982 comes first,
  for how Ewe support parts relate to the bell. The section states only what
  that source supports. If Locke does not support a simple-ratio account, the
  section makes no claim about the tradition's cycle lengths at all. It then
  presents 12/7/5 layering as what Poly adds, with "Ewe Polymetric Ensemble" as
  the Ewe-inspired example.
- Both downbeat passages are rewritten, and "Listen for" describes the
  convergences as Poly's effect.
- The `PolyPatch` table is corrected to the preset it names, and the
  lane-4/lane-5 Kotekan sentence is fixed.
- One sentence explains that the diagram, the appendix and the Agbekor bell are
  rotations of one rhythm.
- A bridge paragraph sends Central Africa to the new chapter.

### CA5 — The reference appendices catch up

- **Euclidean Reference.** A 24-step section with E(13,24) and the mould
  rotations, and the bòndò dì.kétɔ̀ as E(7,12) rotation 0. The stale "rotation 1"
  note in `euclidean-appendix.json` is corrected. The tables are generated, so
  the change goes into the data and its generator, not the page.
- **Timing Model.** Composite subdivision: 1/16 with Tempo Mult 1.25 puts five
  steps in a quarter note.
- **Chapter 1, Cycle Independence.** An aside that co-prime cycles are not the
  Central African norm, linking to the new chapter.

## Sequencing

CA1, then CA2, then CA3, then CA4 and CA5 in either order.

- The pages cite Arom, so the record goes first.
- The chapter's cards need presets, and its test needs `presets.json` to contain
  them.
- Chapter 2's bridge and the appendices' links point at the chapter, so it has
  to exist first.

## What this programme does not claim

- **That the presets sound like a performance.** They play Arom's marks, the
  abstract model of each figure. A performer fills the cells between the marks,
  and the master drummer varies; neither is transcribed in the note, and
  inventing them would be composing, not transcribing.
- **That Arom describes West Africa.** Pp. 229–306 concern Central African
  peoples (Aka, Banda, Zande). Chapter 2's rewrite rests on its own sources.
- **That the note's quote checks were repeated here.** The grids were re-checked
  independently; the note's 45 quote matches were not. Its verification report
  is not in the repository. CA1 and CA3 re-check every quote they use against
  the scan.

## Out of scope, recorded so a later pass does not rediscover it

- **Rotation for timeline-mode lanes** (the note's item 8). This is an engine
  feature. Rotation reaches only the Euclidean generator today, which is why the
  lanes use the recipes as given.
- **Per-lane realisation lists played in any order** (the note's item 9, rule
  R11). This is an engine feature; Scene A/B and Chain cover the case today.
- **A Tempo Mult control in the WebUI.** The parameter exists, and the presets
  carry it.
- **Variation presets for the master drum.** Any variations would be invented
  music.
- **The dancers' quasiperiodic sequences** (pp. 267–269), which fall outside
  strict polyrhythm by Arom's own account.
- **Publishing the theory bundle.** CA1 corrects its Arom attributions, and
  nothing more.
