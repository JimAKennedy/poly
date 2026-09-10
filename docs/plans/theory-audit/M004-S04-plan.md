# M004/S04 — Tihai worked example

**Slice:** M004/S04, in `docs/plans/theory-audit/ledger.md`
**Depends:** nothing — the tihai example needs neither the checklist nor the
divergence marker, which is why this slice was left independent of M004/S05
**Decisions:** `M004-decisions.md`

## Task status

- [x] Task 1 — Work the tihai arithmetic through with the patch's own numbers
- [ ] Task 2 — Close the slice

## Definition of Done

- [ ] The tihai discussion shows Nelson's formula with real numbers, not just
      the principle
- [ ] The arithmetic in the example is checked, not asserted

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

No `e2e`: this slice adds prose and a test, and changes no patch table, so
nothing it does can move the Play card or the Try It modal.

## What is already true

Read these before writing anything — F43's item overstates what is missing.

- **Rule 6 already states the formula**, as `3 × phrase + 2 × gap ≡ remaining
  matras (mod cycle)`, and already cites
  [Nelson 2008](/appendix-references/#fr-nelson-2008), whose entry exists at
  `fr-nelson-2008`. Neither the formula nor the citation needs adding.
- **The worked example's numbers are already in the page**, in its own patch:
  the `Tihai phrase` lane carries `Phrase Len` 5 and `Gap` 0.5 against a
  16-matra teental cycle. `3 × 5 + 2 × 0.5 = 16`, one full cycle, so the third
  statement's final stroke lands on sam.

What is missing is only that nobody works it through. The example must use the
patch's numbers rather than invented ones, so the prose and the lane cannot
drift apart.

## Task 1 — Work the tihai arithmetic through

Closes **F43**.

**Files:** `site/src/content/docs/theory-indian-classical.mdx`,
`site/tests/theory-patch-conformance.test.mjs`

1. **Write the failing case** as a checklist rule on
   `theory-indian-classical.mdx`, id `ind-tihai-worked`, against the patch
   `Rule-Checked Teental Frame` — confirm that title from the file rather than
   trusting it here. It must:

   - read `Phrase Len` and `Gap` from the `Tihai phrase` lane
   - compute `3 × phrase + 2 × gap`
   - assert the page's prose prints that product, and prints both operands
   - assert the product equals the lane's `Steps`, which is the cycle it must
     close

   Assert against values read from the table, never against `5`, `0.5` or `16`
   written into the test. The whole point of the row is that the arithmetic is
   checked rather than asserted, and a test carrying its own copy of the numbers
   checks nothing about the page.

2. **Run it and watch it fail** — the prose does not exist yet. Confirm it fails
   on the missing prose, not on a parse error or a missing column.

3. **Write the worked example** beneath Rule 6 or beside the patch, in the
   page's voice. Show the substitution with the patch's own numbers and say what
   the result means: three statements and two gaps fill the cycle exactly, so
   the third stroke lands on sam. Do not restate Rule 6's formula as though it
   were new — it is already there, one line above.

4. **Run and watch it pass.**

5. **Prove it bites, twice.** Change the printed product, watch it fail;
   restore. Then change the lane's `Gap` cell, watch it fail because the derived
   product no longer matches the printed one; restore. The second mutation is
   the one that matters — it is what proves the case couples the prose to the
   table rather than to a constant.

6. **Check.** `npm --prefix site test`,
   `bash scripts/check-doc-conformance.sh`, `pre-commit run --all-files`, and
   `npm --prefix site run build` if the prose uses braces, since MDX evaluates
   `{...}` as an expression.

7. **Commit** with `Rows: F43`, ticking Task 1, setting F43 `done` naming
   `ind-tihai-worked` in its `Verification`, and appending to
   `docs/plans/theory-audit/evidence/M004-S04.md`.

## Task 2 — Close the slice

1. **Tick both definition-of-done boxes** in the slice and this plan, tick Task
   2, confirm F43 is `done`, set the slice `Status` to `done`.
2. **Run every token** the slice declares. There is no `e2e` here, so no WASM
   artifacts are rebuilt and nothing needs restoring.
3. **Run `jk-standards ledger`** with the slice `done`.
4. **Append the evidence** and **commit** with `Rows: —`.

## Self-review

**DoD coverage.** Item 1 (formula with real numbers) → Task 1 step 3. Item 2
(arithmetic checked, not asserted) → Task 1 step 1, and specifically step 5's
second mutation, which is what distinguishes a checked example from a printed
one.

**Row coverage.** F43 → Task 1 step 7; its verification is `ind-tihai-worked`,
created at step 1.

**Placeholder scan.** No TBDs. The numbers 5, 0.5 and 16 appear in this plan as
the measured state of the page, and every step requires reading them from the
table instead.

**Name consistency.** `ind-tihai-worked` throughout.
