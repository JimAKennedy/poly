# M001 — The known defects are fixed

**Review gate report.** Generated from the ledger, `git log`, and
`M001-decisions.md`. `/jk:ship` is the next step and it is the owner's to take.

**Vision:** The references this audit already proved broken — two dead links and
one citing a review instead of the work — are corrected and locked.

**Branch:** `milestone/M001-known-defects`
**Ledger:** `docs/plans/verifiable-references/ledger.md`

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M001/S01 | The three measured defects are corrected | VR01, VR02, VR03 | done |

## Definition of done

- [x] `ref-26` and `ref-22` each resolve, or are replaced, or are recorded as
      deliberately removed with the claim they supported rewritten
- [x] `ref-6` links to Jones (1959) itself, or names plainly that the link is to
      a review and cites the book by ISBN
- [x] Each correction is locked by a claim test that fails if the old form
      returns

## What changed

| Ref | Was | Is |
|---|---|---|
| `ref-6` | Jones (1959), linked to a Cambridge **review** of the book | Linked to the borrowable archive.org scan, review kept as a labelled secondary |
| `ref-22` | NIOS Hindustani Music (242) teaching text, Tier B | Clayton (2020), *Music Theory Online* 26(1), DOI `10.30535/mto.26.1.2`, Tier A |
| `ref-26` | Fiveable study guide, 404, Tier B | Bonini Baraldi, Bigand & Pozzo (2015), *EMR* 10(4), DOI `10.18061/emr.v10i4.4891`, Tier A |

Two entries rose from Tier B to Tier A. No entry was renumbered: all three were
replaced in place, so `ref-27`–`ref-43` and every citing chapter are untouched.

## Validation

From the slice's evidence file, at the head of each task's commit.

| Token | Command | Task 1 | Task 2 | Task 3 | Task 4 |
|---|---|---|---|---|---|
| `format` | `pre-commit run --all-files` | 0 | 0 | 0 | 0 |
| `site-unit` | `npm --prefix site test` | 307/307 | 308/308 | 309/309 | 310/310 |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | 281/281 | 282/282 | 283/283 | 284/284 |

`format` returned 1 on task 4's first run — `end-of-file-fixer` added a final
newline to `browser-worklist.md` — and 0 on re-run.

Every claim test was seen red before its correction, and each was mutated to
prove it bites. Eight mutations across the four tasks, each applied, tested and
reverted with the tree confirmed byte-identical afterwards.

## Traceability

Every commit on the branch carries `Slice: M001/S01`. **No untraced commits.**

| Commit | Rows | Subject |
|---|---|---|
| `7ead887` | — | plan M001/S01, and correct what VR02 claims |
| `5ebcbea` | VR03 | point ref [6] at Jones rather than a review of Jones |
| `c1da70b` | — | record M001's decisions and mark the milestone in-progress |
| `db8b497` | VR01 | replace ref [26] with an open-access aksak paper |
| `e2ca78e` | — | repair two wrong steps in M001/S01 task 3 |
| `72f2a00` | VR02 | queue ref [22] for a human instead of guessing at it |
| `39cbafb` | — | repair task 4 for the outcome actually taken, seed ref-23 for M002 |
| `1f0e077` | VR02 | replace ref [22] with Clayton on rupak tal |

VR02 appears twice by design: `72f2a00` queued the entry without closing the
row, and `1f0e077` closed it once the owner's verdict arrived.

Three of the eight commits are plan and decision maintenance rather than
content. That ratio is itself a finding: two of them are repairs to a plan
written in this same session, described under "For a reviewer" below.

## What a reviewer should look at twice

### Two plan repairs, one of which was caused by the other

`e2ca78e` fixed two wrong steps in task 3: the plan named the `CLAIMS` harness
for an assertion it cannot express — `registerClaimTests` reads exactly one file
per claim, and the assertion spans two — and its steps contradicted each other
on when the worklist is created. `39cbafb` then repaired task 4, which the first
repair had silently broken: task 4 deleted a worklist row that the repaired test
still required. The post-repair self-review is what caught it, which is the
argument for that step being mandatory rather than optional.

### A dominated guard, recorded and not resolved

Task 4 measured something the plan asserted wrongly. The plan expected the queue
test to go red if `nios.ac.in` returned; it does not, because the worklist still
names `ref-22` in its Resolved section.

| mutation | `REF22-CLAYTON` | `ref-22 stays queued` |
|---|---|---|
| `nios.ac.in` returns | red | green |
| `nios.ac.in` returns and `ref-22` is struck from the worklist | red | red |

The queue test is not dead — it fires — but every failure it detects is one
`REF22-CLAYTON` detects first, because that case forbids the host tree-wide. It
can never be the only signal. Whether to generalise it into the worklist
mechanism M003/S02 needs, or retire it, is a decision this task declined to take
alone.

### A finding seeded into M002, not fixed here

`ref-23` appears to name both the wrong title and the wrong journal: the article
at its URL looks to be "Indian Rhythmic Systems as Sources of Inspiration for…"
in *Analytical Approaches to World Music* 11(2), while the entry claims "…in
Comparative Perspective" in the "Journal of the International Folk Art and World
Music Society" — a name that reads like a guessed expansion of "iftawm". It is
seeded as row VR17 in M002/S02 and **is not fixed**, because `iftawm.org` resets
every automated connection, so the real title needs a browser either way.

### The milestone's premise did not survive contact with the evidence

M001 was cut to fix "two dead links and one citing a review". Of the three:

- `ref-26` was a genuine 404 — one real dead link
- `ref-6` returned **200** throughout. It was never dead; it pointed at a review
  of the book instead of the book
- `ref-22` was never shown to be dead either. It was replaced because it is
  course material, an editorial judgement, after the owner recognised it

So link-checking would have caught **one of three**. Counting `ref-2`'s
fabricated title, `ref-9`'s wrong subject, `ref-6`'s surrogate link, `ref-23`'s
suspected mislabelling, and an Anubis challenge page served as HTTP 200 with no
document, that is five defects found where liveness was green or irrelevant,
against one found by a dead URL. M002 is the milestone that reads sources rather
than pinging them, and this is the evidence for weighting it accordingly.

## Decisions

Verbatim from `M001-decisions.md`:

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
