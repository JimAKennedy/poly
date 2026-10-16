---
class: gated
---

# M003/S01 — The chapter's teaching text

**Slice:** central-africa M003/S01 (rows CA02, CA19–CA24)
**Ledger:** `docs/plans/central-africa/ledger.md`
**Decisions consumed:** `M003-decisions.md` (2026-10-16)

## Task status

- [x] 1. The page exists, is wired in, and teaches box notation
- [ ] 2. Rules R1–R12 and norms N1–N10, with the oddity test
- [ ] 3. The build procedure, what breaks the idiom, and the printing inconsistencies

## Definition of Done

Copied verbatim from the slice:

- [ ] A published page "Central Africa: Strict Polyrhythm" sits directly after
      Chapter 2 in the sidebar
- [ ] The published `appendix-references.mdx` has an `fr-arom-1991` entry,
      added with the chapter's first citation of it
- [ ] The page states rules R1–R12 and norms N1–N10, each with a page
      reference, and what each means in Poly
- [ ] Every quote on the page is one phrase long and was found on the page it
      cites in the scan
- [ ] The page's Poly guidance tells the reader to keep Mutation and Drift at 0
      on any lane that carries marks, and says the presets are marks-only
      reductions
- [ ] The build procedure, what breaks the idiom (including the global macros)
      and the book's three printing inconsistencies are on the page

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |

## Sources and constraints

