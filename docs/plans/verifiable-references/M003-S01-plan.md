# M003/S01 — Scripted retrieval fills what it can

**Slice:** M003/S01 in `docs/plans/verifiable-references/ledger.md`
**Rows:** VR09 (four sources collected by hand, nothing else archived, nothing
records the convention)
**Classification:** bounded. One new script beside the manifest and the
archive helper M002 already built, one new manifest field, and new tests in
`site/tests/` in the style of `references-manifest.test.mjs`. No subsystem is
restructured. Decisions behind every choice below are in `M003-decisions.md`.

## Task status

- [x] 1. The manifest records retrieval state, and the naming convention is code
- [x] 2. The retrieval script, refusing to run without a destination
- [ ] 3. The owner's existing files are matched, renamed and adopted
- [ ] 4. The scripted pass runs, and every refusal is queued

## Definition of Done

Copied verbatim from the slice:

- [ ] Every source classified as a plain fetch is in the archive under the
      `NN - Name.pdf` convention, with the manifest updated to match
- [ ] The retrieval reads its destination from the environment and fails with a
      clear message when unset, rather than writing somewhere arbitrary
- [ ] Sources that resist scripted fetching are recorded as such, not retried
      silently

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `guards` | `bash scripts/check-guards.sh` |

`guards` matters twice here: `check-personal-paths` must stay green while a
script writes into a personal folder, and `check-scripts-readme` fails if the
new script has no entry in `scripts/README.md`.

**The archive check is local only.** CI has no archive, so the existence of
the files is proved by `node scripts/fetch-references.mjs --verify` run with
`POLY_REFERENCES_ARCHIVE` set, and its output goes in the evidence. The
tests prove the shape, the naming and the refusal path; the evidence proves
the files.

The archive path used for every real run in this slice is passed on the
command line for that command alone and never written to a tracked file:

```bash
POLY_REFERENCES_ARCHIVE="$HOME/Library/CloudStorage/Dropbox/Research/drum generator/References" node scripts/fetch-references.mjs …
```

## Shared definitions

These names are used by every task; each is defined by the task named.

- `readBibliography()` — task 1, `site/src/data/references-bibliography.mjs`.
  Returns a `Map` from anchor to `{ text, url }`. Reads
  `site/src/content/docs/appendix-references.mdx` first, then
  `site/src/content/theory/theory-references.mdx`; an anchor present in both
  takes the appendix's text and link. `text` is the entry with the `<span>`
  wrapper, the `**[N]**` label and markdown links removed; `url` is the href
  of the entry's first markdown link, or `null`.
- `archiveFileName(anchor, text)` — task 1, `site/src/data/references-archive.mjs`.
- `RETRIEVAL_STATUS` — task 1, exported from `references-archive.mjs`:
  `['archived', 'script-refused', 'scan-only', 'institution-only', 'no-text', 'not-attempted']`.
- `isChallengePage(html)` — task 2, exported from `scripts/fetch-references.mjs`.
- `selectCandidates(manifest, bibliography, { retry })` — task 2, same file.

## Task 1 — The manifest records retrieval state, and the naming convention is code

**Files:** modify `site/src/data/references.json`,
`site/src/data/references-archive.mjs`,
`site/tests/references-manifest.test.mjs`,
`docs/plans/verifiable-references/browser-worklist.md`; create
`site/src/data/references-bibliography.mjs`,
`site/tests/references-archive-naming.test.mjs`.
**Consumes:** nothing. **Produces:** schema version 2, `RETRIEVAL_STATUS`,
`archiveFileName`, `readBibliography`.

1. In `references-manifest.test.mjs`, change the schema test to expect
   `schemaVersion` 2, add `'retrieval'` to `RECORD_KEYS`, and add a test that
   every record's `retrieval` is an object with exactly `status`, `checked`,
   `detail`; that `status` is in `RETRIEVAL_STATUS` (imported, and the import
   throwing if it is not an array of six); that `checked` is `null` or
   `YYYY-MM-DD`; and that `status === 'archived'` holds exactly when
   `archiveFile` is a string — both directions asserted, each with its own
   message.
2. Create `references-archive-naming.test.mjs` asserting
   `archiveFileName`:
   - `('ref-1', 'Toussaint, G. T. (2005). "The Euclidean…')` → `'01 - Toussaint.pdf'`
   - `('ref-4', '"Rhythm in Sub-Saharan Africa." *Wikipedia*.')` → `'04 - Rhythm in Sub-Saharan Africa.pdf'`
   - `('ref-20', 'Yudane. "Introduction to Balinese Gamelan."…')` → `'20 - Yudane.pdf'`
   - `('ref-46', 'Bjorklund, E. (2003)…')` → `'46 - Bjorklund.pdf'`
   - `('fr-anku-2000', …)` → `'FR - Anku 2000.pdf'`
   - `('fr-polak-london-2014', …)` → `'FR - Polak London 2014.pdf'`
   - a title containing `/`, `:` or `?` has those characters removed and runs
     of spaces collapsed; a Name longer than 80 characters is cut at the last
     space before 80
   - an anchor matching neither `ref-N` nor `fr-…-YYYY` throws.

   And `readBibliography()`: returns more than 100 anchors; `ref-1`'s `url` is
   the appendix's `bridgesmathart.org` link, not the theory bundle's arXiv
   link; `ref-20` (theory bundle only) has the `gamelan.org.nz` URL; an entry
   with no link has `url === null`.
