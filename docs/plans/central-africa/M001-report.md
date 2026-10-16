---
class: gated
---

# M001 — Arom is a source the guide has read

> The guide's record of Arom 1991 says what was read and where, and every page
> that cites it credits only what those pages say.

Ledger: `docs/plans/central-africa/ledger.md` · branch
`milestone/M001-arom-record` · 5 commits, **none untraced**

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M001/S01 | Arom is recorded as read | CA01, CA03 | done |

CA02, the published bibliography entry, **moved to M003/S01** during the run.
The site's FR20 rule admits a published entry only once a published page cites
it, and the Central Africa chapter is the first that will. The row's premise
was also wrong: the theory bundle resolves its Arom links against its own
bibliography, so no published link was ever broken.

## What changed

**The record.** `fr-arom-1991` in `site/src/data/references.json` is now
`verified`, and its evidence says for which pages: the owner's scan of Book VI,
Chapter 3, pp. 229–306, with the rest of the book unexamined. The evidence also
names the two pages checked against the scan: p. 230, the pulsation, and
p. 278, the gàzà accents. The scan is not archived, because it is a
copyrighted book, and a test locks `archiveFile: null` and
`retrieval.status: "not-attempted"`.

**The attributions.** The unpublished theory bundle credited Arom with the
timeline as the ensemble's "shared clock" and with the Ewe dance beat as a
referent. Page 230 says neither. It names the *pulsation* (isochronous, usually
implicit, expressible as handclaps) as the reference unit. Now:

- the timeline sentence carries no Arom citation, and the claim is the page's
  own
- the dance-beat sentence credits Arom with the pulsation (p. 230) and presents
  the dance beat as the page's own reading of Ewe practice
- Sources and the counterpoint overview cite "the pulsation as reference unit,
  p. 230"
- row F52 from the theory-audit programme still pins an Arom citation at the
  methodology sentence, with a corrected reason

A test fails if any Arom citation in the theory bundle stops being about the
pulsation at p. 230, or if the timeline sentence cites him again.

## Definition of done

- [x] `fr-arom-1991` in `site/src/data/references.json` carries a verified
      description limited to pp. 229–306, and its evidence names the owner's
      scan and how it was checked; no scan is added to the reference archive
- [x] Neither theory-bundle page credits Arom with a claim pp. 229–306 do not
      make, and the test that pinned the F52 citation pins the corrected one

## Validation

| Token | Result |
|---|---|
| `format` | pass |
| `site-unit` | pass: 402 tests (398 before the milestone) |
| `doc-discipline` | pass |
| `guards` | pass: 24 guard invocations |

Every new test was written first and seen to fail on the unchanged tree; the
evidence file records each failure.

## Traceability

Commits are listed by subject, not SHA: the branch is squash-merged, and a
branch SHA in a durable document stops resolving once it is.

| Commit | Slice | Rows |
|---|---|---|
| docs(plans): plan central-africa M001/S01 | M001/S01 | — |
| fix(references): record fr-arom-1991 as read for pp. 229–306 | M001/S01 | CA01 |
| docs(plans): move CA02 to M003/S01, where Arom is first cited | M001/S01 | — |
| fix(theory): credit Arom with the pulsation, p. 230, and nothing more | M001/S01 | CA03 |
| docs(plans): write the M001 report | M001/S01 | — |

Untraced commits: none.

## Decisions

The contents of `M001-decisions.md`, verbatim:

## 2026-10-15 — planning M001/S01

- **Q:** Approve the M001/S01 design: `fr-arom-1991` marked verified for
  pp. 229–306, with retrieval left `not-attempted` and no archive file; Arom
  added to the published `appendix-references.mdx`; the theory bundle's Arom
  citations reduced to what p. 230 says, with the dance beat kept as the page's
  own account of Ewe practice; the F52 case kept with a corrected reason? —
  **A:** yes
- **Decision:** classified bounded; a plan is written because the slice has
  three tasks — **Why:** each row lands in a different file with its own test,
  and `/jk:next` needs a checklist to resume from
- **Decision:** the scan is not added to the reference archive — **Why:** it is
  a copyrighted book, and the archive holds texts that are free to keep
- **Decision:** `retrieval.status` stays `not-attempted` — **Why:** that field
  records attempts to fetch an obtainable copy; reading the owner's scan is
  recorded in `evidence`, and no other status in the enum describes it
- **Decision:** the attribution correction rests on p. 230 alone, checked
  against the scan on 2026-10-15 — **Why:** it defines the pulsation as the
  reference unit and says it is usually implicit; nothing on pp. 229–306
  mentions a timeline or dancers' footfalls as referent

## 2026-10-15 — halt at M001/S01 task 2: CA02 moved to M003/S01

- **Q:** Task 2 cannot land: `appendix-entry-rules.test.mjs` ("FR20 cited")
  admits an entry to the published bibliography only once a published page
  cites it, and none cites Arom until the M003 chapter. The row's premise was
  also wrong: the theory pages resolve their Arom links against their own
  bibliography, which has the entry. Move CA02 into M003/S01, so the chapter's
  first citation adds the entry? — **A:** yes
- **Decision:** CA02 and its Definition of Done item move to M003/S01; the
  plan's task 2 is removed and task 3 becomes task 2 — **Why:** the entry
  belongs in the commit that first cites it, which is what FR20 enforces;
  task 1 was already committed and stays ticked

## 2026-10-15 — judgment call in M001/S01 task 2

- **Decision:** the planning entry above says nothing on pp. 229–306 mentions a
  timeline or dancers' footfalls as referent. Only p. 230 was read in this
  session; the wider claim comes from the owner's research note (an OCR search
  of all 78 scanned pages). The corrected sentences cite p. 230 alone, so none
  of them depends on the wider claim — **Why:** an entry that implies a full
  reading which did not happen would be the citation failure this programme
  corrects; the evidence states the limit
