# M003 — decisions

Append-only. Every question `/jk:auto` asked before running, every answer, and
every choice taken on the owner's behalf.

## 2026-09-30 — planning M003/S01 and M003/S02

Measured before asking, on `main` at `6b3bb2c`. The manifest holds 106
records: 34 open-access, 20 browser-only, 26 purchasable, 15 borrowable, 11
library-only. Ten name an `archiveFile`. The owner's Dropbox archive holds 16
top-level items, of which four follow `NN - Name.pdf`; the rest were saved
under whatever name the host gave them (`cohnreich1992.pdf`,
`Locke-PrinciplesOffbeatTiming-1982.pdf`, …), and two Academia.edu bulk
downloads sit in subdirectories. Of the open-access entries, about eight serve
a PDF to a script; the rest are HTML pages. `POLY_REFERENCES_ARCHIVE` is not
set in the session's shell. Headless Chrome and poppler (`pdftotext`) are
installed.

- **Q:** About 25 open-access sources are web pages, not PDFs. How should
  the script archive them? — **A:** "PDF. However, also check [the Dropbox
  archive] for existing pdfs I've downloaded from various official sources.
  All are trustworthy if you can match them to the references in the site."
- **Decision:** web pages are printed to PDF with headless Chrome under the
  same naming convention; and before anything is fetched, every file already
  in the archive, subdirectories included, is matched against the
  bibliography by its title page and adopted into the manifest where it
  matches — **Why:** the owner's answer, and the owner's own precedent:
  `04 - Rhythm in Sub-Saharan Africa - Wikipedia.pdf` is a printed web page.
- **Q:** What should Further Reading entries be called, and should
  hand-saved files be renamed? — **A:** `FR - Surname Year`; rename all.
- **Decision:** numbered entries are `NN - Name.pdf` (two-digit number, Name
  the entry's lead author or, for an authorless entry, its title);
  Further Reading entries are `FR - Surname Year.pdf`, derived from the anchor
  (`fr-polak-london-2014` → `FR - Polak London 2014.pdf`). Every archived
  file that does not follow the convention is renamed, and the manifest
  follows — **Why:** the owner's answer; deriving FR names from the anchor
  makes them deterministic, since anchors are never reassigned.
- **Q:** May the script write directly into the Dropbox archive? — **A:** yes.
- **Decision:** runs set `POLY_REFERENCES_ARCHIVE` to the Dropbox folder for
  the command alone; the script never overwrites an existing file — **Why:**
  the archive is the deliverable, and "never overwrite" makes a re-run safe.
- **Q:** (deferred from M002) Generalise or retire M001/S01's `ref-22`
  worklist guard? — **A:** retire it.
- **Decision:** S02 removes the `nios.ac.in`/`ref-22` case from
  `citation-tier.test.mjs`; the manifest-keyed worklist guard M002 added
  covers the class — **Why:** `ref-22` was replaced, so the guard cannot fire,
  and a check that cannot fire is not a check.

### Taken on the owner's behalf

- **Retrieval state is a new manifest field, and the schema becomes version
  2.** `archiveFile: null` could mean "not tried", "tried and refused" or
  "cannot be archived", and VR10 requires the manifest to tell retrieved from
  unretrievable. Each record gains `retrieval: { status, checked, detail }`
  with `status` one of `archived`, `script-refused`, `scan-only`,
  `institution-only`, `no-text`, `not-attempted`. `archived` holds exactly
  when `archiveFile` is set. Nothing outside `site/tests/` reads the manifest,
  so the version bump breaks no consumer.
- **The script refuses to run without `POLY_REFERENCES_ARCHIVE`.** The
  read-side fallback to `.references/` stays for the tests; a writer that fell
  back would be "writing somewhere arbitrary", which the DoD forbids.
- **A refused entry is not retried unless asked.** `script-refused` records
  are skipped on later runs unless `--retry` is passed, and each one is added
  to the browser worklist in the same change, so "refused" always means
  "queued for a person".
- **Nothing is archived on the script's word alone.** A fetched file is kept
  only if it is a real PDF (`%PDF` magic) or a rendered page that is not a
  bot-challenge stub; then every file adopted or fetched in this milestone has
  its first page read before its record says `archived` — the owner's
  standing rule that a file is checked against its own title page.
- **Where an entry appears in both bibliographies with different links, the
  shipping appendix's link wins.** `ref-1` is the case: the appendix links the
  2005 BRIDGES paper the entry names, the theory bundle links a different
  2007 arXiv paper.
- **Owner-adopted files that match nothing in the bibliography are left
  where they are, untouched and unrenamed.** The two Academia.edu bundles are
  bulk downloads of "more papers by" and "similar papers"; renaming only
  applies to files the manifest names.

### Deferred

- **The browser worklist itself is worked by the owner.** M003/S02 task 2
  is a planned pause: the run stops there, the owner works the worklist in a
  browser, and re-issuing `/jk:auto M003` resumes at task 3 to record what was
  found. Not a question, but a boundary only a person can cross.
