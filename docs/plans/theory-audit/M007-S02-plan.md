# M007/S02 — Roll out the checkable rules

**Slice:** M007/S02, in `docs/plans/theory-audit/ledger.md`
**Decisions:** `M007-decisions.md`

## Task status

- [ ] Task 1 — Rotate the breakbeat kick clear of the snare (B15)
- [ ] Task 2 — Add a checklist entry for every rule triaged checkable
- [ ] Task 3 — Close the slice

## Definition of Done

- [ ] Every rule triaged checkable carries a checklist entry
- [ ] Each entry is proved to fail when the lane it guards is removed
- [ ] A patch violating a rule either carries a divergence marker with a written
      reason or is corrected

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

`.jk/validations.yml` is authoritative if this table disagrees with it.

## What this slice is scoped by

**S01's triage, not this plan.** Read `RULE_TRIAGE` and cover every rule it
marks checkable that has no case yet. The estimate made while planning was
fifteen to twenty-five; if the triage says otherwise, the triage is right.

## Task 1 — Rotate the breakbeat kick clear of the snare (B15)

Closes **B15**, and removes the last divergence marker M004 left.

**Files:** `site/src/content/docs/theory-electronic-breakbeat.mdx`

The chopped kick is E(5,16) at rotation 3 — onsets `{3,6,9,12,15}` on a
16-pulse grid — and the backbeat snare is `{4,12}`. Pulse 12 is shared, so the
kick lands in a snare slot on a page whose Rule 7 has it avoid them.

**The decision is rotation 13**, onsets `{0,3,6,9,13}`: it clears both slots,
keeps an onset on pulse 0 so the pattern still starts on the downbeat, and
leaves four of five onsets where they were. Re-derive that with `bjorklund` and
`rotate` from `site/src/lib/euclidean-claims.mjs` rather than trusting this
paragraph.

1. **Remove the `patch-divergence-ok` marker** and run
   `node --test site/tests/theory-patch-conformance.test.mjs`. `ebb-kick-avoids-snare`
   must fail naming the intersection at pulse 12 — the marker was the only thing
   making it pass.
2. **Change the kick lane's Rotation cell** from 3 to 13.
3. **Run and watch the rule pass.**
4. **Prove it still bites** by restoring rotation 3, watching the failure name
   pulse 12, and restoring 13 by inverse edit.
5. **Confirm no marker remains**: `grep -rn patch-divergence-ok site/` returns
   nothing, and the suite's printed count is zero.
6. **Check.** `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`, `pre-commit run --all-files`, and
   `npm --prefix site run build`.
7. **Commit** with `Rows: B15`, ticking Task 1, closing B15, appending to
   `docs/plans/theory-audit/evidence/M007-S02.md`.

## Task 2 — Add a checklist entry for every rule triaged checkable

Closes **B13**.

**Files:** `site/tests/theory-patch-conformance.test.mjs`, and any page whose
patch a new rule finds in violation

Work page by page, in `RULE_TRIAGE` order. For each rule marked checkable with
no case:

1. **Write the predicate**, reading the rule's full text for what it actually
   requires. The failure message names the page, the rule and what the patch
   does instead — a message that only says "rule violated" makes the next reader
   re-derive what you already knew.
2. **Run it.** A rule that passes immediately is expected and fine: the page may
   already comply. A rule that fails has found a contradiction.
3. **For each contradiction**, stop and decide: correct the patch if the fix is
   mechanical and the rule is clearly right, or add a `patch-divergence-ok`
   marker with the untriaged reason and open a `B` row naming the page and rule.
   **Do not weaken a predicate to make a patch pass.** That is the failure
   M004/S03 avoided by arguing the case in a plan rather than adjusting a
   threshold.
4. **Prove every entry bites** — mutate the lane it guards, watch it fail,
   restore by inverse edit — and record each in the evidence file. This is the
   slice's second definition-of-done item, and with this many rules it is also
   the only thing separating a real checklist from a decorative one.
5. **Commit in page-sized batches**, each with `Rows: —` except the last, which
   carries `Rows: B13`. A single commit covering twenty rules is not reviewable.
6. **Check** the three tokens on each commit.

## Task 3 — Close the slice

1. **Confirm every checkable rule in `RULE_TRIAGE` has a case**, and that the
   triage's checkable count matches the checklist's entry count. A mismatch
   means a rule was triaged and skipped.
2. **Tick all three definition-of-done boxes**, tick Task 3, confirm B13 and B15
   are `done`, set the slice `Status` to `done`.
3. **Run every token**, then `jk-standards ledger`.
4. **Append the evidence** and **commit** with `Rows: —`.

## Self-review

**DoD coverage.** Item 1 (every checkable rule carries an entry) → Task 2, with
Task 3 step 1 verifying the counts agree. Item 2 (each proved to fail) → Task 2
step 4. Item 3 (a violating patch is corrected or marked) → Task 1 for B15, and
Task 2 step 3 for anything new.

**Row coverage.** B15 → Task 1 step 7. B13 → Task 2's last commit.

**Placeholder scan.** No TBDs. The scope is named as S01's output rather than a
number, and rotation 13 is given with an instruction to re-derive it.

**Name consistency.** `RULE_TRIAGE`, `ebb-kick-avoids-snare` and
`patch-divergence-ok` throughout.

**Ordering.** Task 1 first: it is one cell and it clears the last inherited
marker, so Task 2 starts from a tree with no suppressions and any marker it adds
is unambiguously its own.
