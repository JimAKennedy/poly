---
class: gated
---

# M003/S02 — The chapter plays

**Slice:** central-africa M003/S02 (rows CA25–CA28)
**Ledger:** `docs/plans/central-africa/ledger.md`
**Decisions consumed:** `M003-decisions.md` (2026-10-16)

## Task status

- [x] 1. The grid test, proven on Aka: Ngbolu
- [x] 2. The other ten formulae, their tempi, and the mould
- [ ] 3. The site plays a Central African card

## Definition of Done

Copied verbatim from the slice:

- [ ] Each of the eleven formulae has a section with its grid, count row, lane
      recipe and a `PolyPreviewCard` that plays its preset and opens Try it
- [ ] Every card plays at a tempo taken from Arom's figure, and the page says
      how that tempo maps to the host's quarter-note BPM
- [ ] The page tells the reader to open Aka: Ngbolu and turn the dì.kétɔ̀
      lane's Rotation to place the mould of each other formula
- [ ] A site test fails if any grid on the page differs from the preset that
      plays it, or if any count row is not the sum of its grid

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |
| `e2e` | `bash scripts/site-verify-local.sh` |

`e2e` rewrites `webui/poly_engine.js` and `webui/poly_engine.wasm`. This slice
changes no engine source, so after every run restore them with
`git checkout -- webui/poly_engine.js webui/poly_engine.wasm` and stage explicit
paths, never `git add -A`. Run `e2e` under `caffeinate -dimsu`: an idle Mac
starved the audio gate twice on 2026-10-16.

## The grid format

Each formula's grid is a fenced block whose info string is
`text title="<preset name>"`, so it renders with the preset's name as its
title:

````
```text title="Aka: Ngbolu"
dì.kpàkpà [. . x | . . x | … ]
ngúé      [x . x | . x . | … ]
dì.kétɔ̀   [x . x | . x . | … ]
(count)   [2 . 3 | . 2 1 | … ]
```
````

- Each part row is a label, then `[`, then one token per position (`x` or
  `.`), with `|` ignored, then `]`. The last row is labelled `(count)`, with
  tokens `.` (zero) or a digit.
- Part rows map to the preset's lanes in order. A label holding `/` names
  several lanes that play one part (bòndò's three drums,
  `è.ndòmbà/ngúé/dì.kpàkpà`), and consumes one lane per name.
- Every lane is tiled to the grid's length (the formula's period, rule R7).
  A Euclidean lane is `rotateRight(bjorklund(cycleSteps, hits), rotation)`
  from `site/src/audio/bjorklund.ts`; a timeline lane is its `onsets` within
  `fixedPatternLength`, repeated to `cycleSteps`.

## Tempi

Host quarter-note BPM = pulsation tempo × values per pulsation × step length in
beats (decision 2026-10-16). Each value is read from the figure named; Task 2
re-reads every one from the scan and records it in the evidence.

| Preset | Figure | Marking | Factor | `PRESET_BPM` |
|---|---|---|---|---|
| Aka: Ngbolu | ex. 104, p. 288 | ♩. = 144 | 1.5 | 216 |
| Aka: Mokondi | ex. 106, p. 289 | ♩. = 148 | 1.5 | 222 |
| Aka: Yombe | ex. 108, p. 290 | ♩. = 144 | 1.5 | 216 |
| Aka: Monzoli | ex. 111, p. 292 | ♩. = 138 | 1.5 | 207 |
| Aka: Mombenzele | ex. 112, pp. 292–293 | ♩. = 138 | 1.5 | 207 |
| Aka: Zoboko | ex. 115, p. 294 | ♩. = 160 | 1.5 | 240 |
| Aka: Bondo | ex. 102, p. 287 | ♩. = 158 | 1.5 | 237 |
| Aka: Ngbolu Drums | ex. 104, p. 288 (ex. 90 has none) | ♩. = 144 | 1.5 | 216 |
| Banda-Dakpa: Gaza | ex. 83, p. 278 | ♩ = 112 (8 values) | 2 | 224 |
| Zande: Kponingbo | ex. 89, p. 281 | ♩ = 168 (4 values) | 1 | 168 |
| Banda-Linda: Aga-torumo | ex. 100, p. 285 | dotted quaver tied to quaver = 110 (5 values) | 1 | 110 |

If the scan shows a marking that differs from this table, the scan wins:
correct the table and the decisions file in the same commit, and record it.

## Task 1 — The grid test, proven on Aka: Ngbolu

Rows: CA28, and CA25 for this formula.

**Files**

