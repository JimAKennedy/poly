# M003/S02 — Chapter 6 scope

**Slice:** M003/S02
**Ledger:** `docs/plans/theory-audit/ledger.md`

Chapter 6 promises "Hindustani and Carnatic" and delivers Hindustani. The word
*Carnatic* appears exactly once in the chapter — in the description making the
promise. This slice makes the scope claim true and says plainly what is absent.

## Task status

The executable state of this plan. `/jk:next` reads the first unchecked box
here to decide what to do; the task's own commit ticks it.

- [x] Task 1 — Scope Chapter 6 to Hindustani and state the Carnatic absence (F25)

## Definition of Done

Copied verbatim from the slice. The task below argues against *this* text.

- [x] Chapter 6's front-matter description and its in-page scope note both say
      Hindustani, and neither promises Carnatic coverage
- [x] The absence of the Carnatic tala system is stated rather than implied

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

Run `node --test site/tests/scope-framing.test.mjs` in the inner loop.

**Gate ordering.** `format` runs the `ledger` pre-commit hook, which fails while
a slice claiming `done` has no evidence file. This task closes the slice, so the
order is: `site-unit` and `doc-conformance` first, then the evidence, then
`format`.

**Validate after `git add`.** `jk-standards` does not enumerate untracked files
under a doc root, so a check run before staging proves nothing about the tree
you are about to push (jk-standards#96). M003/S01 hit this and caught it only
because its plan said so.

## Context

**The overclaim sits in three places, and the Definition of Done names two.**

| Site | Today | Action |
|---|---|---|
| `06-indian-classical.mdx` front matter, line 4 | `description: The tala system of Hindustani and Carnatic music — …` | rewrite |
| `06-indian-classical.mdx` body | no scope note exists | create |
| `theory-counterpoint-overview.mdx` line 61 | table cell `Hindustani and Carnatic rhythm` | rewrite |

The third is outside the Definition of Done's wording and is in scope by
decision: it is the same claim, one table cell, in the index a reader uses to
choose a deep dive. Left alone, the guide advertises Carnatic rhythm one click
from the page that says it is absent.

**The chapter really does contain no Carnatic material.** `Carnatic` occurs once
in the file — in the description. There is no konnakol, no solkattu, no mention
of the Carnatic tala families. This is a false promise to remove, not a body of
content to relabel.

**Scope the chapter, not the guide.** M004/S04 will add a tihai worked example
using Nelson (2008), a Carnatic source, to `theory-indian-classical.mdx` — the
companion page, not this chapter. So the scope note must say what *this chapter*
covers. A note claiming the guide contains no Carnatic material anywhere would
be wrong the moment M004/S04 lands, and would also contradict the appendix,
which already lists Nelson's *Solkattu Manual* in Further Reading.

**The About page already points here.** M003/S01 wrote, under "What this guide
does not cover", that the Carnatic absence "is stated in [Chapter 6] rather than
left to be inferred from what is missing". That sentence is a forward reference
until this slice lands — it links the page, not an anchor, so nothing is broken,
but the claim it makes about Chapter 6 only becomes true here.

**Where the scope note goes.** The chapter opens with a paragraph, then a
`:::note[Theory deep dive]` admonition pointing at the companion page, then
`## Tala as Rhythmic Architecture`. The scope note belongs immediately after the
opening paragraph and before that admonition, so a reader meets the scope before
being sent elsewhere.

**Four existing locks live in this file** — `S02-F01` (rupak illustration),
`S02-F12` (theka framing) and `S06-F16-indian` (the lcm translation caveat) in
`theory-audit-claims.test.mjs`, and `S04-F20` (tier citations) in
`citation-tier.test.mjs`. None constrains the description or the chapter
opening, but step 6 runs `site-unit` immediately and names them, because a slice
that quietly broke a previous milestone's lock would be the worst outcome here.

**Case-id note.** This case is `S02-F25`. M001's host already carries `S02-F01`
and `S02-F12` — its own slice S02. The prefix does not encode the milestone, so
the F-number is what disambiguates; `S02-F25` is unique across the ledger. Do
not rename the M001 cases to make the scheme tidier.

**The lock host** is `site/tests/scope-framing.test.mjs`, created by M003/S01
and already wired into `scripts/check-doc-conformance.sh` and the `REQUIRED`
array. Append to its `CLAIMS`; no wiring work is needed.

**Reverting a test mutation** while a file holds this task's uncommitted work:
use an inverse edit, not `git checkout --`.

## Task 1 — Scope Chapter 6 to Hindustani and state the Carnatic absence

**Modifies:** `site/src/content/docs/06-indian-classical.mdx`,
`site/src/content/docs/theory-counterpoint-overview.mdx`,
`site/tests/scope-framing.test.mjs`, `docs/plans/theory-audit/ledger.md`
**Creates:** `docs/plans/theory-audit/evidence/M003-S02.md`
**Rows:** F25

### Steps

1. **Write the failing case.** Add to the `CLAIMS` array in
   `site/tests/scope-framing.test.mjs`:

   - `id: 'S02-F25'`, `file: '06-indian-classical.mdx'`
   - `rule`: ledger F25 — the chapter was described as covering "Hindustani and
     Carnatic" while the Carnatic tala system (solkattu/konnakol, different tala
     families, a different conceptual frame) is absent; the description now says
     Hindustani and the absence is stated in the chapter rather than left to be
     inferred
   - `forbidden`: `['Hindustani and Carnatic']`
   - `present`: `['Carnatic', 'solkattu', 'konnakol']`

   The `present` arm is what makes this more than a deletion: removing the word
   from the description would satisfy `forbidden` while leaving the absence
   unstated, which is the half of the Definition of Done that matters.

   Then add a second claim for the index page: `id: 'S02-F25-overview'`,
   `file: 'theory-counterpoint-overview.mdx'`, same `rule`, with
   `forbidden: ['Hindustani and Carnatic']` and no `present` arm — that page
   states no scope of its own, it only must stop advertising one it cannot back.

