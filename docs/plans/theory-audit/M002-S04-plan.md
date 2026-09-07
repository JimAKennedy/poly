# M002/S04 — Chapter 6 Indian-classical citations

**Slice:** M002/S04
**Ledger:** `docs/plans/theory-audit/ledger.md`

Chapter 6 cites Clayton (2000), Nelson (2008) and Kippen (1988) exactly zero
times. All three sit in Further Reading, and all three are cited on the
chapter's companion page — but the chapter itself carries a commercial blog, a
high-school textbook PDF and a konnakol video instead.

## Task status

The executable state of this plan. `/jk:next` reads the first unchecked box
here to decide what to do; each task's own commit ticks its box.

- [ ] Task 1 — Chapter 6: source the tala, theka and layakari claims
- [ ] Task 2 — Companion page: drop the video citations from two claims (F20)

## Definition of Done

Copied verbatim from the slice. Both tasks below argue against *this* text.

- [ ] Chapter 6's tala, laya and theka claims cite Clayton (2000), Nelson
      (2008) or Kippen (1988) inline
- [ ] Refs [21]–[25] no longer carry a named-theory claim in Chapter 6

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

Run `node --test site/tests/citation-tier.test.mjs` in the inner loop.

**Gate ordering.** `format` runs the `ledger` pre-commit hook, which fails while
a slice claiming `done` has no evidence file. Task 2 closes the slice, so its
order is: `site-unit` and `doc-conformance` first, then the evidence file, then
`format`. Task 1 does not close the slice and may run `format` at any point.

## Context

**The finding.** Ledger F20: refs [21]–[25] include a commercial blog
(*Artium Academy*), a high-school textbook PDF (*NIOS*), a journal PDF, a
pubpub article and a YouTube konnakol video, while Clayton, Nelson and Kippen
sit unused in Further Reading.

**The citation sites.** Five superscript blocks across two files, plus one
bibliographic listing that stays:

| Site | Cites | What it is |
|---|---|---|
| `06-indian-classical.mdx:24` | `ref-21` | tala-as-architectural-space claim — Task 1 |
| `06-indian-classical.mdx:32` | `ref-22` | tala/vibhag claim — Task 1 |
| `06-indian-classical.mdx:54` | `ref-25` | layakari claim — Task 1 |
| `theory-indian-classical.mdx:13` | `ref-21`, `ref-22` | tala definition — Task 2 |
| `theory-indian-classical.mdx:25` | `ref-23`, `ref-24` | layakari ratios — Task 2 |
| `theory-indian-classical.mdx:67` | `ref-21`–`ref-25` | Sources "See also" — **leave alone** |

**The companion page uses a multi-reference superscript form** that the narrow
pattern M002/S02 and M002/S03 used cannot match:

```
<sup>[21](/appendix-references/#ref-21), [22](/appendix-references/#ref-22)</sup>
```

Both cases in this slice therefore use the generalised pattern
`/<sup>[^<]*#ref-2[1-5][^<]*<\/sup>/`, which catches single- and
multi-reference blocks alike and cannot match line 67's plain-link listing —
that line has no `<sup>` at all. Verified against both files: it finds three
blocks in the chapter and two on the companion.

**The Definition of Done's theka claim is currently uncited.** It is line 42 of
the chapter, the sentence M001/S02 wrote when it corrected F12:

> E(7,16) is not itself a tintal theka — thekas are fixed, named bol sequences
> that every tabla player learns by rote, not Bjorklund distributions.

That is a named-theory claim about theka with no citation, so satisfying the
Definition of Done means **adding** Kippen (1988) there — the Further Reading
entry describes it as "Theka elaboration practice", and the companion page
credits it for "theka elaboration, kaida grammar". This is the same shape as
M002/S03's line 41.

**Three M001 locks live in `06-indian-classical.mdx`** —
`S02-F01` (rupak illustration), `S02-F12` (the theka sentence Task 1 edits) and
`S06-F16` (the lcm-as-sam translation caveat). `S02-F12` asserts three prose
phrases — `not itself a tintal theka`, `thekas are fixed, named bol sequences`,
`rough analogue` — and constrains no citation, so appending one is safe. Task 1
runs `site-unit` immediately after its edits and names all three as the cases to
watch. A slice that quietly broke a previous milestone's lock would be the worst
outcome here.

**Nelson (2008) is not used.** The Definition of Done offers Clayton *or*
Nelson *or* Kippen. Nelson's territory is Carnatic tihai and mora arithmetic,
which is M004/S04's worked example, not this slice's subject.

