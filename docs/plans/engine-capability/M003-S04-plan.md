---
class: gated
---

# M003/S04 — The guide catches up

**Slice:** `M003/S04` in `docs/plans/engine-capability/ledger.md`
**Depends:** `M003/S01`

## Task status

- [x] 1. The patch table carries a `Timing` column
- [x] 2. Rule 6 becomes checkable, mutation-proved, and close-out

## Definition of Done

Copied verbatim from the slice.

- [ ] `theory-brazilian`'s patch table carries a `Timing` column expressing the
      per-beat profile
- [ ] Rule 6 reads `checkable`, with a predicate that has been shown to fail when
      the profile is flattened
- [ ] The "until subdivision profiles ship" sentence is gone from the page

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |

## Task 1 — The patch table carries a `Timing` column

Modifies `site/src/content/docs/theory-brazilian.mdx`.

1. Add a `Timing` column to the page's `<PolyPatch>` table, carrying each lane's
   profile as the ratios themselves rather than a profile name — `EC11`'s
   verification flattens the profile and watches a predicate fail, which needs
   values in the table to flatten.
2. Update the patch's "Feel" construction step, which currently reads "Swing 0.2
   ± per-lane offsets as the Rule 6 approximation": the approximation is what
   this milestone removes.
3. Run `site-unit` and `doc-conformance` — the table parses and no existing case
   breaks. `parsePolyPatch` takes a title, so confirm the right table is being
   read if the page carries more than one. Commit.

## Task 2 — Rule 6 becomes checkable, mutation-proved, and close-out

Modifies `site/tests/theory-patch-conformance.test.mjs`,
`site/src/content/docs/theory-brazilian.mdx`, the evidence, the ledger, and this
plan.

1. Write the failing predicate first, registered for `theory-brazilian.mdx` in
   the per-page named-rule checklist. **`CHECKLIST.push(...)` must sit above the
   `let liveMarkers = 0;` line**, as every other case in that file does. The
   predicate reads the `Timing` column and asserts Rule 6's stated shape: first
   of each beat's four sixteenths long, middle two compressed, fourth long.
2. Flip Rule 6's triage entry from its `absentColumn` verdict to `checkable`.
3. Watch it fail by flattening the table's profile to an even grid — `EC11`'s
   stated verification — then restore and watch it pass. Restore with an explicit
   edit rather than `git checkout`, which in M007/S03 discarded uncommitted work
   in the same file.
4. Delete the "until subdivision profiles ship, use light swing (0.15–0.25) plus
   small per-lane offsets as an admitted approximation" clause from Rule 6,
   keeping the sentence that swing approximates the profile poorly — that remains
   true and is why the profile exists.
5. Run the whole validation set, append the evidence, tick the boxes, set `EC11`
   and the slice `done`, run `jk-standards ledger`, commit with
   `Slice: M003/S04` and `Rows: EC11`.
