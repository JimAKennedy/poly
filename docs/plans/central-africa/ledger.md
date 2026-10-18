---
class: gated
---

# Central Africa — delivery ledger

**Source:** docs/plans/central-africa/vision.md
**Slug:** central-africa

A reader can hear Central African strict polyrhythm in Poly, formula by
formula, and learn to build it. The source is Simha Arom, *African Polyphony
and Polyrhythm* (1991), Book VI, Chapter 3, pp. 229–306, as distilled in the
owner's research note *Arom's Strict Polyrhythm — Rules & Norms for Poly*
(2026-10-15) and reviewed against the engine and the scan in the vision.

## Reconciliation, before any structure

Read the vision: 37 actionable items, counting each of the eleven presets in
its CA2 table as an item, plus four owner decisions.

| | |
|---|---|
| Already satisfied | 0. The vision was written from the same day's review, so nothing in it predates the tree |
| Partly satisfied | 1. CA36: the engine already accepts Tempo Mult 1.25, but nothing documents composite subdivision; the row is rescoped to the Timing Model page |
| Contradicted by the current code | 0. The research note's four wrong claims about Poly were corrected in the vision, before assessment |
| Outstanding | 36 |
| Deferred and recorded | 6. See Out of scope |

## Milestone M001 — Arom is a source the guide has read

**Vision:** The guide's record of Arom 1991 says what was read and where, and
every page that cites it credits only what those pages say.

**Branch:** milestone/M001-arom-record
**Status:** done

### Slice M001/S01 — Arom is recorded as read

**Plan:** M001-S01-plan.md
**Validation:** format, site-unit, doc-discipline, guards
**Evidence:** evidence/M001-S01.md
**Status:** done

**Definition of Done**

- [x] `fr-arom-1991` in `site/src/data/references.json` carries a verified
      description limited to pp. 229–306, and its evidence names the owner's
      scan and how it was checked; no scan is added to the reference archive
- [x] Neither theory-bundle page credits Arom with a claim pp. 229–306 do not
      make, and the test that pinned the F52 citation pins the corrected one

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| CA01 | `fr-arom-1991` still says "The text was not read" with `description: "unverified"`, though the owner has read and checked pp. 229–306 from a scan | `source` | `site/src/data/references.json` | `references-manifest.test.mjs` passes on the changed entry; the evidence quotes the new `evidence` and `description` values | `done` |
| CA03 | The theory bundle credits Arom with the timeline as "shared clock" and with the dance-beat referent. Pp. 229–306 name the pulsation, usually implicit, as referent (p. 230) and never use "timeline" | `correct` | `site/src/content/theory/theory-sub-saharan-africa.mdx`, `theory-counterpoint-overview.mdx`, `site/tests/literature-enrichment.test.mjs` | Each corrected sentence quotes or cites a page in 229–306 that was checked against the scan; the F52 case asserts the new citation site | `done` |

## Milestone M002 — The eleven formulae are factory presets

**Vision:** Every formula Arom transcribes in the chapter's worked examples
ships as a factory preset that plays his marks exactly, and CI fails if one
position moves.

**Branch:** milestone/M002-central-african-presets
**Status:** done

**One slice, not two.** The taxonomy conformance test fails the moment the
engine's preset count changes, so the presets and every site that hard-codes
the count land together; splitting by formula would move each count site
twice.

### Slice M002/S01 — Eleven formulae are presets

**Plan:** M002-S01-plan.md
**Validation:** format, unit, rt-safety, site-unit, webui-e2e, cubase-harness, doc-discipline, e2e
**Evidence:** evidence/M002-S01.md
**Status:** done

**Definition of Done**

- [x] Eleven presets exist in a new "Central African" category, and the
      existing "African" category is named "West African"
- [x] Every preset's lanes, tiled to the formula's period, equal Arom's grid
      position by position at the preset's own macro settings, and the
      per-position count of marking parts equals his `(count)` row, asserted
      by a test that fails if one position moves
- [x] Every lane that carries marks has Mutation, Drift, Swing and Humanize at
      0, Probability 1 and no phrase gating
- [x] Every place that hard-codes the preset or category count agrees with 56
      and 11, and the WebUI lists all 56 presets with a lane name for each
      Central African lane
