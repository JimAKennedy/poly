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