**`ref-25` was off-topic as well as low-tier.** It is a "Konnakol Mastery"
YouTube video cited for *layakari*. Konnakol is Carnatic vocal percussion, a
different subject from rhythmic augmentation ratios — the same shape as
`ref-17` in M002/S03, which was an Afro House guide cited in the Afrobeat
chapter.

**The lock host.** `site/tests/citation-tier.test.mjs` exists — M002/S01
created it, S02 and S03 extended it. Append to it. Do not add cases to
`theory-audit-claims.test.mjs`, which is M001's host.

**Reverting a test mutation.** When proving a lock bites, revert with an
inverse edit, not `git checkout --`, if the file carries uncommitted work from
the same task. M002/S03 lost all three of its edits that way and had to redo
them.

## Task 1 — Chapter 6: source the tala, theka and layakari claims

**Modifies:** `site/src/content/docs/06-indian-classical.mdx`,
`site/tests/citation-tier.test.mjs`
**Rows:** none — F20 also covers the companion page and closes in Task 2

### Steps

1. **Write the failing case.** Add to the `CLAIMS` array in
   `site/tests/citation-tier.test.mjs`:

   - `id: 'S04-F20'`, `file: '06-indian-classical.mdx'`
   - `rule`: ledger F20 — Ch 6's tala and layakari claims were cited to a
     commercial blog, a school textbook PDF and a konnakol video while Clayton
     and Kippen sat unused in Further Reading; the theka claim was uncited
     altogether
   - `forbiddenRegex`: `[/<sup>[^<]*#ref-2[1-5][^<]*<\/sup>/]`
   - `presentRegex`: `[/#fr-clayton-2000/, /#fr-kippen-1988/]`

   Both `presentRegex` arms are unsatisfied in the chapter today — it cites
   none of the three scholarly sources — so each is a real lock this task
   establishes rather than a standing guard.

2. **Run it and watch it fail on the forbidden side.**
   `node --test site/tests/citation-tier.test.mjs` must report
   `forbidden pattern reappeared` for `S04-F20`. `assertClaim` checks forbidden
   before present, so that arm bites first even though both present arms are
   also unsatisfied.

3. **Edit line 24**, the tala-as-architectural-space claim. Replace

   > `treats the rhythmic cycle itself as an architectural space<sup>[21](/appendix-references/#ref-21)</sup>`

   with

   > `treats the rhythmic cycle itself as an architectural space ([Clayton 2000](/appendix-references/#fr-clayton-2000))`

4. **Edit line 32**, the tala/vibhag claim. Replace

   > `each with a characteristic emphasis<sup>[22](/appendix-references/#ref-22)</sup>.`

   with

   > `each with a characteristic emphasis ([Clayton 2000](/appendix-references/#fr-clayton-2000)).`

5. **Edit line 42**, the theka claim, which currently has no citation. Replace

   > `not Bjorklund distributions.`

   with

   > `not Bjorklund distributions ([Kippen 1988](/appendix-references/#fr-kippen-1988)).`

   Leave the rest of the sentence exactly as it stands: `S02-F12` requires the
   phrases `not itself a tintal theka`, `thekas are fixed, named bol sequences`
   and `rough analogue`, and none of them is touched by appending a citation to
   this clause.

6. **Edit line 54**, the layakari claim. Replace

   > `maps directly to Poly's subdivision parameter<sup>[25](/appendix-references/#ref-25)</sup>.`

   with

   > `maps directly to Poly's subdivision parameter ([Clayton 2000](/appendix-references/#fr-clayton-2000)).`

   The citation attaches to *layakari* — rhythmic augmentation and diminution,
   which is Clayton's subject. The clause about Poly's subdivision parameter is
   the project's own and needs no source; it is not being sourced here, it is
   simply in the same sentence.

7. **Run the case and watch it pass**, then **run `site-unit` immediately** —
   `npm --prefix site test`. Confirm `S02-F01`, `S02-F12` and `S06-F16` are all
   among the passing cases; they are M001's locks on this file. If any goes red,
   stop and report rather than editing it.

8. **Run the gates**: `pre-commit run --all-files`, `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`. Read each exit code.

9. **Append to** `docs/plans/theory-audit/evidence/M002-S04.md`, creating it, in
   the format `/jk:next` section 4 gives. Name no commit SHA.