- Create `site/tests/central-africa-grids.test.mjs`
- Modify `site/src/content/docs/02b-central-africa.mdx`
- Modify `site/src/audio/preset-patterns.ts` (`PRESET_BPM`)
- Modify `scripts/check-doc-conformance.sh` (add the test to `TESTS`)

**Steps**

1. Write the test. It reads the page, extracts every `text title="…"` fence,
   resolves the title with `resolvePresetName` (failing if it does not
   resolve), parses the rows as above, and asserts for each grid: (a) each
   part row equals each lane it maps to, tiled, position by position, naming
   the preset, row and position on failure; (b) the number of lanes consumed
   equals the preset's lane count; (c) the `(count)` row equals the
   per-position sum of the part rows. It also asserts that every title in a
   required list of eleven names is present. Leave the list holding only
   `Aka: Ngbolu` for this task.
2. Run it and watch it fail: the page has no grid.
3. Add the section "The formulae" and its first subsection, Aka: Ngbolu
   (exx. 104–105, p. 288): what the formula is, its grid fence, the lane
   recipe table (part, steps, setting, note), and
   `<PolyPreviewCard preset="Aka: Ngbolu" caption="…" />` with `preset` first.
4. Add `'Aka: Ngbolu': 216` to `PRESET_BPM`.
5. Run the test and watch it pass. Flip one mark in the page's ngúé row, run,
   watch it fail naming the position; restore. Change one `(count)` digit,
   run, watch it fail; restore. Record both red runs in
   `evidence/M003-S02.md`.
6. Add the test to `TESTS` in `scripts/check-doc-conformance.sh`.
7. Run format, site-unit, doc-conformance and doc-discipline. Commit with
   `Rows: CA25, CA28`.

## Task 2 — The other ten formulae, their tempi, and the mould

Rows: CA25, CA26, CA27.

**Files**

- Modify `site/src/content/docs/02b-central-africa.mdx`
- Modify `site/src/audio/preset-patterns.ts`
- Modify `site/tests/central-africa-grids.test.mjs` (required list)
- Modify `docs/plans/central-africa/evidence/M003-S02.md`

**Steps**

1. Extend the test's required list to all eleven presets. Run it and watch it
   fail on the ten missing titles.
2. Add the ten subsections in the vision's order of teaching: the two-part
   formulae first (Gaza, Kponingbo, Ngbolu Drums), then Bondo, Aga-torumo, then
   the four-part Aka formulae (Mokondi, Yombe, Monzoli, Mombenzele, Zoboko).
   Each has its figure and page, the grid fence, the lane recipe and a card.
   Include the two comparisons that teach: mò.nzòlì against mò.mbénzélé, where
   ngúé alone tells them apart (p. 291), and gàzà as the only strictly
   interwoven block (p. 278). For Bondo, write the drums row with the
   `/`-joined label.
3. Re-read every tempo marking in the Tempi table from the scan; record figure,
   page and marking in the evidence. Add the ten remaining `PRESET_BPM`
   entries.
4. Add "Tempo" text to the page: the mapping formula, the factor for each
   subdivision, and that ex. 90 has no marking, so Aka: Ngbolu Drums takes ex.
   104's.
5. Add "The mould": the dì.kétɔ̀ figure is E(13,24); a table of Arom's offsets
   (p. 298) against Rotation 4, 22, 19, 13, 10; and the instruction to open
   Aka: Ngbolu in Try it and turn the dì.kétɔ̀ lane's Rotation to each value.
   Each value is one the engine grid test (CA15) proves.
6. Run the grid test and watch it pass on all eleven.
7. Run format, site-unit, doc-conformance and doc-discipline. Commit with
   `Rows: CA25, CA26, CA27`.

## Task 3 — The site plays a Central African card

Rows: CA25 (e2e), closes the slice.

**Files**

- Modify `site/tests-e2e/site-audio-gate.spec.ts` (`CHAPTERS`)
- Modify `docs/plans/central-africa/evidence/M003-S02.md`
- Modify `docs/plans/central-africa/ledger.md`

**Steps**

1. Add rows to the audio gate's `CHAPTERS` for `Aka: Ngbolu` and
   `Banda-Linda: Aga-torumo` (the Tempo Mult 1.25 case), path
   `/02b-central-africa/`, `expectedBpm` from `PRESET_BPM`,
   `resolutionPath: 'direct'`, following the existing rows' shape.
2. Run `caffeinate -dimsu bash scripts/site-verify-local.sh`. The two new rows
   pass, and the console-error gate loads the new page with no errors. Restore
   the WASM artifacts.
3. Run all five validation commands; record results; tick the slice's DoD,
   rows and status in the ledger; commit with `Rows: CA25`.
