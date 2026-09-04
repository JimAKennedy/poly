# M002/S03 — Chapter 4 Afrobeat citations

**Slice:** M002/S03
**Ledger:** `docs/plans/theory-audit/ledger.md`

Chapter 4 carried two numbered-reference citations, both to sources that cannot
support what they were attached to, while Allen & Veal (2013) and Veal (2000) —
the autobiography and the standard biography — sat unused in Further Reading.

## Task status

The executable state of this plan. `/jk:next` reads the first unchecked box
here to decide what to do; the task's own commit ticks it.

- [ ] Task 1 — Chapter 4: source the opening and phrase claims, drop the rest (F19)

## Definition of Done

Copied verbatim from the slice. The task below argues against *this* text.

- [ ] Chapter 4's opening and phrase-gating claims cite Allen & Veal (2013) or
      Veal (2000) inline
- [ ] Refs [14]–[17] no longer carry a named-theory claim in Chapter 4

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

Run `node --test site/tests/citation-tier.test.mjs` in the inner loop.

**Gate ordering.** `format` runs the `ledger` pre-commit hook, which fails while
a slice claiming `done` has no evidence file. This task closes the slice, so the
order is: `site-unit` and `doc-conformance` first, then write the evidence file,
then `format`.

## Context

**The finding.** Ledger F19: refs [14]–[17] are YouTube videos and production
blogs, while `fr-allen-veal-2013` and `fr-veal-2000` sit unused in Further
Reading.

**Chapter 4 carries exactly two numbered-reference citations**, and neither is
what the finding's wording implies:

| Site | Cites | What it is |
|---|---|---|
| `04-afrobeat.mdx:13` | `ref-14` | co-attribution opening — already carries `fr-allen-veal-2013` in the same sentence |
| `04-afrobeat.mdx:49` | `ref-17` | a claim about **Poly's** lane behaviour, not about Afrobeat |

**Refs [15] and [16] are cited nowhere.** No `.mdx` under the docs root contains
`#ref-15` or `#ref-16`. Half of the second Definition-of-Done item holds before
this slice starts. Do not invent work for them, and do not delete the entries —
M002/S06 owns reference tiers.

**`theory-afrobeat.mdx` needs no edit.** Its only reference to refs [14]–[17] is
on line 63, a Sources `See also refs [14]–[17]` bibliographic listing — the same
shape as the Afro-Cuban companion's line 73, which M002/S02 deliberately left in
place. It is a pointer, not a named-theory claim. The companion-page rule S02
established still applies here; applied, it finds nothing to change.

**The Definition of Done's "phrase-gating claim" is currently uncited.** It is
line 41, the opening sentence of `## Phrase Architecture: Breathing Grooves`:

> The defining characteristic of Afrobeat drumming is that it breathes. Parts
> enter and exit over multi-bar cycles, creating an arrangement that thickens
> and thins without any deliberate compositional decision — each voice simply
> follows its own phrase schedule.

That is a musicological claim with no citation at all, so satisfying the
Definition of Done means **adding** a source, not re-pointing one. Veal (2000)
is the right one: the companion page's own Sources note credits it for
"ensemble arranging and form".

**Line 49 gets no citation.** It reads:

> When you set five or six lanes with different phrase lengths and offsets, the
> result is an ensemble that self-arranges — some combination of lanes is always
> active, but the specific combination is always changing.

It sits under "In Poly, each lane's phrase parameters control this breathing"
and describes what Poly does. Under the `research-provenance` discipline's three
claim classes that is a project-specific value, declared as the project's own
rather than cited to anyone. `ref-17` is an *Afro House* production guide — a
different genre from Fela's Afrobeat, so off-topic as well as low-tier — and
re-citing the sentence to Veal would replace a wrong-tier citation with a
wrong-claim one, which is the trap M002/S02 avoided on its line 79. Delete the
citation and add nothing.

**M001/S06 holds a lock on line 13's sentence.** `S06-F09` in
`site/tests/theory-audit-claims.test.mjs` requires the phrases
`came out of that partnership rather than from either alone` and
`co-attributed the result throughout his own account of it`, plus
`presentRegex` `/#fr-allen-veal-2013/`. All three survive removing the
superscript, which sits immediately after "either alone" — but a slice that
quietly broke a previous milestone's lock would be the worst outcome here, so
step 5 runs `site-unit` immediately and names `S06-F09` as the case to watch.

**The lock host.** `site/tests/citation-tier.test.mjs` exists — M002/S01 created
it, M002/S02 extended it. Append to it. Do not add cases to
`theory-audit-claims.test.mjs`, which is M001's host.

## Task 1 — Chapter 4: source the opening and phrase claims, drop the rest

