# M003/S02 — Nothing needs an institution

**Slice:** M003/S02 in `docs/plans/first-release/ledger.md`
**Rows:** FR18 (six `library-only` entries), FR19 (107 entries, 40 cited),
FR24 (the verifiable-references ledger is overtaken)
**Depends:** M003/S01 — the mismatches are corrected before the appendix is cut,
so no entry is deleted while still describing the wrong work.
**Classification:** bounded, with a research component. The edits are to one
bibliography and the chapters that cite it; the research is six retrieval
attempts whose outcome the plan cannot predict and does not pretend to.

## Task status

- [x] 1. The six library-only entries are searched for an obtainable copy
- [x] 2. The six are queued for the owner's browser — **planned pause**, resumed and settled 2026-09-22
- [x] 3. The appendix drops to what the shipping pages cite
- [x] 4. The verifiable-references ledger records what this overtook

## Definition of Done

Copied verbatim from the slice:

- [x] No entry in the shipping appendix is `library-only` — none of the 40
      cited entries is; the one uncited entry keeps FR19's exemption, see
      `M003-decisions.md`
- [x] Every claim whose source left carries a source a reader can obtain, or no
      longer asserts something that needs one — no source left; all six were
      settled with an obtainable route
- [x] The appendix contains only entries cited from a shipping page
- [x] The verifiable-references ledger records what this programme changed about
      its remaining milestones

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |

## The six, and what each supports

Measured on this branch. Each is cited from exactly one chapter, and each
supports a claim rather than decorating one.

| Entry | Chapter | What it supports |
|---|---|---|
| `fr-locke-1982` | 02 | the offbeat-timing and cross-rhythm principles the chapter builds on |
| `fr-vitale-1990` | 05 | kotekan interlocking — polos and sangsih each incomplete alone |
| `fr-powers-1980` | 06 | the *New Grove* tala article, offered as where to start |
| `fr-brailoiu-1951` | 07 | **the origin of the term "aksak"** |
| `fr-cohn-1992` | 08 | phase-shifting interference generating more than either pattern contains |
| `fr-harrison-2025` | 13 | the Amen break as the most sampled recording in music history |

## On FR19's number

The row says the appendix holds **34**. That figure is 40 cited minus 6
`library-only`, and it assumes every one of the six is dropped. Under the
owner's decision any entry with an obtainable copy is kept, so the real count is
**34 plus however many survive** — between 34 and 40.

The property FR19 is testing is *every entry is cited from a shipping page*. The
count is reported at close rather than asserted in advance, and S03's guard
tests the property, never the number: a guard pinned to 34 would fail the day an
entry is legitimately added.

---

## Task 1 — The six library-only entries are searched for an obtainable copy

**Consumes:** nothing. **Produces:** a verdict per entry that task 2 acts on.

For each of the six, in this order, stopping at the first that resolves:

1. The DOI, checked for an open-access version at the publisher.
2. The author's own site or institutional repository — Cohn, Locke and Brăiloiu
   are all widely reposted, and a 1951 *Revue de Musicologie* article is old
   enough that scans circulate.
3. `archive.org` — a borrowable scan is obtainable under the programme's own
   rule, as `ref-6` established in M001 of verifiable-references.
4. Nothing found → the entry is not obtainable, and task 2 resolves its claim.

**Fetch what you find.** A search result is not evidence; M002 of
verifiable-references recorded four citations that looked right and pointed at
something else. Record the HTTP status and what the document's own title page
says, exactly as that milestone's manifest does.

Update `site/src/data/references.json` for each: the new `obtainability`, the
`evidence` naming what was consulted, and the `checked` date. The manifest is
the programme's record of what was verified and how, and a retrieval attempt
that leaves no trace is one the next pass repeats.

Write the six verdicts into the evidence file before task 2 begins, so the
research and the editorial decisions it drives are separable in review.

## Task 2 — The six are queued for the owner's browser

**Repaired after task 1 ran.** The step below said that an entry task 1 found
nothing for must have its claim stop depending on it. Task 1 found nothing for
**all six**, and the owner's decision is that "automation found none" is not
"none exists" — so they are queued rather than reworded, and **FR18 stays open**
until the browser settles them.

The rewording path is kept below because it remains correct for whichever
entries the browser does not settle.

1. Add a section to `docs/plans/verifiable-references/browser-worklist.md` — the
   established queue for references a human must settle — naming this milestone
   and listing all six with the specific search each needs. Group by where the
   copy is most likely to be, not by chapter.
2. Record in the evidence that FR18 is paused, not failed, and what unblocks it.
3. Run `format`, `site-unit`, `doc-conformance`, `doc-discipline`. Commit with
   `Slice: M003/S02` and **no `Rows:` trailer**, because this task closes no row.

**When the owner reports**, for each entry: found → repoint the link in
`appendix-references.mdx`, keep the citation exactly as it is, and update the
manifest's `obtainability`; not found → read the citing sentence and make the
claim stop depending on it, without deleting a claim to make a reference go
away. If a claim genuinely needs a source and none is obtainable, that is a
finding to report.

## Task 3 — The appendix drops to what the shipping pages cite

**Consumes:** tasks 1 and 2. **Produces:** the reduced bibliography.

67 of the 107 entries are cited by no shipping page. They are cited by the deep
dives, which carry their own bibliography at
`site/src/content/theory/theory-references.mdx` — so removing them here loses
nothing and breaks nothing.

1. Derive the keep-set by reading the shipping pages, not from any list in this
   plan. An entry is kept when a shipping `.mdx` outside the appendix cites its
   anchor.
2. Remove the rest, and remove any section heading left with no entries.
3. Confirm against the theory bundle that every removed anchor is still defined
   in `theory-references.mdx` — this is the check that the removal is safe, and
   `site/tests/theory-bundle-references.test.mjs` already asserts the other
   direction.
4. Run the full token set and **record the resulting count**. Commit with
   `Slice: M003/S02`, `Rows: FR19`, closing FR19 in the same edit.

## Task 4 — The verifiable-references ledger records what this overtook

**Consumes:** the finished appendix. **Produces:** FR24, and the slice closes.

That programme's M003, M004 and M005 were scoped to 107 entries and a two-list
bibliography that this milestone has just removed.

1. Read its M003, M004 and M005 as they stand.
2. Add a note to each recording what changed and what remains — not a rewrite,
   and not a silent status change. **M004 in particular is largely solved**:
   every Tier C entry in the guide was cited only from the deep dives, which are
   now unpublished, so its VR11 Wikipedia question and VR12 YouTube removals may
   have no subjects left. Verify that against the tree rather than asserting it.
3. Do **not** close rows in another programme's ledger. Recording what overtook
   them is this row's job; deciding what to do about it is that programme's.
4. Run `jk-standards ledger` — it checks all five ledgers, so a malformed note
   fails here.
5. Run the full token set, append the closing evidence, tick every
   definition-of-done box, close FR24, set the slice `done`, and commit with
   `Slice: M003/S02`, `Rows: FR24`.
