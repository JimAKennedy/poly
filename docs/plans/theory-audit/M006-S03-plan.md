# M006/S03 — De-duplicate the appendix

**Slice:** M006/S03, in `docs/plans/theory-audit/ledger.md`
**Decisions:** `M006-decisions.md` — B06, B07 and B11 were researched before
planning, and three ledger amendments follow from what that found.

## Task status

- [x] Task 1 — Retire the duplicate and assert no two entries share a work
- [x] Task 2 — Close the slice

## Definition of Done

- [x] No two appendix entries name the same work
- [x] A check fails when two entries share a title and year

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

`.jk/validations.yml` is authoritative if this table disagrees with it.

## What is duplicated

`ref-1` and `fr-toussaint-2005` are the same 2005 BRIDGES paper, listed twice in
one appendix at different weights. `ref-1` is cited from six files;
`fr-toussaint-2005` from none, which is why it also appears in M006/S02's orphan
list. Deleting it resolves both.

They cannot disagree today, because nothing cites the duplicate. The defect is
that nothing stops a later citation picking the wrong one.

## Task 1 — Retire the duplicate and assert no two entries share a work

Closes **B09**.

**Files:** `site/src/content/docs/appendix-references.mdx`,
`site/tests/citation-tier.test.mjs`

1. **Write the failing case**: no two appendix entries share a normalised title
   and year. Normalise by lowercasing, stripping punctuation and collapsing
   whitespace — the two entries phrase the same paper differently, so an exact
   match would miss them, which is the whole reason this went unnoticed.
2. **Run it and watch it fail**, naming both anchors and the shared work.
3. **Delete `fr-toussaint-2005`**, keeping `ref-1`, which is the cited one.
   Confirm by `grep` that nothing references the deleted anchor.
4. **Run and watch it pass.**
5. **Prove it bites.** Re-add a duplicate of any entry under a new anchor, watch
   the case name both, remove it by inverse edit.
6. **Record the sweep in the row.** B09's `Verification` asks for it: say how
   many entries were compared and that this was the only collision, so a later
   reader knows the check was run across the whole appendix rather than aimed at
   a known pair.
7. **Check** the three tokens, and **commit** with `Rows: B09`, ticking Task 1.

## Task 2 — Close the slice

1. **Tick both definition-of-done boxes**, tick Task 2, confirm B09 is `done`,
   set the slice `Status` to `done`.
2. **Run every token**, then `jk-standards ledger`.
3. **Append the evidence** and **commit** with `Rows: —`. Note in the message
   whether M006/S02 has already run, since deleting this duplicate changes its
   orphan count either way.

## Self-review

**DoD coverage.** Item 1 (no two entries name the same work) → Task 1 step 3.
Item 2 (a check fails when two entries share a title and year) → Task 1 step 1,
with step 5 proving it bites.

**Row coverage.** B09 → Task 1 step 7; its `Verification` also asks for the
sweep to be recorded, which is step 6.

**Placeholder scan.** No TBDs. The normalisation rule is stated rather than left
to the executor.

**Name consistency.** `ref-1` is kept, `fr-toussaint-2005` is deleted, named
consistently throughout.