10. **Commit** the chapter, the test, the plan's ticked box and the evidence as
    one unit, with trailers `Plan: docs/plans/theory-audit/ledger.md` and
    `Slice: M002/S04`. No `Rows:` trailer — this task closes no row.

## Task 2 — Companion page: drop the video citations from two claims

**Modifies:** `site/src/content/docs/theory-indian-classical.mdx`,
`site/tests/citation-tier.test.mjs`, `docs/plans/theory-audit/ledger.md`,
`docs/plans/theory-audit/evidence/M002-S04.md`
**Consumes:** Task 1's `S04-F20` case, as the pattern to copy
**Rows:** F20

### Steps

1. **Write the failing case.** Add to `CLAIMS` in
   `site/tests/citation-tier.test.mjs`:

   - `id: 'S04-F20-theory'`, `file: 'theory-indian-classical.mdx'`
   - `rule`: ledger F20 — the companion page cited the same blog and textbook
     PDF for its tala definition, on a sentence already carrying Clayton, and
     cited a journal PDF and a pubpub article for the layakari ratios; the
     Sources "See also" listing is a bibliographic pointer and stays
   - `forbiddenRegex`: `[/<sup>[^<]*#ref-2[1-5][^<]*<\/sup>/]`
   - `presentRegex`: `[/#fr-clayton-2000/]`

   The `presentRegex` arm is already satisfied — the page already cites
   `fr-clayton-2000` — so it is a standing guard against a later
   edit stripping Clayton out, not a lock this task establishes. The forbidden
   arm is the only way this case can fail, which is what step 2 expects.

2. **Run it and watch it fail on the forbidden side**, for the reason above.

3. **Edit line 13.** Delete
   ` <sup>[21](/appendix-references/#ref-21), [22](/appendix-references/#ref-22)</sup>`
   including the space before it, so the sentence reads
   `… the point of arrival and agreement ([Clayton 2000](/appendix-references/#fr-clayton-2000)).`
   Add nothing: the Clayton citation already in that sentence sources the claim.

4. **Edit line 25**, the layakari-ratios rule. Replace

   > `only the subdivision multiplies <sup>[23](/appendix-references/#ref-23), [24](/appendix-references/#ref-24)</sup>.`

   with

   > `only the subdivision multiplies ([Clayton 2000](/appendix-references/#fr-clayton-2000)).`

   Clayton is the scholarly standard on laya and metre, which is what this rule
   states.

5. **Run the case and watch it pass**, then confirm line 67 survives:
   `grep -n 'See also refs' site/src/content/docs/theory-indian-classical.mdx`
   must still show the `#ref-21` and `#ref-25` links. If it does not, the
   forbidden pattern is matching the listing, which it must not.

6. **Prove the lock bites on the multi-reference form**, which is the form
   M002/S02's and M002/S03's narrower pattern would have missed. Re-insert
   ` <sup>[23](/appendix-references/#ref-23), [24](/appendix-references/#ref-24)</sup>`
   before the full stop on line 25, confirm `S04-F20-theory` fails, then revert
   **with an inverse edit, not `git checkout --`**, and confirm the case passes
   again.

7. **Run `site-unit` and `doc-conformance`** and read their exit codes.

8. **Close the row and the slice.** In `docs/plans/theory-audit/ledger.md`, set
   F20's `Status` to `done`. Rewrite its `Verification` cell to name cases
   `S04-F20` and `S04-F20-theory` in `site/tests/citation-tier.test.mjs` — the
   cell currently forward-references the Tier-A check M002/S06 builds, which
   cannot be what proves this slice. Correct its `Lands in` cell to
   `06-indian-classical.mdx`, `theory-indian-classical.mdx` — the files this
   slice actually modified, not the `appendix-references.mdx` it names and never
   touches. In the `Item` cell record that the theka claim was uncited rather
   than mis-cited, that `ref-25` was a konnakol video cited for layakari and so
   off-topic as well as low-tier, and that the Sources listing was deliberately
   left. Use no `|` in any cell.

   Then tick both Definition-of-Done boxes in the slice and in this plan's copy,
   tick this plan's Task 2 box, and set the slice `Status` to `done`.

9. **Append the evidence**, then run `format` (`pre-commit run --all-files`) —
   last, per the gate ordering above — and `jk-standards ledger`.

10. **Commit** as one unit with trailers
    `Plan: docs/plans/theory-audit/ledger.md`, `Slice: M002/S04`, `Rows: F20`.
