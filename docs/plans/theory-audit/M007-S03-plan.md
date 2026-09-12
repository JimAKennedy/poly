# M007/S03 — Burn down the divergence markers

**Slice:** M007/S03, in `docs/plans/theory-audit/ledger.md`
**Decisions:** `M007-decisions.md`

## Task status

- [ ] Task 1 — Fail on any untriaged marker, and close the milestone

## Definition of Done

- [ ] Every `patch-divergence-ok` marker either is replaced by a corrected patch
      or carries a reason a reviewer has accepted
- [ ] No marker carries the untriaged placeholder reason

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `gate` | `bash scripts/pre-push-check.sh` |

`.jk/validations.yml` is authoritative if this table disagrees with it.

## What is left to burn

M004 created six markers; five were handoffs removed by the slices that owed
them, and the sixth is B15's, which M007/S02 task 1 removes. Whether any remain
depends on what S02's rollout found — a rule that catches a contradiction the
slice cannot mechanically fix leaves a marker and a `B` row behind.

**Re-measure. Do not assume the count is zero.**

## Task 1 — Fail on any untriaged marker, and close the milestone

Closes **B14**.

**Files:** `site/tests/theory-patch-conformance.test.mjs`, the ledger, evidence

1. **Write the failing case**, or confirm it already fails: no
   `patch-divergence-ok` marker carries the phrase `not yet triaged`. The
   message must name the file and rule id, and say that the marker's `B` row
   needs resolving rather than the marker deleting.
2. **Run it.** If the tree is already clean it passes on the day it is written,
   which is expected — every untriaged marker either was resolved by S02 or
   never existed. Its non-vacuity then rests on step 3, not on that pass.
3. **Prove it bites.** Add a marker carrying the placeholder reason, watch the
   case name it, remove it by inverse edit.
4. **Confirm the suite prints the live marker count**, which M004/S05 built. If
   markers remain with accepted reasons, the count is not zero and that is fine
   — B14 forbids the *placeholder*, not the mechanism.
5. **Tick both definition-of-done boxes**, tick Task 1, confirm B14 is `done`,
   set the slice `Status` to `done`.
6. **Run every token**, ending with `bash scripts/pre-push-check.sh` for `gate`.
   Read its exit code.
7. **Run `jk-standards ledger`** with the slice `done`.
8. **Append the evidence** and **commit** with `Rows: B14`.

## Self-review

**DoD coverage.** Item 1 (every marker replaced or carrying an accepted reason)
→ Task 1 step 4, which distinguishes an accepted reason from a placeholder.
Item 2 (no marker carries the untriaged placeholder) → step 1, proved at step 3.

**Row coverage.** B14 → Task 1 step 8.

**Placeholder scan.** No TBDs. The plan says outright that the case may pass on
creation and where its non-vacuity then comes from.

**Name consistency.** `patch-divergence-ok` and the phrase `not yet triaged`
throughout — the latter must match what M004/S05 actually wrote.
