# M002/S05 — Chapter 7 Balkan citations

**Slice:** M002/S05
**Ledger:** `docs/plans/theory-audit/ledger.md`

Chapter 7 cited an educational aggregator for the definition of *aksak* and a
theory blog for the Bulgarian wedding-music tradition, while Brăiloiu — who
coined the term — and Rice, whose ethnography is the standard on Bulgarian
practice, sat in Further Reading only.

## Task status

The executable state of this plan. `/jk:next` reads the first unchecked box
here to decide what to do; the task's own commit ticks it.

- [x] Task 1 — Chapter 7: cite Brăiloiu for aksak and Rice for svatbarska (F21)

## Definition of Done

Copied verbatim from the slice. The task below argues against *this* text.

- [x] The aksak-definition and svatbarska-muzika claims cite Brăiloiu (1951),
      Rice (1994) or Goldberg (2015) inline
- [x] Refs [26] and [27] no longer carry a named-theory claim in Chapter 7
- [x] The `S04-F06` long-beat lock still passes after the citation edits

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

Run `node --test site/tests/citation-tier.test.mjs` in the inner loop.

**Gate ordering.** `format` runs the `ledger` pre-commit hook, which fails while
a slice claiming `done` has no evidence file. This task closes the slice, so the
order is: `site-unit` and `doc-conformance` first, then the evidence file, then
`format`.

## Context

**The finding.** Ledger F21: refs [26] (*Fiveable*) and [27] (*Chromatone*) are
educational aggregator pages, while Brăiloiu, Rice and Goldberg sit in Further
Reading only.

**Two citation sites, both in the chapter:**

| Site | Cites | Claim |
|---|---|---|
| `07-balkan.mdx:37` | `ref-26` | the definition of *aksak* — "limping", short (2) and long (3) groupings |
| `07-balkan.mdx:88` | `ref-27` | *svatbarska muzika* pushes aksak metres to extraordinary tempos |

**`theory-balkan.mdx` needs no edit.** It mentions `ref-26` once, on line 68,
inside the `Primary:` Sources line — a plain link, not a superscript. Checked
with the generalised pattern below, the companion has **zero** superscript
blocks, so unlike `theory-indian-classical.mdx` in M002/S04 there is genuinely
nothing there to fix. Leave the Sources line alone; it is a bibliographic
pointer, and it is what keeps refs [26] and [27] from becoming orphans once
their claims move away.

**Source matching.** Both are unusually clean, because the Further Reading
entries name exactly these subjects:

- Brăiloiu (1951), "Le rythme aksak", is described in the appendix as "The
  origin of the term and the two/three-cell theory of additive meter" — which
  is precisely what line 37 claims.
- Rice (1994), *May It Fill Your Soul: Experiencing Bulgarian Music*, is the
  Bulgarian ethnography; `theory-balkan.mdx` already credits it for "Bulgarian
  practice and dance linkage", which is what line 88 claims.

Goldberg (2015) is the third source the Definition of Done offers, but it is
already cited at line 41 for the long-beat timing claim and is not the
authority for either of these two. Do not move it.

**Use the generalised superscript pattern**
`/<sup>[^<]*#ref-2[67][^<]*<\/sup>/`, not the single-reference form M002/S02
and M002/S03 used. M002/S04 proved directly that the narrow form fails to match
a block carrying two references inside one `<sup>`. Chapter 7 happens to use
only single-reference blocks, so either would pass here — the generalised form
is used so the milestone's cases are consistent, and so a later edit that adds
a second reference to one of these superscripts cannot slip past.

**`S04-F06` is named in this slice's Definition of Done**, which no other slice
in the milestone does. It is M001/S04's lock on the aksak long-beat claim at
line 41 — that performed long beats deviate from an exact 3:2 ratio, cited to
`fr-goldberg-2015`. Line 41 sits *between* the two lines this task edits and is
not touched, but the Definition of Done asks for the lock to be asserted rather
than assumed, so step 6 checks it by name in the `site-unit` output.

**The lock host.** `site/tests/citation-tier.test.mjs` exists — M002/S01
created it, S02 through S04 extended it. Append to it. Do not add cases to
`theory-audit-claims.test.mjs`, which is M001's host and owns `S04-F06`.

**Reverting a test mutation.** Revert with an inverse edit, not
`git checkout --`, while the file carries this task's uncommitted work.
M002/S03 lost all three of its edits that way.

## Task 1 — Chapter 7: cite Brăiloiu for aksak and Rice for svatbarska