3. Run `npm --prefix site test` and see both files fail: the schema version is
   1, `retrieval` is absent, and the modules do not export the names.
4. Implement. In `references-archive.mjs` add `RETRIEVAL_STATUS` and
   `archiveFileName`: for `ref-N`, the number zero-padded to two digits; the
   Name is the text before the first `,`, `.` or ` (` — unless the text starts
   with a quotation mark, in which case it is the quoted title with its
   trailing period dropped. For `fr-<parts>-<year>`, the Name is each part
   capitalised, joined by spaces, then the year, prefixed `FR - `. Create
   `references-bibliography.mjs` with `readBibliography()` as defined above.
   Migrate `references.json` with a one-off `node -e` (not committed): set
   `schemaVersion: 2`, and give every record `retrieval` —
   `{ status: 'archived', checked: <record's checked>, detail: 'archived before M003; adopted by M003/S01 task 3' }`
   where `archiveFile` is set, else
   `{ status: 'not-attempted', checked: null, detail: null }`. Keep the file's
   existing two-space formatting and trailing newline.
5. Add a `## The archive` section to `browser-worklist.md` stating the
   convention — `NN - Name.pdf` for numbered entries, `FR - Surname Year.pdf`
   for Further Reading, defined by `archiveFileName` — and that the manifest's
   `retrieval.status` says what was retrieved and what cannot be. This closes
   VR09's "nothing records the convention".
6. Run `npm --prefix site test`; all pass. Run `format` and `guards`.
7. Commit with `Rows:` left empty (VR09 closes in task 4).

## Task 2 — The retrieval script, refusing to run without a destination

**Files:** create `scripts/fetch-references.mjs`,
`site/tests/fetch-references.test.mjs`; modify `scripts/README.md`.
**Consumes:** `readBibliography`, `archiveFileName`, `RETRIEVAL_STATUS`,
`resolveArchiveFile`. **Produces:** the script, used by tasks 3 and 4.

1. Write `fetch-references.test.mjs`:
   - spawning `node scripts/fetch-references.mjs --dry-run` with
     `POLY_REFERENCES_ARCHIVE` removed from the environment exits 2, writes
     nothing, and its stderr names `POLY_REFERENCES_ARCHIVE` and says the
     script will not fall back to a default;
   - the same with the variable set to a directory that does not exist exits
     2 and names the path;
   - the same with a whitespace-only value exits 2;
   - with the variable set to a fresh temp directory, `--dry-run` exits 0,
     prints one line per candidate, and leaves both the temp directory and
     `references.json` byte-identical;
   - `isChallengePage` is true for fixtures containing `Anubis`,
     `Just a moment...`, `cf-chl` and `captcha`, false for an ordinary article
     page;
   - `selectCandidates` returns an `open-access` record whose status is
     `not-attempted` and which has a URL or a DOI; skips `archived` records;
     skips `script-refused` records unless `{ retry: true }`; skips every
     obtainability other than `open-access`; uses `https://doi.org/<doi>` when
     the entry has no link but the manifest has a DOI.
2. Run it and watch it fail: the script does not exist.
3. Implement `scripts/fetch-references.mjs` (SPDX header like its
   neighbours; `main()` runs only when the file is executed, so the tests can
   import it):
   - Destination: `process.env.POLY_REFERENCES_ARCHIVE`; unset, blank, or not
     an existing directory → message to stderr and exit 2. No fallback.
   - Flags: `--dry-run` (list candidates, touch nothing), `--retry` (include
     `script-refused`), `--verify` (no fetching: every `archived` record's
     file exists under the archive and equals
     `archiveFileName(anchor, text)`; prints each failure, exits 1 on any).
   - For each candidate, `fetch` with a browser User-Agent and a 30 s
     timeout. Not 200 → `script-refused`, detail `HTTP <code> from <host>`.
     `application/pdf` whose first bytes are `%PDF` → write. `text/html` →
     if `isChallengePage` → `script-refused`, detail
     `bot challenge at <host>`; otherwise render with
     `"/Applications/Google Chrome.app/Contents/MacOS/Google Chrome" --headless=new --disable-gpu --no-pdf-header-footer --print-to-pdf=<tmp> <url>`,
     and keep it only if the output starts with `%PDF` and exceeds 10 KB, else
     `script-refused`, detail `render produced no document`. Anything else →
     `script-refused`, detail `content-type <type>`.
   - Writing: target is `resolveArchiveFile(archiveFileName(...))`. If it
     already exists, do not write and do not mark archived: print
     `exists, not adopted: <name>` and leave the record unchanged.
   - Record: `archived` sets `archiveFile` and `retrieval` with today's date
     and detail `fetched from <host> (<pdf|rendered html>), <bytes> bytes`.
     Every `script-refused` is printed at the end as a list, so nothing is
     refused silently.
   - Rewrite `references.json` with two-space JSON and a trailing newline,
     only when not `--dry-run`.
