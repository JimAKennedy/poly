# M003/S02 — The browser worklist finishes the archive

**Slice:** M003/S02 in `docs/plans/verifiable-references/ledger.md`
**Rows:** VR10 (scholarly hosts refuse scripts; "a script cannot fetch it" is
not "unobtainable")
**Classification:** bounded, and mostly editorial. The worklist exists and
is guarded already (M002); this slice gives every pending row an exact
filename, has the owner work it, and records the outcome in the field
M003/S01 added. Decisions are in `M003-decisions.md`.

## Task status

- [x] 1. Every pending row names its filename, and the dead guard is retired
- [x] 2. The owner works the worklist (planned pause)
- [ ] 3. The results are recorded, and nothing open is left unaccounted for

## Definition of Done

Copied verbatim from the slice:

- [ ] A worklist names every browser-only source: URL, what to save, and the
      exact filename to save it as
- [ ] The worklist has been worked and the archive contains its results
- [ ] Sources that are free but only as unsearchable scans, or free only to an
      institution, are recorded as what they are rather than as archived

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |

As in S01, the files' presence is proved locally by
`node scripts/fetch-references.mjs --verify` with `POLY_REFERENCES_ARCHIVE`
set, and the output goes in the evidence.

## Task 1 — Every pending row names its filename, and the dead guard is retired

**Files:** modify `docs/plans/verifiable-references/browser-worklist.md`,
`site/tests/references-manifest.test.mjs`, `site/tests/citation-tier.test.mjs`,
`site/src/data/references.json`.
**Consumes:** `archiveFileName`, `readBibliography` (S01 task 1).

1. Add a test to `references-manifest.test.mjs`: parse the tables under
   `## Pending` in `browser-worklist.md`; every row whose first cell is an
   anchor in backticks has a **Save as** cell equal to
   `` `archiveFileName(anchor, text)` ``, and a non-empty URL and
   what-to-check cell. A row whose URL is `— none in the entry —` must say in
   its what-to-check cell where to look. Run it; it fails — no row has a
   Save as column yet.
2. Add the **Save as** column to every Pending table. For any Pending row
   whose anchor S01 already archived (task 3 adopted the owner's copy — for
   example `ref-30` if the Academia.edu file matched), move the row to
   `## Resolved` with its outcome rather than asking the owner to fetch it
   again; update that record's `description` only if the title page S01 read
   is what the worklist row asked to confirm, and say so in `evidence`.
3. YouTube rows (`ref-10`, `ref-11`, `ref-14`, `ref-15`, `ref-25`) have no
   text to archive: their Save as cell reads `— no text; record existence only —`,
   and the test accepts exactly that string for a record whose obtainability
   is browser-only and whose URL host is `youtube.com`.
4. Retire the `ref-22`/`nios.ac.in` test in `citation-tier.test.mjs`. Before
   deleting it, confirm it is vacuous: `nios.ac.in` appears nowhere under
   `site/src/content/`. Record that grep's output in the evidence.
5. Run `site-unit` and `format`; both pass.
6. Commit with `Rows:` empty.

## Task 2 — The owner works the worklist (planned pause)

This task is the owner's. **The executor stops here** and reports: the
number of pending rows, grouped by host as the worklist groups them, and the
archive path to save into. The owner saves each file under its Save as name
and notes, per row, one of: found and saved; free only as an unsearchable
scan; free only through an institution; a video that exists (with its actual
title and uploader); not found.

On resumption, the owner's per-row answers are appended to
`M003-decisions.md` under `## <date> — the worklist, as worked`, verbatim, and
committed with this task's box ticked and `Rows:` empty. Nothing else changes
in this commit: the record of what the owner found lands before anything is
derived from it.

## Task 3 — The results are recorded, and nothing open is left unaccounted for

**Files:** modify `site/src/data/references.json`,
`docs/plans/verifiable-references/browser-worklist.md`,
`site/tests/references-manifest.test.mjs`, the ledger, the evidence file.
**Consumes:** task 2's recorded answers.

1. Add a test: every record whose obtainability is `open-access` or
   `browser-only` has `retrieval.status` in `archived`, `scan-only`,
   `institution-only`, `no-text`. Run it; it fails on every row task 2
   settled but the manifest does not yet reflect.
2. For each "found and saved" row: confirm the file exists under its Save as
   name, read pages 1–2 with `pdftotext -l 2`, and set `archiveFile` and
   `retrieval: archived` with detail `owner, browser session <date>; title page read: "<title as printed>"`.
   A file whose title page is not the cited work is not archived: say so in
   the record and in the worklist row, and halt the run — that is a
   question for the owner, not a judgment call.
3. Scan-only → `scan-only`; institution-only → `institution-only`; video →
   `no-text`, with the actual title and uploader in `detail`. Each records the
   owner's words in `detail`, and none sets `archiveFile`.
4. "Not found" is not one of the allowed end states. If any row is not
   found, the test from step 1 stays red: halt, and report which entries need
   a decision (M004 removes or replaces them, but this slice may not claim to
   be done while they are open).
5. Move every settled row to `## Resolved` under a heading for this pass,
   with its outcome.
6. Run the test from step 1 and the worklist test from task 1; both pass.
   Run `--verify` with the archive set; exits 0.
7. Run `site-unit` and `format`.
8. Append to the evidence: counts by outcome, each title-page check, the
   `--verify` line. Tick the DoD, set VR10 `done` and the slice `done`; run
   `jk-standards ledger`.
9. Commit with `Rows: VR10`.

## Self-review

- **DoD 1** (worklist names every browser-only source with URL, what to save,
  exact filename) — task 1 steps 1–3, with the test enforcing all three
  cells.
- **DoD 2** (worklist worked, archive contains results) — task 2, then task 3
  steps 2 and 6 (`--verify`).
- **DoD 3** (scan-only and institution-only recorded as what they are) — task
  3 step 3, enforced by the step 1 test, which admits those statuses and not
  `not-attempted`.
- **VR10** — the manifest distinguishes retrieved (`archived`) from
  unretrievable (`scan-only`, `institution-only`, `no-text`) by task 3.
- **Names** — `archiveFileName`, `readBibliography`, `retrieval.status`
  values match S01's `RETRIEVAL_STATUS`.
- **Placeholders** — none. Task 2 is an owner action by design, and says so.
