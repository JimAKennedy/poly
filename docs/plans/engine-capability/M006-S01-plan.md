---
class: gated
---

# M006/S01 — `doc-drift` is runnable locally

**Slice:** M006/S01 — `docs/plans/engine-capability/ledger.md`
**Rows:** GAP01
**Classification:** bounded. One wrapper script, one token repointed.

## Task status

- [x] 1. Write the wrapper and repoint the `doc-discipline` token
- [x] 2. Prove all three behaviours, document the command, close the slice

## Definition of Done

- [ ] A developer can run the `doc-drift` check against the default branch with
      a documented command, without knowing to set an environment variable by
      hand
- [ ] The `doc-discipline` token no longer reports success while silently
      skipping a check CI enforces — either it runs `doc-drift`, or a separate
      declared token does and slices that owe it name it
- [ ] A run genuinely unable to determine a base still explains why rather than
      failing, so a detached or shallow checkout is not made unusable

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` (after task 1; `jk-standards all` before it) |

## Context both tasks need

- **The defect, measured.** `jk-standards all` with no base prints
  `doc-drift: no --base or GITHUB_BASE_REF — skipped` and exits 0. With a base
  that cannot be resolved — `--base origin/nonexistent-ref` — it prints **no
  `doc-drift` line at all** and still exits 0. Both were confirmed on this tree.
- **Why a bare `--base` is not the fix.** A token reading
  `jk-standards all --base origin/main` passes while the check never runs on any
  clone where `origin/main` is not fetched. That is the present bug relocated,
  not removed.
- **The contract the wrapper must hold**, and the distinction the whole slice
  turns on:
  - a base **is** determinable → run the check, and *assert it ran*; fail if no
    `doc-drift` line appears
  - a base is **not** determinable → say so in plain words and exit 0, because
    a detached HEAD or shallow clone must stay usable
  Explaining is not the same as silently skipping. The present behaviour is a
  skip line that reads like a pass in a wall of green output.
- **CI already runs this correctly** via `GITHUB_BASE_REF` in the `site-lint`
  job. The wrapper must not break that path — if the variable is set, honour it
  rather than second-guessing it.

## Task 1 — Write the wrapper and repoint the token

**Files:** `scripts/check-doc-discipline.sh` (new), `.jk/validations.yml`

1. Write `scripts/check-doc-discipline.sh`:
   - Resolve the base in this order: `$GITHUB_BASE_REF` if set (as CI sets it),
     else `origin/<default-branch>` if that ref resolves, else no base.
   - With a base: run `jk-standards all --base "$base"`, capturing output while
     still showing it, and exit non-zero if the run failed.
   - **Then assert the check ran**: if the captured output contains no line
     beginning `doc-drift`, exit non-zero with a message saying doc-drift did
     not run and that the script refuses to report success for a check that did
     not execute. This arm is the point of the script.
   - With no base: run `jk-standards all` plainly, print a clearly-worded line
     saying doc-drift could not run and why — no base ref available — and exit
     with the run's own status.
   - Set `set -euo pipefail` and follow the style of the repo's existing
     `scripts/check-*.sh`.
2. In `.jk/validations.yml`, repoint `doc-discipline` to
   `bash scripts/check-doc-discipline.sh`, and rewrite its comment to say what
   the script adds over `jk-standards all`: it supplies the base ref CI supplies
   and refuses to pass if doc-drift did not execute.
3. Run `bash scripts/check-doc-discipline.sh` on this branch and confirm it
   exits 0 **and** prints a `doc-drift` line. If it prints no such line the
   script is not doing its job — stop, because that is the defect it exists to
   remove.
4. Run `format`. Append evidence to
   `docs/plans/engine-capability/evidence/M006-S01.md`, tick task 1, run
   `jk-standards ledger`, and commit with the slice's trailers.

## Task 2 — Prove all three behaviours, document it, and close the slice

A wrapper whose failure arm has never fired is exactly the kind of check this
milestone exists to remove. All three paths get exercised.

**Files:** `CLAUDE.md`, `docs/plans/engine-capability/ledger.md`,
`docs/plans/engine-capability/evidence/M006-S01.md`

1. **Prove it bites on a real violation.** Make a change that trips a drift
   mapping — touching `engine/tools/emit_presets.cpp` without
   `docs/preset-taxonomy.md` is the mapping M005 hit, and a whitespace-only edit
   is enough. Commit it on a scratch commit, run the script, and confirm it
   fails naming the drift violation. Reset the scratch commit with
   `git reset --hard HEAD~1` and confirm the script passes again.
2. **Prove the assertion arm fires.** Temporarily point the script at a base
   that cannot resolve (edit the resolution to use a nonexistent ref), run it,
   and confirm it fails with the "doc-drift did not run" message rather than
   passing. Revert the edit and confirm `git diff --quiet` on the script.
3. **Prove the no-base path explains rather than fails.** Run the script in an
   environment where no base is determinable — unset `GITHUB_BASE_REF` and pass
   a repo state with no `origin` remote resolution, or invoke the resolution
   helper directly — and confirm it prints the explanation and exits 0.
   Record exactly how this was arranged in the evidence, since it is the
   hardest of the three to stage honestly.
4. In `CLAUDE.md`, under the Pre-Push Quality Gate section, add a short note
   naming `bash scripts/check-doc-discipline.sh` as the way to run the doc
   checks including doc-drift, and stating that `jk-standards all` alone skips
   doc-drift. A developer reading only `CLAUDE.md` must not be left with the
   impression that `jk-standards all` is the whole doc gate.
5. Run the full validation set. Append evidence, tick task 2, set row GAP01 to
   `done`, tick all three definition-of-done boxes, set slice M006/S01 to
   `done`, run `jk-standards ledger`, and commit with the slice's trailers.
