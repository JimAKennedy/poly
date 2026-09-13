---
class: gated
---

# M006/S02 — Every site test runs in CI

**Slice:** M006/S02 — `docs/plans/engine-capability/ledger.md`
**Rows:** GAP02
**Classification:** bounded. One step added to an existing CI job, and one
assertion that the step stays there.

## Task status

- [x] 1. Run the whole site suite in CI
- [x] 2. Guard the step against quiet removal, and close the slice

## Definition of Done

- [ ] Every `site/tests/*.test.mjs` file runs in at least one CI job
- [ ] A test file added to that directory cannot silently go unrun — something
      fails if it is covered by nothing
- [ ] #272's counts are corrected to what the tree holds, or the issue is closed
      by this work

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `site-unit` | `npm --prefix site test` |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` |

## Context both tasks need

- **The gap, measured.** No CI job runs `npm --prefix site test`. The only
  `site/tests/**` files CI executes are those named in
  `scripts/check-doc-conformance.sh`, so six run nowhere: `bjorklund`,
  `dump-mode`, `preset-patterns`, `presets-json-schema`, `sample-loader`,
  `smf-writer`. Issue [#272](https://github.com/JimAKennedy/poly/issues/272)
  raised this as 7 of 21; the tree now holds 23 files with 6 uncovered, so the
  issue's numbers are stale and this slice owes the correction.
- **Why this is not hypothetical.** `presets-json-schema.test.mjs` carries
  M005/S01's staleness guard. The check that catches a stale `presets.json` is
  itself unproven in CI, which is why M005/S02 deliberately put its conformance
  case in a protected file instead.
- **The site test script is `node --test tests/**/*.test.mjs`** — a directory
  glob. Once CI runs it, every file in `site/tests` runs by construction and a
  new file is picked up with no wiring. That is why this slice writes no
  "orphaned test" guard: the orphan case stops existing. What needs guarding is
  the CI step itself, which task 2 covers.
- **The job to extend is `site-lint`.** It already runs `npm ci` in `site/` and
  already executes a test suite via `check-doc-conformance.sh`, so the
  dependencies are installed and no `ci-complete` wiring changes.

## Task 1 — Run the whole site suite in CI

**Files:** `.github/workflows/ci.yml`

1. In the `site-lint` job, add a step after the doc-conformance step:
   `name: Site unit tests (all of site/tests)` running
   `npm --prefix site test`. Place it after the existing checks so a doc
   failure still reports first — the conformance suite is the more specific
   signal.
2. Confirm locally that the command covers the whole directory: run
   `npm --prefix site test` and check the file count it executes against
   `ls -1 site/tests/*.test.mjs | wc -l`. They must agree; if they do not, the
   glob is not doing what this slice assumes and the plan is wrong — stop.
3. Run `format` and `site-unit`. Append evidence to
   `docs/plans/engine-capability/evidence/M006-S02.md`, tick task 1, run
   `jk-standards ledger`, and commit with the slice's trailers.

## Task 2 — Guard the step, and close the slice

Nothing currently notices if the CI step is deleted. The repo already has this
pattern: `site/tests/doc-conformance-wiring.test.mjs` asserts that
`check-doc-conformance.sh` runs the files it is supposed to. This is the same
guard one level up.

**Files:** `site/tests/doc-conformance-wiring.test.mjs` (or a sibling — decide
when you see the file, and say which in the evidence),
`docs/plans/engine-capability/ledger.md`,
`docs/plans/engine-capability/evidence/M006-S02.md`

1. Add a case that reads `.github/workflows/ci.yml` and asserts the `site-lint`
   job contains a step running `npm --prefix site test`. Assert on the command
   string rather than the step's `name`, since a name is cosmetic and a command
   is the thing that runs.
2. Run `site-unit` and watch it pass.
3. **Mutation-prove it:** delete the CI step, re-run, confirm the case fails
   naming what is missing. Restore, confirm `git diff --quiet` on the workflow,
   and re-run green.
4. Confirm the case is itself protected: the file it lives in must be named in
   `scripts/check-doc-conformance.sh`, or it is a guard that only runs on a
   developer's machine — the precise failure this milestone exists to remove.
   Check, and if it is not protected, move the case to a file that is.
5. Run the full validation set. Append evidence, tick task 2, set row GAP02 to
   `done`, tick all three definition-of-done boxes, set slice M006/S02 to
   `done`, run `jk-standards ledger`, and commit with the slice's trailers. The
   pull request body corrects #272's counts and closes it; record in the
   evidence that this is where the issue is discharged.