- **Source.** Simha Arom, *African Polyphony and Polyrhythm* (1991), Book VI,
  Chapter 3, pp. 229–306, read from the owner's scan (book page n = PDF page
  n − 228; render with `pdftoppm -f N -l N -r 150 -gray -png`, rotate
  upside-down pages with `sips -r 180`). The owner's research note is the
  working draft for rules, norms and grids; it is not committed and not cited
  (decision 2026-10-16). Its rule R10 and R12 advice ("timeline mode, which
  ignores Mutation") is wrong for Poly: a lane's own Mutation and Drift still
  apply, so the page says Mutation 0 and Drift 0 on every lane that carries
  marks.
- **Quotes.** One phrase each, at most one per rule or norm. Each is checked
  by reading the rendered scan page it cites; the evidence file lists every
  quote with its page and "seen". A quote not found verbatim on its page is
  cut, not reworded into quotation marks.
- **Guards the page must satisfy** (surveyed 2026-10-16):
  - `class: gated` frontmatter; no `Status:` line; no file:line references.
  - count-drift: no number word or digit followed within three words by
    "factory preset(s)", "presets in total", "of the presets", "chapters
    total" and the like, outside code fences and table rows.
  - Every `#fr-…` anchor must exist in `appendix-references.mdx`.
  - `<PolyPreviewCard preset="…">` with `preset` first (S02's concern).
  - No box-drawing glyphs (`check-ascii-diagrams.mjs`); box notation uses
    `x`, `.` and `|` only.
  - `console-error-gate.spec.ts` auto-loads every docs page (e2e, S02).
- **No `patternClaims` frontmatter.** It is opt-in, and its parser rejects the
  `|` bars the page's grids use; the grids are guarded by S02's test.

## Task 1 — The page exists, is wired in, and teaches box notation

Rows: CA02, CA19, CA20.

**Files**

- Create `site/src/content/docs/02b-central-africa.mdx`
- Modify `site/astro.config.mjs` (Chapters items, after chapter 2)
- Modify `.github/docs-drift-map.yml` (`cannot_drift`, musical-scholarship block)
- Modify `site/src/content/docs/appendix-references.mdx` (new section)

**Steps**

1. Run `npm --prefix site test 2>&1 | tail -5` and record the pass count as
   the baseline.
2. Write the page frontmatter:
   `class: gated`, `title: "2b. Central Africa: Strict Polyrhythm"`, and a
   `description` naming Arom's strict polyrhythm of the Central African
   Republic and the eleven playable formulae without a count-drift trigger.
3. Section "What strict polyrhythm is": the source (book, chapter, pages,
   linked once as `[Arom 1991](/appendix-references/#fr-arom-1991)`), the
   peoples (Aka, Banda-Dakpa, Banda-Linda, Zande), and what separates it from
   Chapter 2's co-prime layering (simple ratios, p. 231), stated as Arom's
   description, not a ranking.
4. Section "Reading box notation": one character per minimal operational
   value (p. 231), `x` a mark, `.` none, `|` a pulsation boundary for reading
   only, positions numbered from 1; Arom's three-step count (p. 277); the
   marks-only reduction (p. 253) and that the presets are marks-only
   reductions. A worked example: the ngbɔ̀lù drums alone (ex. 90, p. 281) as a
   fence with info string `text title="Aka: Ngbolu Drums"`, its two rows and
   `(count)` row `[1 . 2 | . 1 1 | . . 2 | . 1 1]` — the format S02's grid
   test reads, so CA28's test checks this count row too.
5. Sidebar: add `{ label: '2b. Central Africa', slug: '02b-central-africa' }`
   directly after the chapter 2 item. Do not touch any `region:` marker.
6. Drift map: add a `cannot_drift` entry for
   `site/src/content/docs/02b-central-africa.mdx` with the reason
   "Musicology of Central African strict polyrhythm (Arom 1991); its grids are
   guarded against presets.json by a site test".
7. Appendix: add a section `## Central Africa (Chapter 2b)` between the
   Sub-Saharan Africa and Afro-Cuban sections, holding
   `- <span id="fr-arom-1991" data-tier="A">Arom, S. (1991). *African Polyphony and Polyrhythm: Musical Structure and Methodology*. Cambridge University Press.</span> ISBN 9780521616010 *(Purchasable)*`
   — the theory bibliography's wording. Leave the theory copy in place.
8. Verify the quotes this task adds against the scan (pp. 229, 231, 253, 277)
   and start `evidence/M003-S01.md` with a quote table.
9. Run `npm --prefix site test`; FR20 cited, tier and obtainable pass with
   the entry present; record the pass count. Run
   `bash scripts/check-doc-discipline.sh`; doc-completeness maps the page and
   research-provenance resolves its anchor. Run `pre-commit run --all-files`
   and `bash scripts/check-doc-conformance.sh`.
10. Run `npm --prefix site run build` and confirm `dist/02b-central-africa/index.html`
    exists.
11. Commit with `Rows: CA02, CA19, CA20`.

## Task 2 — Rules R1–R12 and norms N1–N10, with the oddity test

Rows: CA21, CA22.

**Files**

- Create `site/tests/central-africa-oddity.test.mjs`
- Modify `site/src/content/docs/02b-central-africa.mdx`
- Modify `scripts/check-doc-conformance.sh` (add the test to `TESTS`)

**Steps**

1. Write `central-africa-oddity.test.mjs` first. It reads the page, finds the
   oddity table (a markdown table whose header row contains `Cells`), and for
   each row parses the cell sequence (`3+2+2+3+2`) and the stated length. It
   asserts the cells sum to the length, that no running total equals
   length ÷ 2, and that the box-notation onsets in the row equal the cell
   starts. It also asserts the table has the six rows below.
2. Run it and watch it fail: the table does not exist yet.
3. Write "The rules" section: R1–R12, each as a heading or bold lead, the
   rule in one or two sentences of paraphrase, one quoted phrase with its page,
   and "In Poly". The R10 and R12 Poly guidance says: put the marks on a lane
   with Mutation 0 and Drift 0 (timeline mode alone does not stop either; the
   global macros skip timeline lanes, but a lane's own Mutation and Drift
   apply), and give variation to a second, quieter lane. R7's Poly guidance
   names co-prime lengths as Poly's own default, not this idiom's.
4. Write "The norms" section: N1–N10 in the same form, with the oddity table
   (exx. 31, 33, 34, 35, 36 and the bòndò model ex. 56: cells, steps, split,
   onsets) and the mò.nzòlì hemiola grid (p. 291).
5. Run the oddity test and watch it pass. Flip one cell in the page table
   (`3+2+2+3+2` → `3+2+3+2+2`), run, watch it fail on the halfway cut or the
   onsets, restore, and record the red run in the evidence.
6. Verify every quote this task adds against the scan, page by page, and add
   them to the evidence table. Cut any quote not found verbatim.
7. Add the test to `TESTS` in `scripts/check-doc-conformance.sh`.
8. Run all four validation commands; commit with `Rows: CA21, CA22`.

## Task 3 — The build procedure, what breaks the idiom, and the printing inconsistencies

Rows: CA23, CA24.

**Files**

- Modify `site/src/content/docs/02b-central-africa.mdx`
- Modify `docs/plans/central-africa/evidence/M003-S01.md`

**Steps**

1. Write "Building a formula": the note's eight steps, with Poly settings
   corrected (marks on lanes with Mutation 0 and Drift 0; the varying part
   split into a fixed marks lane and a quieter Mutation lane). Step 5's audit
   targets are N5's as stated on the page: 20 or more of 24 positions marked,
   no more than two positions shared by every part, at least one solo
   position for each part.
2. Write "What breaks the idiom": co-prime lane lengths (R7); a loud step 1
   or high Emphasis (R6); Mutation, Drift, Density or Complexity on a lane that
   carries marks (R10), including that the global Density and Complexity
   macros re-derive a Euclidean lane's hits and so move its marks, while
   Rotation is the one control that moves the mould on purpose; Swing or
   Humanize (R1); parts that line up (R8, R9); variation on several lanes
   (R12).
3. Write "Three printing inconsistencies": ex. 129 p. 305 (mò.kóndí ngúé mark
   at 11, read as 8 per exx. 95 and 107); p. 248 (the 24-value paradigm row
   sums to 20; ex. 35 p. 247 is followed); pp. 291 and 303 (mò.mbénzélé ngúé
   period; p. 303 and ex. 112 are followed). Check each against the rendered
   scan pages and record what was seen.
4. Check the audit targets against N5 on the page and the "what breaks" list
   against the vision's CA3 list; note both in the evidence.
5. Run all four validation commands; set the slice's DoD boxes, rows and
   status in the ledger; commit with `Rows: CA23, CA24`.
