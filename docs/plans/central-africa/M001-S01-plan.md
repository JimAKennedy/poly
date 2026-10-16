---
class: gated
---

# M001/S01 — Arom is recorded as read

**Slice:** central-africa M001/S01 (rows CA01–CA03)
**Ledger:** `docs/plans/central-africa/ledger.md`
**Decisions consumed:** `M001-decisions.md` (2026-10-15)

## Task status

- [x] 1. `fr-arom-1991` records what was read
- [ ] 2. The published bibliography carries Arom
- [ ] 3. The theory bundle credits Arom only with what pp. 229–306 say

## Definition of Done

Copied verbatim from the slice:

- [ ] `fr-arom-1991` in `site/src/data/references.json` carries a verified
      description limited to pp. 229–306, and its evidence names the owner's
      scan and how it was checked; no scan is added to the reference archive
- [ ] The published `appendix-references.mdx` has an `fr-arom-1991` entry, so
      every link to that anchor resolves
- [ ] Neither theory-bundle page credits Arom with a claim pp. 229–306 do not
      make, and the test that pinned the F52 citation pins the corrected one

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |
| `guards` | `bash scripts/check-guards.sh` |

**What the scan says, for every task below.** Book page 230, §3.1.3.2: the
pulsation is "an isochronous reference unit" by which a culture measures time,
"the common denominator" of the parts, "only rarely given material existence"
in Central Africa and "usually implicit", though it "can always be given the
material form of handclaps". Nothing on pp. 229–306 speaks of a timeline or of
dancers' footfalls as a referent. Book page 278 gives gàzà's accents at
positions 7, 12, 15 and 3, 9, 14. The PDF page for book page *n* is *n* − 228.

---

## Task 1 — `fr-arom-1991` records what was read

**Row:** CA01. **Consumes:** nothing. **Produces:** the record task 2's
published entry and task 3's citations rest on.

**Files:** `site/src/data/references.json`; a test beside
`site/tests/references-manifest.test.mjs`.

1. Write the failing test first: a case asserting that `fr-arom-1991` has
   `description: "verified"`, and that its `evidence` names pp. 229–306 and
   does not contain "was not read". Run `npm --prefix site test` and watch it
   fail on the current entry.
2. Edit the entry:
   - `description`: `"verified"`
   - `evidence`: the catalogue confirmation already there, plus that the owner's
     scan of Book VI, Chapter 3, pp. 229–306 was read; that the description is
     verified for those pages only; and that pp. 230 and 278 were checked
     against the scan on 2026-10-15
   - `checked`: the date the edit is made
   - `retrieval` stays `not-attempted` and `archiveFile` stays `null`: no open
     copy exists to fetch, and the scan of a copyrighted book is not archived
3. Run `npm --prefix site test` and watch it pass, including every existing
   manifest invariant.
4. Run `format` and `doc-discipline`, append the evidence, tick this box,
   commit with trailers `Plan`, `Slice: M001/S01`, `Rows: CA01`.

## Task 2 — The published bibliography carries Arom

**Row:** CA02. **Consumes:** task 1's record. **Produces:** the anchor
`/appendix-references/#fr-arom-1991` resolves on the published site.

**Files:** `site/src/content/docs/appendix-references.mdx`; the site test that
checks the published bibliography's entries, if one asserts its list.

1. Find where `appendix-references.mdx` lists `fr-` entries and how an entry is
   written (the `<span id="fr-…" data-tier="…">` form). Confirm by grep that no
   `fr-arom-1991` span exists in it today.
2. Write the failing check first, if the site has no test that every
   `/appendix-references/#…` link in `site/src/content/` resolves to a span in
   the published appendix: a case asserting that `fr-arom-1991` is defined
   there. Run it and watch it fail.
3. Add the entry in the section where it belongs: Arom, S. (1991). *African
   Polyphony and Polyrhythm: Musical Structure and Methodology*. Cambridge
   University Press. ISBN 9780521616010, Tier A, *(Purchasable)*, matching the
   theory bibliography's line.
4. Run `site-unit` and `doc-discipline` (its `research-provenance` resolves
   anchors) and watch them pass.
5. Run `format`, append the evidence, tick this box, commit with
   `Rows: CA02`.

## Task 3 — The theory bundle credits Arom only with what pp. 229–306 say

**Row:** CA03. **Consumes:** task 2's resolving anchor. **Produces:** the
slice's last row; the slice closes here.

**Files:** `site/src/content/theory/theory-sub-saharan-africa.mdx`,
`site/src/content/theory/theory-counterpoint-overview.mdx`,
`site/tests/literature-enrichment.test.mjs`.

1. Write the failing test first, in `literature-enrichment.test.mjs` or beside
   it: the sub-Saharan theory page's sentence describing the timeline as a
   "shared clock" carries no `fr-arom-1991` link, and the page's Arom
   citations mention the pulsation. Run it and watch it fail.
2. Edit `theory-sub-saharan-africa.mdx`:
   - **The System**, the timeline sentence: remove the first Arom citation. In
     the next sentence ("Every other part is defined…") remove Arom and keep
     Agawu 2006.
   - **The dance-beat sentence:** keep the phrase "Western transcription
     usually omits". Credit Arom only with the idea that a regular, usually
     unplayed pulsation is the reference for every part (p. 230). Present the
     dance beat as this page's account of how Ewe practice embodies that
     reference, no longer attributed to Arom.
   - **Sources:** "(referent theory)" becomes "(the pulsation as reference
     unit, p. 230)".
3. Edit `theory-counterpoint-overview.mdx`: the sentence crediting the
   *referent* concept to Arom gains "the pulsation as reference unit, p. 230".
   The rest of the sentence stays.
4. Update the F52 case: `near` still finds the methodology sentence; `why`
   says Arom is cited for the pulsation as referent, not for the dance beat.
5. Run `site-unit` and `doc-discipline` and watch them pass.
6. Run `format` and `guards`, append the evidence (each changed sentence
   before and after, and the scan page each rests on), tick this box and the
   Definition of Done, set the slice `done` and rows CA01–CA03 `done` in the
   ledger, run `jk-standards ledger`, and commit with `Rows: CA03`.