- [x] The committed WASM engine contains the new presets

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| CA04 | Aka: Ngbolu — ngbɔ̀lù, exx. 104–105, p. 288: three parts over 24 positions, the dì.kétɔ̀ lane E(13,24) rotation 4 | `capability` | `engine/src/presets.cpp`, `engine/include/poly/presets.h` | The grid test passes for this preset (CA15) | `done` |
| CA05 | Aka: Mokondi — mò.kóndí, exx. 106–107, p. 289: four parts over 24 positions, dì.kétɔ̀ rotation 13 | `capability` | `engine/src/presets.cpp` | The grid test passes for this preset (CA15) | `done` |
| CA06 | Aka: Yombe — yómbè, exx. 108–109, p. 290: four parts over 24 positions, dì.kétɔ̀ rotation 19 | `capability` | `engine/src/presets.cpp` | The grid test passes for this preset (CA15) | `done` |
| CA07 | Aka: Monzoli — mò.nzòlì, ex. 111, p. 292: four parts over 24 positions, dì.kétɔ̀ rotation 22 | `capability` | `engine/src/presets.cpp` | The grid test passes for this preset (CA15) | `done` |
| CA08 | Aka: Mombenzele — mò.mbénzélé, ex. 112, pp. 292–293: four parts over 24 positions, dì.kétɔ̀ rotation 10, ngúé over the full 24 values (p. 303) | `capability` | `engine/src/presets.cpp` | The grid test passes for this preset (CA15) | `done` |
| CA09 | Aka: Zoboko — zɔ̀bɔ̀kɔ̀, exx. 113–120, pp. 294–297: three parts over 24 positions, handclaps on the pulsation, dì.kétɔ̀ rotation 19 | `capability` | `engine/src/presets.cpp` | The grid test passes for this preset (CA15) | `done` |
| CA10 | Aka: Bondo — bòndò, exx. 101–103, pp. 286–287: five parts over 12 positions, three drums on one model rhythm as three lanes | `capability` | `engine/src/presets.cpp` | The grid test passes for this preset (CA15), including the count of 3 at the model's positions | `done` |
| CA11 | Aka: Ngbolu Drums — ngbɔ̀lù drums only, ex. 90, p. 281: two parts over 12 positions | `capability` | `engine/src/presets.cpp` | The grid test passes for this preset (CA15) | `done` |
| CA12 | Banda-Dakpa: Gaza — gàzà, exx. 83–84, p. 278: two slit drums over 16 positions, strictly interwoven | `capability` | `engine/src/presets.cpp` | The grid test passes for this preset (CA15), including no position shared | `done` |
| CA13 | Zande: Kponingbo — kpóníngbó, ex. 89, p. 281: drum and pellet bells over 8 positions | `capability` | `engine/src/presets.cpp` | The grid test passes for this preset (CA15) | `done` |
| CA14 | Banda-Linda: Aga-torumo — àgà-tɔ̀rúmɔ̃, ex. 99, p. 285: two drums over 10 positions at 1/16 with Tempo Mult 1.25 | `capability` | `engine/src/presets.cpp` | The grid test passes for this preset (CA15); a test asserts the step length is 0.2 beats | `done` |
| CA15 | Nothing proves a preset plays what a book transcribes. The eleven grids and `(count)` rows become fixtures, compared position by position with each preset's rendered lanes at its own macro settings | `coverage` | `tests/` (engine tests), fixture data beside them | The test fails when a fixture position is flipped, run once and recorded in the evidence, and passes on the real fixtures | `done` |
| CA16 | There is no Central African category, and "African" holds only West African and Afrobeat presets | `tooling` | `engine/src/presets.cpp`, `engine/include/poly/presets.h`, `docs/preset-taxonomy.md` | `preset-taxonomy-conformance.test.mjs` passes with 11 categories | `done` |
| CA17 | The preset count is hard-coded in about ten places: `kWebPresetLaneNames`, the per-index tables in `webui/mock-host.js`, `docs/preset-taxonomy.md`, the Cubase nightly's "all 45 presets" with the preset-recall spec and contract, a site e2e test that expects 43, and a stale comment in `emit_presets.cpp` | `tooling` | the files named | A repo search for 43 and 45 as preset counts finds none; site-unit, webui-e2e and cubase-harness pass | `done` |
| CA18 | The committed `webui/poly_engine.wasm` and `poly_engine.js` would not contain the new presets | `tooling` | `webui/poly_engine.js`, `webui/poly_engine.wasm` | The `e2e` token rebuilds them and the site plays a new preset; `wasm-freshness` passes in CI | `done` |

## Milestone M003 — The Central Africa chapter

**Vision:** A published chapter teaches Arom's strict polyrhythm as rules,
norms and eleven playable formulae, and its grids cannot disagree with the
presets that play them.

**Branch:** milestone/M003-central-africa-chapter
**Status:** done

### Slice M003/S01 — The chapter's teaching text

**Depends:** M001/S01
**Plan:** M003-S01-plan.md
**Validation:** format, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M003-S01.md
**Status:** done

