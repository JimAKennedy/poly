# M001 — decisions

Append-only. One entry per decision that shaped the milestone, with the reason,
so a reviewer can see what was chosen on the owner's behalf and what the owner
chose themselves.

## 2026-09-20 — planning M001/S01

Three questions were put to the owner before the plan was written, each with the
measurement that made it answerable.

- **Q:** `ref-26` (Fiveable) is a confirmed 404 and is course-marketing
  material, so it goes. Removing it outright would renumber refs 27–43 across
  the site; replacing it in place costs nothing. What replaces it? —
  **A:** Bonini Baraldi, Bigand & Pozzo (2015), *Empirical Musicology Review*
  10(4), 265–291, cited by DOI.
- **Decision:** replace in place, keeping the number 26, and raise the entry
  from Tier B to Tier A — **Why:** deleting it would renumber the bibliography
  and every citing chapter, and M005 renumbers the whole list again when the two
  bibliographies merge; two disruptive renumbers for one weak entry.

- **Q:** `ref-22` is not a dead link. Nothing on `nios.ac.in` answers from this
  machine — not the PDF, not a sibling chapter a search engine lists as live,
  not the homepage — while DNS resolves. It is the same browser-only class as
  the owner's D-Scholarship@Pitt link. How should the plan handle it? —
  **A:** browser worklist; hold VR02 open until the owner reports.
- **Decision:** task 3 records the measurement and queues the URL; VR02 does not
  close there — **Why:** replacing a legitimate national-education source on
  evidence that only proves one network cannot reach it is the failure this
  programme exists to end.

- **Q:** `ref-6` links to a Cambridge review of Jones (1959) instead of the
  book, which is on archive.org as a borrowable scan. What should the entry
  become? — **A:** link the scan as primary, keep the review as a labelled
  secondary.
- **Decision:** taken, and the definition of done's ISBN arm was not used —
  **Why:** not a preference. The archive.org record carries no ISBN and a 1959
  imprint predates the ISBN system, so that arm was unavailable.

### Deferred

- **Q:** Does
  `https://nios.ac.in/media/documents/Hindustani_Music_242/hindustanimusictheorybook1/HMB1Ch3.pdf`
  load in the owner's browser? — **Deferred to:** the end of task 3, which is a
  declared pause. Task 4 specifies both outcomes in full, so the answer selects
  a branch rather than requiring a fresh decision.

### Corrections made during planning

- VR02's ledger text said `ref-22` "does not respond" and that "the PDF moved
  rather than the source being bad". Re-measurement supports neither. The plan
  records the correction in its own measurement table rather than silently
  planning around it.
- VR10's ledger text claimed the bot-blocked entries "load fine in a browser".
  Five 403s and one 406 had been measured; no browser load had. The owner
  confirmed one by hand (the Pitt record behind `ref-9`), and the row was
  amended before the ledger merged to say which part is evidence and which is
  inference.

## 2026-09-20 — repairing M001/S01 task 3

- **Q:** Task 3's step 2 named the `CLAIMS` harness for an assertion it cannot
  express — a conditional across two files, where `registerClaimTests` reads
  exactly one. Approve writing it as a standalone `node:test` case instead? —
  **A:** approved.
- **Decision:** the queue assertion becomes a standalone test in the same file,
  under the same `site-unit` token — **Why:** only the mechanism was wrong. The
  intent, the file, and the gate are unchanged, and the relative-path dodge
  would have asserted the obligation unconditionally, going red the day the
  entry is legitimately resolved.

- **Decision:** the worklist gains `## Pending` and `## Resolved` sections, and
  task 4 moves the row between them rather than deleting it — **Why:** found
  while re-running the self-review the repair requires. Deleting the row would
  go red under the repaired test whenever the URL is kept, which is exactly the
  "PDF loads" branch; moving it keeps the gate green and keeps the record of
  what was checked and when.

- **Decision:** task 3's steps were reordered so the worklist is created after
  the red run — **Why:** a second defect found by the same self-review. Step 1
  created the file, step 3 expected it absent, step 4 created it again. No
  executor could have followed all three.

## 2026-09-20 — resolving M001/S01 task 4

- **Q:** (the deferred browser question, superseded) Does the NIOS PDF load? —
  **A:** not asked. The owner identified `ref-22` as course material, which
  settles the entry without the liveness question.
- **Decision:** `ref-22` is replaced on editorial grounds, not liveness grounds
  — **Why:** it is the National Institute of Open Schooling's teaching text, the
  same class as the Fiveable entry removed in task 2. Whether a teaching PDF
  loads does not make it a citable source, so the browser check was moot and was
  not spent.

- **Q:** Which open-access tala source replaces it? The owner asked for a
  peer-reviewed open-access paper to be found before choosing. — **A:** Clayton,
  M. (2020), "Theory and Practice of Long-form Non-isochronous Meters: The Case
  of the North Indian *rūpak tāl*", *Music Theory Online* 26(1),
  DOI `10.30535/mto.26.1.2`.
- **Decision:** taken — **Why:** peer-reviewed, platinum open access, freely
  readable without login, DOI-cited, and on North Indian tāl, which is the slot
  `ref-22` occupied. Clayton is already the chapter's primary authority
  (`fr-clayton-2000`) and the article engages London 2012, cited as
  `fr-london-2012`. Verified against the publisher page, the served HTML and
  Crossref rather than a search summary.

- **Decision:** neither file the owner supplied from Scribd was used — **Why:**
  "Fundamentals of Indian Music" is Dr. Swatantra Sharma's student textbook, an
  image-only scan with no text layer behind an account wall — the same class
  being removed. The "Rowell" file is a seven-page browser printout of the Open
  Library catalogue page, not the book.

- **Q:** `ref-23` appears to name both the wrong title and the wrong journal.
  Fix it here or record it? — **A:** record it for M002.
- **Decision:** seeded as row VR17 in M002/S02, and VR07 updated to note a
  fourth instance of its class — **Why:** M002 is the pass built for exactly
  this verdict, and the correct title cannot be read from here: iftawm.org
  resets every automated connection, so confirming it needs a browser either
  way. Fixing it in M001 would widen a slice past the three measured defects its
  milestone vision names.