**Modifies:** `site/src/content/docs/04-afrobeat.mdx`,
`site/tests/citation-tier.test.mjs`, `docs/plans/theory-audit/ledger.md`
**Creates:** `docs/plans/theory-audit/evidence/M002-S03.md`
**Rows:** F19

### Steps

1. **Write the failing case.** Add to the `CLAIMS` array in
   `site/tests/citation-tier.test.mjs`:

   - `id: 'S03-F19'`, `file: '04-afrobeat.mdx'`
   - `rule`: ledger F19 — Ch 4's opening cited a YouTube video while Allen &
     Veal (2013) sat unused; the phrase-architecture claim was uncited and takes
     Veal (2000); the lane-behaviour sentence is Poly's own and takes no source
   - `forbiddenRegex`:
     `[/<sup>\[14\]\(\/appendix-references\/#ref-14\)<\/sup>/, /<sup>\[17\]\(\/appendix-references\/#ref-17\)<\/sup>/]`
   - `presentRegex`: `[/#fr-allen-veal-2013/, /#fr-veal-2000/]`

   `#fr-veal-2000` is absent from the chapter today, so that arm is a real lock
   this task establishes. `#fr-allen-veal-2013` is already present — M001/S06
   put it there — so that arm is a standing guard against a later edit stripping
   it out, not something this task creates.

2. **Run it and watch it fail on the forbidden side.**
   `node --test site/tests/citation-tier.test.mjs` must report
   `forbidden pattern reappeared` for `S03-F19`. `assertClaim` checks forbidden
   before present, so that is the first arm to bite even though the
   `#fr-veal-2000` arm is also unsatisfied.

3. **Edit line 13.** Delete `<sup>[14](/appendix-references/#ref-14)</sup>`,
   leaving the sentence as

   > `…its rhythmic vocabulary came out of that partnership rather than from either alone — a way of playing that…`

   Add nothing: the `([Allen & Veal 2013](/appendix-references/#fr-allen-veal-2013))`
   citation later in the same sentence already sources the co-attribution, which
   is why `S06-F09` passes on it today.

4. **Edit line 41**, the phrase-architecture claim. Replace

   > `each voice simply follows its own phrase schedule.`

   with

   > `each voice simply follows its own phrase schedule ([Veal 2000](/appendix-references/#fr-veal-2000)).`

   This is the parenthetical Further-Reading form used throughout the guide, and
   the form the companion page uses for the same source.

5. **Edit line 49.** Delete `<sup>[17](/appendix-references/#ref-17)</sup>` so
   the sentence ends `…the specific combination is always changing.` Add no
   citation.

6. **Run the case and watch it pass**, then **run `site-unit` immediately** —
   `npm --prefix site test`. Confirm `S06-F09 (04-afrobeat.mdx)` is among the
   passing cases; it is M001's lock on the sentence step 3 edited. If it goes
   red, stop and report rather than editing it: a previous milestone's lock
   objecting to this change is information, not an obstacle.

7. **Confirm the companion page is untouched and its listing intact:**
   `git diff --quiet site/src/content/docs/theory-afrobeat.mdx` must succeed, and
   `grep -n 'See also refs' site/src/content/docs/theory-afrobeat.mdx` must still
   show the `#ref-14` and `#ref-17` links.

8. **Prove the lock bites**, as the M001 and M002 evidence files do. Re-insert
   `<sup>[17](/appendix-references/#ref-17)</sup>` at the end of line 49, confirm
   `S03-F19` fails naming the `ref-17` pattern, then revert and confirm with
   `git diff --quiet`. Two forbidden patterns in one case means either can bite;
   this shows the second one does, not just the first.

9. **Run `site-unit` and `doc-conformance`** and read their exit codes.

10. **Close the row and the slice.** In `docs/plans/theory-audit/ledger.md`, set
    F19's `Status` to `done`, and rewrite its `Verification` cell to name case
    `S03-F19` in `site/tests/citation-tier.test.mjs` — the cell currently
    forward-references the Tier-A check M002/S06 builds, which cannot be what
    proves this slice. In the `Item` cell record that refs [15] and [16] carried
    no claim to begin with, that line 49's citation was removed rather than
    re-sourced because the sentence is Poly's own, and that the companion page's
    Sources listing was deliberately left. Use no `|` in either cell.

    Then tick both Definition-of-Done boxes in the slice and in this plan's copy,
    tick this plan's Task status box, and set the slice `Status` to `done`.

11. **Write the evidence** to `docs/plans/theory-audit/evidence/M002-S03.md` in
    the format `/jk:next` section 4 gives. Name no commit SHA. Then run `format`
    (`pre-commit run --all-files`) — last, per the gate ordering above — and
    `jk-standards ledger`.

12. **Commit** as one unit with trailers
    `Plan: docs/plans/theory-audit/ledger.md`, `Slice: M002/S03`, `Rows: F19`.
