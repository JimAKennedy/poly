# M001/S01 — The three measured defects are corrected

**Slice:** M001/S01 in `docs/plans/verifiable-references/ledger.md`
**Rows:** VR01 (`ref-26`), VR02 (`ref-22`), VR03 (`ref-6`)
**Classification:** bounded. Every task edits an existing bibliography file and
extends an existing claim-test harness (`site/tests/citation-tier.test.mjs`,
which already carries `REF9-OLURANTI` in exactly this shape). No new mechanism,
no new token, no interface change.

## Task status

- [x] 1. `ref-6` links to Jones itself, locked by `REF6-JONES`
- [x] 2. `ref-26` becomes an open-access aksak paper, locked by `REF26-AKSAK`
- [x] 3. `ref-22` is measured, recorded, and put on the owner's browser worklist
- [ ] 4. The owner's verdict on `ref-22` is applied and the slice closes

## Definition of Done

Copied verbatim from the slice:

- [ ] `ref-26` and `ref-22` each resolve, or are replaced, or are recorded as
      deliberately removed with the claim they supported rewritten
- [ ] `ref-6` links to Jones (1959) itself, or names plainly that the link is to
      a review and cites the book by ISBN
- [ ] Each correction is locked by a claim test that fails if the old form
      returns

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

All three run on every task. `site-unit` is the one that actually bites here:
`site/package.json` runs `node --test tests/**/*.test.mjs`, which picks up
`citation-tier.test.mjs` with no further wiring.

## What was measured before this plan was written

Recorded because two of the three rows state something the measurement does not
support, and a plan that silently corrected them would hide the correction.

| Ref | Ledger row says | Measured, this machine |
|---|---|---|
| `ref-26` | 404 | **404** — confirmed |
| `ref-22` | "does not respond … the PDF moved" | **Not established.** Nothing on `nios.ac.in` answers: not the PDF, not a sibling chapter a search engine lists as live, not the homepage. DNS resolves to a single A record. The host is unreachable from here; the path may be perfectly good |
| `ref-6` | links to a review | **HTTP 200** — the link works and is a review, which is the defect |

VR02's framing is therefore wrong in a way that matters: "the PDF moved" invites
a replacement, and replacing a legitimate source because one network cannot
reach it is the failure this programme exists to end. Task 3 handles it as the
browser-only class instead.

## Finding for M006 — a liveness check keyed on status codes is already blind

While sourcing the `ref-26` replacement, `emusicology.org` returned **HTTP 200
with an Anubis proof-of-work challenge page** — `<title>Making sure you're not a
bot!</title>`, 4440 bytes of HTML — for a URL whose content type should be PDF.

M006's definition of done says the check must distinguish 403 and 406 from 404
and no-response. That is necessary and not sufficient: this case is a 200 that
carries no document. VR16 should assert something about the response body or
content type, not only its status. Recorded here rather than acted on, because
changing M006's row is outside this slice.

---

## Task 1 — `ref-6` links to Jones itself

**Consumes:** nothing. **Produces:** the `REF6-JONES` case that tasks 2 and 4
sit beside.

`ref-6` cites Jones, A. M. (1959) *Studies in African Music* and links to a
Cambridge review of it. A reader reaches two pages of someone else's opinion of
a book they still cannot read. The book is on archive.org as a borrowable scan:
identifier `studiesinafrican0000amjo`, creator `A.M. Jones`, publisher Oxford
University Press, collections `inlibrary` and `printdisabled` — controlled
digital lending, so a free account reads the actual text.

The DoD's other arm — keep the review and cite the book by ISBN — is not
available: the archive.org record carries no ISBN field, and a 1959 imprint
predates the ISBN system. That is a second reason to take the scan arm.