**Definition of Done**

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

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| CA02 | The book is absent from the published `appendix-references.mdx`. The FR20 rule admits an entry only once a published page cites it, and this chapter is the first that will. **Moved from M001/S01 on 2026-10-15:** the theory pages resolve their Arom links against their own bibliography, which has the entry, so no published link is broken today | `source` | `site/src/content/docs/appendix-references.mdx` | `appendix-entry-rules.test.mjs` (FR20 cited) passes with the entry present; `research-provenance` (doc-discipline) resolves the chapter's citations | `done` |
| CA19 | Nothing on the site describes Central African music; the chapter page and its sidebar entry do not exist | `docs` | new page in `site/src/content/docs/`, `site/astro.config.mjs` | The site build includes the page; doc-discipline's doc-completeness maps it | `done` |
| CA20 | A reader has no way to read box notation or a `(count)` row, Arom's three-step method (p. 277) | `docs` | the new page | A worked count row on the page equals the sum of its grid, checked by CA28's test | `done` |
| CA21 | Rules R1–R12, with the vision's correction that timeline mode does not stop Mutation or Drift | `docs` | the new page | Each rule cites a page; each quote is checked against the scan and listed in the evidence | `done` |
| CA22 | Norms N1–N10, with the rhythmic-oddity table and the hemiola | `docs` | the new page | Each norm cites a page; each oddity cell sequence on the page sums to its stated length and avoids the halfway cut, asserted by a site test | `done` |
| CA23 | The build procedure and what breaks the idiom, including the global macros acting on Euclidean lanes | `docs` | the new page | The procedure's audit targets match N5 as stated on the page; reviewed against the vision's CA3 list | `done` |
| CA24 | The book's three printing inconsistencies (ex. 129, p. 248, pp. 291 and 303), with the reading the chapter follows | `disclose` | the new page | Each is checked against the scan and the evidence records what was seen | `done` |

### Slice M003/S02 — The chapter plays

**Depends:** M002/S01, M003/S01
**Plan:** M003-S02-plan.md
**Validation:** format, site-unit, doc-conformance, doc-discipline, e2e
**Evidence:** evidence/M003-S02.md
**Status:** done

**Definition of Done**

- [x] Each of the eleven formulae has a section with its grid, count row, lane
      recipe and a `PolyPreviewCard` that plays its preset and opens Try it
- [x] Every card plays at a tempo taken from Arom's figure, and the page says
      how that tempo maps to the host's quarter-note BPM
- [x] The page tells the reader to open Aka: Ngbolu and turn the dì.kétɔ̀
      lane's Rotation to place the mould of each other formula
- [x] A site test fails if any grid on the page differs from the preset that
      plays it, or if any count row is not the sum of its grid

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| CA25 | The eleven formulae have no section and no way to be heard on the site | `docs` | the new page | Each section embeds a card naming a preset that `resolvePresetName` resolves; e2e plays one | `done` |
| CA26 | The note gives no tempo for àgà-tɔ̀rúmɔ̃, the ngbɔ̀lù drums alone or zɔ̀bɔ̀kɔ̀, and gàzà's ♩ = 112 is for a pulsation of eight values; `PRESET_BPM` has no entry for any formula | `source` | `site/src/audio/preset-patterns.ts`, the new page | The four tempi are read from the scan with page and example recorded in the evidence; each `PRESET_BPM` value is derivable from the figure's marking by the page's stated mapping | `done` |
| CA27 | The mould is taught as rotation values, but nothing tells the reader to hear it by turning one knob | `docs` | the new page | The instruction names rotations 4, 22, 19, 13 and 10, each of which the grid test (CA15) proves | `done` |
| CA28 | Nothing stops the page's grids drifting from the presets | `coverage` | `site/tests/` | The test fails when one page grid position is flipped, run once and recorded, and passes on the page as committed | `done` |

## Milestone M004 — Chapter 2 says what the tradition does

**Vision:** Chapter 2 describes West African practice only as far as its cited
sources support, presents co-prime layering as Poly's own, and agrees with the
Central Africa chapter.

**Branch:** milestone/M004-chapter-2-reconciled
**Status:** done

### Slice M004/S01 — Chapter 2 reconciled

**Depends:** M003/S01
**Plan:** M004-S01-plan.md
**Validation:** format, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M004-S01.md
**Status:** done

