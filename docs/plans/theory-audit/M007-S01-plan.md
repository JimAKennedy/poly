# M007/S01 — Rule triage

**Slice:** M007/S01, in `docs/plans/theory-audit/ledger.md`
**Decisions:** `M007-decisions.md`

## Task status

- [x] Task 1 — Record a verdict for every numbered rule

## Definition of Done

- [x] Every numbered rule on every theory page is classified checkable or not,
      with a one-line reason recorded for each not-checkable verdict
- [x] The triage lives beside the checklist, so a rule added to a theory page
      without a verdict is visible

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

`.jk/validations.yml` is authoritative if this table disagrees with it.

## What this slice decides

Eleven theory pages state 92 numbered rules. Nine were asserted by the
2026-07-30 review's harness and five more by M004/S05, so fourteen are covered.
**Nothing records what the other 78 are.** Until a verdict exists, "this rule is
not checked" and "this rule is not checkable" look identical from the outside,
and that is what B12 is about.

A rule is **checkable** when a lane table settles it — hit counts, step counts,
rotations, velocities, mutation, swing, subdivision, or onset relationships
between lanes. It is **not checkable** when deciding it needs a judgement no
predicate makes: "The One is sacred", "each part alone must be playable and
idiomatic", "declare the clave and its direction first".

**A not-checkable verdict is a claim, and it needs its reason.** Without one the
triage becomes a place rules go to be excused, which is the same failure the
divergence marker was designed against.

## Task 1 — Record a verdict for every numbered rule

Closes **B12**.

**Files:** `site/tests/theory-patch-conformance.test.mjs`

1. **Write the failing case.** Add a `RULE_TRIAGE` structure keyed by page, then
   by rule number, each entry carrying `checkable: true|false` and — when false
   — a `why`. Add a case that, for every `theory-*.mdx` page, extracts its
   numbered rules from the `## The Rules` section and asserts each has an entry.
   It must fail naming the page and rule number of anything missing, and
   separately fail on a `checkable: false` entry with no `why`.

   Extract the rules from the page rather than hard-coding a count: a page that
   gains a rule must fail this case, which is the second definition-of-done item.

2. **Run it and watch it fail**, naming every rule with no verdict.

3. **Fill in the triage, page by page.** Read each rule's full text, not its
   bolded headline — several headlines sound mechanical and are not, and at
   least one sounds prose-like and is not. Record `checkable: true` where a lane
   table settles it, and a specific reason otherwise. "Prose judgement" is not a
   reason; "asks whether a part is idiomatic for a human player, which no cell
   in the table reports" is.

   For rules already asserted, mark them checkable and name the existing case,
   so the triage doubles as a coverage map.

4. **Run and watch it pass.**

5. **Prove it bites, both ways.** Delete one verdict and watch the case name
   that page and rule; restore. Blank one `why` on a `checkable: false` entry
   and watch the second arm fire; restore. Both by inverse edit.

6. **Record the count in the row.** B12's `Verification` wants the verdict
   recorded: say how many rules were triaged, how many are checkable, and how
   many of those already have cases. That number is what M007/S02 is scoped by.

7. **Check.** `bash scripts/check-doc-conformance.sh`,
   `pre-commit run --all-files`.

8. **Commit** with `Rows: B12`, ticking Task 1, closing B12, appending to
   `docs/plans/theory-audit/evidence/M007-S01.md`, and setting the slice `done`.

## Self-review

**DoD coverage.** Item 1 (every rule classified, with a reason for each
not-checkable verdict) → Task 1 steps 1 and 3, with step 5 proving both arms.
Item 2 (a rule added without a verdict is visible) → step 1's requirement to
extract rules from the page rather than hard-code a count.

**Row coverage.** B12 → Task 1 step 8; step 6 records the count its
`Verification` asks for.

**Placeholder scan.** No TBDs. The plan states what makes a rule checkable and
what does not count as a reason.

**Name consistency.** `RULE_TRIAGE` throughout.