2. **Run it and watch both fail on the forbidden side.**
   `node --test site/tests/scope-framing.test.mjs` must report
   `forbidden phrase reappeared: "Hindustani and Carnatic"` for each. A
   `present`-side failure first on `S02-F25` would mean the forbidden phrase is
   not where you think it is.

3. **Rewrite the chapter description.** In `06-indian-classical.mdx` front
   matter, replace

   > `description: The tala system of Hindustani and Carnatic music — how cycle lengths of 7, 10, and 16 beats create rhythmic architecture through subdivision and recombination.`

   with

   > `description: The Hindustani tala system — how cycle lengths of 7, 10, and 16 beats create rhythmic architecture through subdivision and recombination.`

   Keep the rest of the sentence exactly as it is: the cycle lengths and the
   subdivision/recombination framing are accurate and are not what F25 is about.

4. **Add the scope note.** Immediately after the chapter's opening paragraph and
   before the `:::note[Theory deep dive]` admonition, add a `:::note[Scope]`
   admonition saying, in the guide's voice, that this chapter covers Hindustani
   practice; that Carnatic music has its own tala system — different tala
   families, and solkattu and konnakol as its rhythmic-solfège vocabulary — and
   that it is not covered here; and that Nelson's *Solkattu Manual* in the
   reference appendix is where a reader should start on it.

   Say what is absent and why a reader might want it. "Carnatic is out of scope"
   alone tells a reader nothing about what they are missing.

5. **Rewrite the index row.** In `theory-counterpoint-overview.mdx` line 61,
   change the middle cell from `Hindustani and Carnatic rhythm` to
   `Hindustani tala and rhythmic arithmetic`. Leave the two link cells alone.

6. **Run the case and watch both pass**, then **run `site-unit` immediately** —
   `npm --prefix site test`. Confirm `S02-F01`, `S02-F12`, `S06-F16-indian` and
   `S04-F20` are all among the passing cases; they are M001's and M002's locks
   on this chapter. If any goes red, stop and report rather than editing it.

7. **Prove the lock bites on the second site.** Restore
   `Hindustani and Carnatic` to the overview table row, confirm
   `S02-F25-overview` fails while `S02-F25` still passes, then revert with an
   inverse edit. Two independent cases, not one guard counted twice.

8. **Run `site-unit` and `doc-conformance`** with everything staged
   (`git add -A -- site`), and read their exit codes.

9. **Close the row and the slice.** In `docs/plans/theory-audit/ledger.md`, set
   F25's `Status` to `done`, name cases `S02-F25` and `S02-F25-overview` in its
   `Verification` cell, and correct its `Lands in` cell to the two content files
   this task modified. Record in the `Item` cell that the overview index row
   carried the same overclaim and was corrected with it. Use no `|` in any cell.

   Then tick both Definition-of-Done boxes in the slice and in this plan's copy,
   tick this plan's Task status box, and set the slice `Status` to `done`.

10. **Write the evidence** to `docs/plans/theory-audit/evidence/M003-S02.md` in
    the format `/jk:next` section 4 gives. Name no commit SHA. Then run `format`
    last, per the gate ordering, and `jk-standards ledger`.

11. **Commit** as one unit with trailers
    `Plan: docs/plans/theory-audit/ledger.md`, `Slice: M003/S02`, `Rows: F25`.