1. Add a case to the `CLAIMS` array in `site/tests/citation-tier.test.mjs`,
   beside `REF9-OLURANTI` and in the same shape:
   - `id: 'REF6-JONES'`, `file: 'appendix-references.mdx'`
   - `rule`: one line saying the entry must reach the work, not a commentary on
     it, and that the review survives only as a labelled secondary
   - `present: ['Studies in African Music', 'Jones, A. M. (1959)']`
   - `presentRegex: [/archive\.org\/details\/studiesinafrican0000amjo/]`
   - `forbiddenRegex`: a pattern matching a `ref-6` entry whose **only** link is
     the Cambridge review — see step 2 for why this is the load-bearing arm
2. Run `npm --prefix site test`. Watch `REF6-JONES` fail on the missing
   `archive.org` link. A case that passes here is testing nothing: the whole
   point is that the scan link is absent right now.
3. Edit `site/src/content/docs/appendix-references.mdx` line 23 so the entry
   reads: author, year, title, volumes, publisher, then
   `[Borrowable scan](https://archive.org/details/studiesinafrican0000amjo)`,
   then the existing Cambridge link relabelled so it plainly says it is a review
   of the work rather than the work.
4. Run `npm --prefix site test` — `REF6-JONES` passes.
5. **Prove the case bites.** Revert the entry to its old single-link form, run
   the test, confirm it fails naming `REF6-JONES`, then restore. Record both
   the failure message and the restore in the evidence file. A claim test whose
   red state was never seen is not evidence.
6. Run `format`, `site-unit`, `doc-conformance`. Commit with
   `Slice: M001/S01`, `Rows: VR03`.

## Task 2 — `ref-26` becomes an open-access aksak paper

**Consumes:** the harness pattern from task 1. **Produces:** nothing task 4
needs.

`ref-26` is a Fiveable course-marketing study guide and returns 404. Under the
obtainability principle it is a replacement, not a repair. The replacement is
Bonini Baraldi, F., Bigand, E., & Pozzo, T. (2015), "Measuring Aksak Rhythm and
Synchronization in Transylvanian Village Music by Using Motion Capture",
*Empirical Musicology Review* 10(4), 265–291, DOI `10.18061/emr.v10i4.4891` —
the lead article of that journal's aksak special issue, open access, cited by
DOI rather than by a host path so it cannot rot the way `ref-26` did.

Author list, title, volume, issue, pages and DOI all come from Crossref
(`api.crossref.org/journals/1559-5749/works`), not from a search summary.

It is replaced **in place**, keeping the number 26. Deleting the entry would
renumber `ref-27` through `ref-43` across the bibliography and every citing
chapter, and M005 renumbers the whole list again when the two bibliographies
merge — two disruptive renumbers for one weak entry.

The slot's neighbour `fr-goldberg-2015` is already "Timing Variations in Two
Balkan Percussion Performances" from the same issue, so this replacement must
not be the Goldberg paper.

1. Add `REF26-AKSAK` to `CLAIMS`:
   - `forbiddenRegex: [/fiveable\.me/]` — tree-wide on the bibliography, so the
     dead study-guide host cannot return under any entry number
   - `present: ['Measuring Aksak Rhythm and Synchronization', 'Empirical Musicology Review']`
   - `presentRegex: [/10\.18061\/emr\.v10i4\.4891/]`
   - `rule`: one line recording that the predecessor 404'd and was
     course-marketing material, so this is a replacement under the
     obtainability principle
2. Run `npm --prefix site test`. Watch it fail: `fiveable.me` is still present
   and the DOI is absent, so both arms should be red.
3. Edit line 73 of `appendix-references.mdx`. Set `data-tier="A"` — the entry
   moves from a study guide to a peer-reviewed open-access paper, and leaving it
   at `B` would understate it.
4. Run `npm --prefix site test` — passes.
5. **Prove it bites**, both arms: restore the `fiveable.me` URL alone and
   confirm the forbidden arm fails; then remove the DOI alone and confirm the
   present arm fails. Restore. Record both messages in the evidence file.
6. Run `format`, `site-unit`, `doc-conformance`. Commit with
   `Slice: M001/S01`, `Rows: VR01`.

## Task 3 — `ref-22` is measured, recorded, and handed to the owner

