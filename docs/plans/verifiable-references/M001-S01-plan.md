# M001/S01 — The three measured defects are corrected

**Slice:** M001/S01 in `docs/plans/verifiable-references/ledger.md`
**Rows:** VR01 (`ref-26`), VR02 (`ref-22`), VR03 (`ref-6`)
**Classification:** bounded. Every task edits an existing bibliography file and
extends an existing claim-test harness (`site/tests/citation-tier.test.mjs`,
which already carries `REF9-OLURANTI` in exactly this shape). No new mechanism,
no new token, no interface change.

## Task status

- [ ] 1. `ref-6` links to Jones itself, locked by `REF6-JONES`
- [ ] 2. `ref-26` becomes an open-access aksak paper, locked by `REF26-AKSAK`
- [ ] 3. `ref-22` is measured, recorded, and put on the owner's browser worklist
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

`ref-22` cites a chapter of the National Institute of Open Schooling's
Hindustani Music (242) theory book. NIOS is India's national open-schooling
board and the source is legitimate. What is not established is that the link is
dead — see the measurement table above.

This is the same class as the D-Scholarship@Pitt record behind `ref-9`, which
403s every script and which the owner loaded in a browser and downloaded without
difficulty. VR10 names that class; the owner's browser is the instrument that
settles it.

1. Create `docs/plans/verifiable-references/browser-worklist.md` — the file
   M003/S02 later extends rather than invents. One row per URL: the ref id, the
   URL, what to check, and what to save. Seed it with `ref-22` alone, plus the
   measurement that put it there (host-wide timeout, DNS resolving, a sibling
   chapter indexed as live timing out identically).
2. Add `REF22-BROWSER-PENDING` to `CLAIMS`, asserting that while `ref-22` still
   carries the `nios.ac.in` URL, the worklist file names `ref-22`. This is what
   stops the entry being quietly forgotten in the state "we could not check it":
   the assertion is not that the URL works, but that an unresolved entry has an
   owner and a place in the queue.
3. Run `npm --prefix site test`. Watch it fail before the worklist exists.
4. Create the worklist, re-run, watch it pass. **Prove it bites**: remove the
   `ref-22` row from the worklist, confirm the case fails, restore.
5. Run `format`, `site-unit`, `doc-conformance`. Commit with
   `Slice: M001/S01`, `Rows: VR02` — the row does **not** close here.
6. **Stop and ask the owner** to load
   `https://nios.ac.in/media/documents/Hindustani_Music_242/hindustanimusictheorybook1/HMB1Ch3.pdf`
   in a browser and report one of: it loads (save the PDF to the Dropbox
   archive), it 404s, or the host is down for them too. Do not proceed to
   task 4 without that answer, and do not guess it.

## Task 4 — apply the owner's verdict and close the slice

**Consumes:** the owner's answer from task 3. Both outcomes are specified below;
neither is a judgement call left to the executor.

**If the PDF loads for the owner** — `ref-22` is sound and only unreachable from
automation:

1. Leave the URL as it is. Replace `REF22-BROWSER-PENDING` with `REF22-NIOS`,
   whose `rule` records that the entry was verified by hand on a stated date,
   that the host refuses automated fetches, and that the PDF is in the archive.
2. Remove the `ref-22` row from the worklist, which turns the
   `REF22-BROWSER-PENDING` assertion off by satisfying it rather than deleting
   the mechanism.

**If it 404s for the owner too** — the path really has rotted, and the ledger's
original framing was right after all:

1. Replace the URL with the live NIOS path for the same course the owner
   reached, or, if no NIOS path is reachable, replace the entry with a
   DOI-citable source on Hindustani tala and record in the `rule` why the
   NIOS entry was dropped.
2. Rename the case `REF22-NIOS` and give it a `forbiddenRegex` for the dead
   path, matching how `REF9-OLURANTI` forbids the rotted D-Scholarship path.

Then, in either case:

3. Prove the final case bites, as in tasks 1 and 2.
4. Append the closing entry to `docs/plans/verifiable-references/evidence/M001-S01.md`.
5. Set VR01, VR02, VR03 to `done`, tick all three definition-of-done boxes, and
   set the slice `done` in the ledger.
6. Run `jk-standards ledger`, then `format`, `site-unit`, `doc-conformance`.
   Commit with `Slice: M001/S01`, `Rows: VR02`.