**Modifies:** `site/src/content/docs/07-balkan.mdx`,
`site/tests/citation-tier.test.mjs`, `docs/plans/theory-audit/ledger.md`
**Creates:** `docs/plans/theory-audit/evidence/M002-S05.md`
**Rows:** F21

### Steps

1. **Write the failing case.** Add to the `CLAIMS` array in
   `site/tests/citation-tier.test.mjs`:

   - `id: 'S05-F21'`, `file: '07-balkan.mdx'`
   - `rule`: ledger F21 — Ch 7 cited an educational aggregator for the
     definition of aksak and a theory blog for svatbarska muzika, while
     Brăiloiu, who coined the term, and Rice, whose ethnography is the standard
     on Bulgarian practice, sat in Further Reading only
   - `forbiddenRegex`: `[/<sup>[^<]*#ref-2[67][^<]*<\/sup>/]`
   - `presentRegex`: `[/#fr-brailoiu-1951/, /#fr-rice-1994/]`

   Both `presentRegex` arms are unsatisfied in the chapter today — it cites only
   `fr-goldberg-2015` — so each is a real lock this task establishes rather than
   a standing guard.

2. **Run it and watch it fail on the forbidden side.**
   `node --test site/tests/citation-tier.test.mjs` must report
   `forbidden pattern reappeared` for `S05-F21`. `assertClaim` checks forbidden
   before present, so that arm bites first even though both present arms are
   also unsatisfied.

3. **Edit line 37**, the aksak definition. Replace

   > `short (2) and long (3) groupings<sup>[26](/appendix-references/#ref-26)</sup>.`

   with

   > `short (2) and long (3) groupings ([Brăiloiu 1951](/appendix-references/#fr-brailoiu-1951)).`

   Keep the `ă` in the display text; the anchor id is plain ASCII
   (`fr-brailoiu-1951`) and must be typed exactly as shown.

4. **Edit line 88**, the svatbarska-muzika claim. Replace

   > `pushes aksak metres to extraordinary tempos<sup>[27](/appendix-references/#ref-27)</sup>.`

   with

   > `pushes aksak metres to extraordinary tempos ([Rice 1994](/appendix-references/#fr-rice-1994)).`

5. **Run the case and watch it pass**, and confirm the chapter now contains no
   `#ref-26` or `#ref-27`:
   `grep -c '#ref-2[67]' site/src/content/docs/07-balkan.mdx` must print `0`.

6. **Run `site-unit` immediately** — `npm --prefix site test` — and confirm
   `S04-F06 (07-balkan.mdx)` is among the passing cases **by name**, not merely
   that the suite is green. The third Definition-of-Done item asks for that lock
   specifically, and line 41, which it guards, sits between the two lines this
   task edits. If it goes red, stop and report rather than editing it.

7. **Confirm the companion is untouched:**
   `git diff --quiet site/src/content/docs/theory-balkan.mdx` must succeed, and
   `grep -c '#ref-26' site/src/content/docs/theory-balkan.mdx` must still print
   `1` — its Sources `Primary:` line keeps the reference from orphaning.

8. **Prove the lock bites.** Re-insert
   `<sup>[26](/appendix-references/#ref-26)</sup>` before the full stop on line
   37, confirm `S05-F21` fails naming the forbidden pattern, then revert **with
   an inverse edit, not `git checkout --`**, and confirm the case passes again.

9. **Run `site-unit` and `doc-conformance`** and read their exit codes.

10. **Close the row and the slice.** In `docs/plans/theory-audit/ledger.md`, set
    F21's `Status` to `done`. Rewrite its `Verification` cell to name case
    `S05-F21` in `site/tests/citation-tier.test.mjs` and to keep the `S04-F06`
    clause, which is a real acceptance condition rather than a forward
    reference — the Tier-A half of that cell points at the check M002/S06
    builds, which cannot be what proves this slice. Correct its `Lands in` cell
    to `07-balkan.mdx` alone, the only file this slice modifies. In the `Item`
    cell record which source took which claim and that the companion page's
    Sources line was deliberately left. Use no `|` in any cell.

    Then tick all three Definition-of-Done boxes in the slice and in this plan's
    copy, tick this plan's Task status box, and set the slice `Status` to
    `done`.

11. **Write the evidence** to `docs/plans/theory-audit/evidence/M002-S05.md` in
    the format `/jk:next` section 4 gives, recording the `S04-F06` result
    explicitly since the Definition of Done asks for it. Name no commit SHA.
    Then run `format` (`pre-commit run --all-files`) — last, per the gate
    ordering above — and `jk-standards ledger`.

12. **Commit** as one unit with trailers
    `Plan: docs/plans/theory-audit/ledger.md`, `Slice: M002/S05`, `Rows: F21`.