**Consumes:** nothing. **Produces:** the worklist task 4 consumes.
**This task ends in a planned pause.** VR02 stays `open`.

> **Repaired after tasks 1 and 2 had already run.** Two steps were wrong and are
> corrected below; the task had not started, so nothing committed is revised.
>
> - Step 2 named the `CLAIMS` harness for an assertion it cannot express.
>   `registerClaimTests` reads exactly one file per claim
>   (`const src = await loadSource(claim.file)`), and this assertion is
>   conditional across two files: its condition is in the bibliography, its
>   consequent is in the worklist. Pointing `claim.file` at the worklist via a
>   relative escape would assert "ref-22 is queued" unconditionally, which goes
>   red the day the entry is legitimately resolved — backwards. It is now a
>   standalone `node:test` case that reads both files itself.
> - Steps 1, 3 and 4 contradicted each other: step 1 created the worklist, step
>   3 expected it absent, step 4 created it again. Creation now follows the red
>   run, which is what test-first requires.

`ref-22` cites a chapter of the National Institute of Open Schooling's
Hindustani Music (242) theory book. NIOS is India's national open-schooling
board and the source is legitimate. What is not established is that the link is
dead — see the measurement table above.

This is the same class as the D-Scholarship@Pitt record behind `ref-9`, which
403s every script and which the owner loaded in a browser and downloaded without
difficulty. VR10 names that class; the owner's browser is the instrument that
settles it.

1. Add a standalone case to `site/tests/citation-tier.test.mjs` — a `test(...)`
   alongside the `registerClaimTests` call, not a `CLAIMS` entry. Name it
   `ref-22 stays queued while its URL is unverified`. It reads both
   `site/src/content/docs/appendix-references.mdx` and
   `docs/plans/verifiable-references/browser-worklist.md` and asserts one
   implication: **if** the bibliography still cites `nios.ac.in`, **then** the
   worklist must name `ref-22`. A missing worklist file fails the same way an
   empty one does.

   The assertion is deliberately not "the URL works". It is that an entry
   nobody could verify has an owner and a place in a queue — because "we could
   not check it" and "we checked it and it was fine" are indistinguishable
   states once the session ends, and only one of them is true here.

2. Run `npm --prefix site test`. Watch it fail: the worklist does not exist yet
   while the bibliography still cites `nios.ac.in`, so the implication is
   violated.

3. Create `docs/plans/verifiable-references/browser-worklist.md` — the file
   M003/S02 later extends rather than invents. Two sections, `## Pending` and
   `## Resolved`, so an entry leaves the queue by moving rather than by being
   deleted and losing its record. One row per URL: ref id, URL, what to check,
   what to save. Seed `## Pending` with `ref-22` alone, plus the measurement
   that put it there — host-wide timeout, DNS resolving to a single A record,
   and a sibling chapter a search engine lists as live timing out identically.

4. Re-run `npm --prefix site test`. Watch it pass.

5. **Prove it bites, both directions.** The second is the one that matters,
   because it is what distinguishes a real implication from an unconditional
   assertion that happens to be satisfied:
   - remove the `ref-22` row from the worklist → **red** (cited but unqueued)
   - restore it, then remove the `nios.ac.in` URL from the bibliography entry
     → **green** (the condition is false, so the obligation lifts)

   Restore the tree after each.

6. Run `format`, `site-unit`, `doc-conformance`. Commit with
   `Slice: M001/S01`, `Rows: VR02` — the row does **not** close here.

7. **Stop and ask the owner** to load
   `https://nios.ac.in/media/documents/Hindustani_Music_242/hindustanimusictheorybook1/HMB1Ch3.pdf`
   in a browser and report one of: it loads (save the PDF to the Dropbox
   archive), it 404s, or the host is down for them too. Do not proceed to
   task 4 without that answer, and do not guess it.

## Task 4 — apply the owner's verdict and close the slice

**Consumes:** the owner's answer from task 3. Both outcomes are specified below;
neither is a judgement call left to the executor.