4. Add the script to `scripts/README.md` under the section closest to site
   data, one line, naming the variable it requires.
5. Run the test file, then `site-unit`, `format`, `guards`; all pass.
6. Commit.

## Task 3 — The owner's existing files are matched, renamed and adopted

**Files:** modify `site/src/data/references.json`,
`site/tests/references-manifest.test.mjs`. The archive itself changes
outside the repository. **Consumes:** tasks 1–2. **Produces:** every file
the owner already holds that matches a bibliography entry, named by the
convention and recorded.

1. Add a test to `references-manifest.test.mjs`: every record with an
   `archiveFile` has `archiveFile === archiveFileName(anchor, text)`, using
   `readBibliography()`. Run it; it fails on the ten existing files whose
   names predate the convention (for example `cohnreich1992.pdf`).
2. Inventory every `.pdf` under the archive, subdirectories included. For
   each, read pages 1–2 with `pdftotext -l 2` and match title and author
   against `readBibliography()`. A match is confirmed only when both the
   title and an author (or, for an authorless entry, the title alone) appear
   on those pages.
3. For each confirmed match: `mv -n` the file to the archive root under
   `archiveFileName` (`-n`, so nothing is overwritten); set `archiveFile`;
   set `retrieval` to `archived`, today's date, detail
   `owner's copy, title page read: "<first line of the title as printed>"`.
   Where several files match one entry (Powers has three), the one whose
   pages are the section the guide cites takes the canonical name and the
   others take `<canonical stem> - part 2.pdf`, `- part 3.pdf`; the manifest
   names the canonical one. Files that match nothing stay exactly where they
   are, unrenamed.
4. Run the conformance test from step 1; it passes. Run
   `--verify` with the archive set; it exits 0.
5. Run `site-unit`, `format`, `guards`.
6. Append to the evidence: every rename as `old → new`, every unmatched file
   by name, and the `--verify` output's count line.
7. Commit.

## Task 4 — The scripted pass runs, and every refusal is queued

**Files:** modify `site/src/data/references.json`,
`docs/plans/verifiable-references/browser-worklist.md`,
`site/tests/references-manifest.test.mjs`, the ledger, the evidence file.
**Consumes:** tasks 1–3. **Produces:** VR09 closed; the slice done.

1. Add a test: every `script-refused` record's anchor is named in
   `browser-worklist.md`'s `## Pending` section. Run it; it passes vacuously
   now (no refusals yet), so prove it non-vacuous by setting one record to
   `script-refused` in a scratch copy and watching it fail, then discard the
   copy. Record both results in the evidence.
2. Run `node scripts/fetch-references.mjs --dry-run` with the archive set;
   read the candidate list.
3. Run it for real.
4. Read pages 1–2 of every file it wrote (`pdftotext -l 2`). A file whose
   first pages are not the work its entry names is deleted from the archive
   and its record set to `script-refused` with detail
   `fetched file was not the cited work: <what it was>` — the script's word
   is not the record. A file that is the cited work stays.
5. Add every `script-refused` anchor to `browser-worklist.md` under a
   `### Refused by M003/S01's scripted pass` heading in `## Pending`, one row
   each: entry, URL, the refusal detail, and **Save as** with
   `archiveFileName`. Run the test from step 1; it passes.
6. Run `--verify`; exits 0.
7. Run `site-unit`, `format`, `guards`.
8. Append to the evidence: candidates, archived count, refused count with
   each refusal's detail, the title-page check result per file, and the
   `--verify` line. Tick the DoD, set VR09 `done` and the slice `done`; run
   `jk-standards ledger`.
9. Commit with `Rows: VR09`.

## Self-review

- **DoD 1** (plain-fetch sources archived under the convention, manifest
  matching) — task 4 steps 3–6; the convention test is task 3 step 1, the
  file existence is `--verify`.
- **DoD 2** (destination from the environment, clear failure when unset) —
  task 2 step 1's three refusal tests, implemented in step 3.
- **DoD 3** (refusals recorded, not retried silently) — `script-refused`
  status (task 1), skipped without `--retry` (task 2), listed at the end of
  every run (task 2), queued in the worklist with a guard (task 4).
- **VR09** — the convention is recorded in task 1 step 5 and enforced by task
  3's test; scripted sources present and listed by task 4.
- **Names** — `readBibliography`, `archiveFileName`, `RETRIEVAL_STATUS`,
  `isChallengePage`, `selectCandidates`, `resolveArchiveFile` used
  consistently; each defined in the task named under Shared definitions.
- **Placeholders** — none; every threshold (30 s, 10 KB, 80 chars) is stated.