**Definition of Done**

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

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| CA29 | The description and opening claim "West and Central African drumming" and offer timeline, support and lead roles for both | `reframe` | `site/src/content/docs/02-sub-saharan-africa.mdx` | `scope-framing.test.mjs` (or the suite that locks scope corrections) asserts the narrowed wording | `done` |
| CA30 | Polymetric Stacking says Ewe support drums "often run at different cycle lengths", with a 12/7/5 example, and cites nothing. Arom's R7 says periods stand in simple ratios | `reframe` | `02-sub-saharan-africa.mdx` | Locke 1982 is checked first and the evidence records what it supports; if it supports no simple-ratio account, the section makes no claim about the tradition's cycle lengths; a test locks out the uncited sentence | `done` |
| CA31 | "Pulling forward toward the next downbeat" and "structural downbeats" assert a downbeat; R6 says there is none | `correct` | `02-sub-saharan-africa.mdx` | A test asserts neither phrase returns | `done` |
| CA32 | The `PolyPatch` labelled "Sub-Saharan: Agbekor" lists five lanes; that preset has four and the card below plays "Ewe Polymetric Ensemble". The prose puts Kotekan on lane 4; the table puts it on lane 5 | `patch-align` | `02-sub-saharan-africa.mdx` | `theory-patch-conformance` / preset-resolution tests pass against the corrected table | `done` |
| CA33 | The chapter's E(7,12) diagram is rotation 0, the appendix shows rotation 2, and the Agbekor bell is a third rotation; nothing says they are one rhythm | `docs` | `02-sub-saharan-africa.mdx` | The sentence's rotations are checked against `bjorklund.ts` by an existing or new pattern-claims test | `done` |
| CA34 | Chapter 2 does not send readers to Central Africa | `docs` | `02-sub-saharan-africa.mdx` | The link resolves in the site build | `done` |

## Milestone M005 — The reference appendices catch up

**Vision:** The Euclidean Reference, the Timing Model and Chapter 1 carry what
the Central Africa chapter relies on.

**Branch:** milestone/M005-appendices
**Status:** in-progress

### Slice M005/S01 — Appendices

**Depends:** M003/S01
**Plan:** M005-S01-plan.md
**Validation:** format, site-unit, doc-conformance, doc-discipline
**Evidence:** evidence/M005-S01.md
**Status:** in-progress

**Definition of Done**

- [ ] The Euclidean Reference has a 24-step section with E(13,24) and the five
      mould rotations, and names bòndò's dì.kétɔ̀ as E(7,12) at rotation 0
- [ ] The generator data no longer describes the gankogui bell at a rotation
      the table does not show
- [ ] The Timing Model explains composite subdivision as 1/16 with Tempo Mult
      1.25
- [ ] Chapter 1's Cycle Independence says co-prime cycles are not the Central
      African norm and links to the chapter

| ID | Item | Kind | Lands in | Verification | Status |
|---|---|---|---|---|---|
| CA35 | The Euclidean Reference has no 24-step section and no E(13,24); its data file's note still says the gankogui bell is at rotation 1 | `docs` | `site/src/data/euclidean-appendix.json`, `scripts/generate-euclidean-appendix.mjs` | `generated-freshness` passes; a site test checks every new row against `bjorklund.ts` | `done` |
| CA36 | The engine accepts Tempo Mult 1.25, but the Timing Model never describes five steps to a beat. Partly satisfied: the capability exists, the documentation does not | `docs` | `site/src/content/docs/appendix-timing-model.mdx` | The worked arithmetic (0.25 ÷ 1.25 = 0.2 beats) is asserted by a site test | `done` |
| CA37 | Chapter 1's Cycle Independence presents co-prime cycles without saying they are not the Central African norm | `docs` | `site/src/content/docs/01-foundations.mdx` | The link resolves in the site build; the aside cites R7's page | `open` |

## Sequencing

**M001 and M002 are independent.** The record touches references only, and
the presets cite nothing. Either can go first.

**M003/S01 depends on M001/S01** because the chapter cites `fr-arom-1991`
as M001 records it, and adds the published entry itself (CA02, moved there on
2026-10-15). **M003/S02 depends on M002/S01** because
its cards name the presets and its test reads them from `presets.json`, and on
**M003/S01** because the formula sections sit on that page.

**M004/S01 and M005/S01 depend on M003/S01** because each links to the new
chapter. They touch different files and are independent of each other.

## Out of scope

- Rotation for timeline-mode lanes, the note's item 8. Rotation reaches only
  the Euclidean generator, which is why the presets use the recipes as given
- Per-lane realisation lists played in any order, the note's item 9 (rule
  R11). Scene A/B and Chain cover the case today
- A Tempo Mult control in the WebUI. The parameter exists, and the presets
  carry it
- Variation presets for the master drum. Any variations would be invented
  music
- The dancers' quasiperiodic sequences (pp. 267–269), outside strict
  polyrhythm by Arom's own account
- Publishing the theory bundle. M001 corrects its Arom attributions, and
  nothing more