> **Repaired alongside task 3.** The old step 2 said to remove the `ref-22` row
> from the worklist, which under the repaired task 3 would go red: keeping the
> `nios.ac.in` URL keeps the implication's condition true, so the obligation to
> name `ref-22` survives. The row now **moves to `## Resolved`**, which both
> satisfies the test and keeps the record of what was checked and when.

**Third outcome, taken.** The owner identified `ref-22` as course material —
the National Institute of Open Schooling's teaching text, not scholarship. That
is the class the Fiveable entry was removed for in task 2, and it settles the
entry on editorial grounds without the liveness question being asked at all. The
browser check is moot: whether a teaching PDF loads does not make it a citable
source. The two branches below are retained because they remain correct for the
next entry of this shape, and because deleting the branch that was not taken
would hide that a choice existed.

The replacement is Clayton, M. (2020), "Theory and Practice of Long-form
Non-isochronous Meters: The Case of the North Indian *rūpak tāl*", *Music Theory
Online* 26(1), DOI `10.30535/mto.26.1.2`. Verified three ways before being
proposed: the publisher's article page, the served HTML (190 KB, 112 occurrences
of *rūpak*), and Crossref, with the DOI resolving to the article. Peer-reviewed,
platinum open access, freely readable without login. Clayton is already the
chapter's primary authority (`fr-clayton-2000`), and the article engages London
2012 on non-isochronous meter, which the guide cites as `fr-london-2012`.

1. Add `REF22-CLAYTON` to `CLAIMS`: `forbiddenRegex` for `nios.ac.in`, tree-wide
   so the teaching text cannot return under another number; `present` for the
   title and journal; `presentRegex` for the DOI.
2. Run `npm --prefix site test`, watch it fail on both arms.
3. Replace the entry at `data-tier="A"`, re-run, watch it pass.
4. Move the `ref-22` row from `## Pending` to `## Resolved` in the worklist,
   recording that it was settled editorially rather than by the browser check,
   so the queue does not carry a task nobody needs to do.
5. Prove the queue test still holds: with `nios.ac.in` gone the implication's
   condition is false, so it passes for the right reason rather than by
   accident. Confirm by re-adding the string alone and watching it go red.

**If the PDF loads for the owner** — `ref-22` is sound and only unreachable from
automation:

1. Leave the URL as it is. Add a `REF22-NIOS` entry to `CLAIMS`, whose `rule`
   records that the entry was verified by hand on a stated date, that the host
   refuses automated fetches, and that the PDF is in the archive. This is a
   single-file claim on `appendix-references.mdx`, which the harness expresses
   without difficulty.
2. Move the `ref-22` row from `## Pending` to `## Resolved` in the worklist,
   with the date and the outcome. The standalone test stays green because the
   row is still named; the queue test and the claim test then say different
   things — one that the entry is accounted for, one that it is correct.

**If it 404s for the owner too** — the path really has rotted, and the ledger's
original framing was right after all:

1. Replace the URL with the live NIOS path for the same course the owner
   reached, or, if no NIOS path is reachable, replace the entry with a
   DOI-citable source on Hindustani tala and record in the `rule` why the
   NIOS entry was dropped.
2. Add `REF22-NIOS` to `CLAIMS` with a `forbiddenRegex` for the dead path,
   matching how `REF9-OLURANTI` forbids the rotted D-Scholarship path.
3. Move the `ref-22` row to `## Resolved` with the date and the outcome. If the
   replacement removed `nios.ac.in` from the bibliography entirely, the queue
   test's condition is now false and it passes either way — the move is for the
   record, not for the gate.

Then, in either case:

4. Prove the final case bites, as in tasks 1 and 2.
5. Append the closing entry to `docs/plans/verifiable-references/evidence/M001-S01.md`.
6. Set VR02 to `done`, tick all three definition-of-done boxes, and set the
   slice `done` in the ledger.
7. Run `jk-standards ledger`, then `format`, `site-unit`, `doc-conformance`.
   Commit with `Slice: M001/S01`, `Rows: VR02`.
