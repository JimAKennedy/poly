---
class: gated
---

# M007/S01 — The orphaned guards get a home

**Slice:** M007/S01 — `docs/plans/engine-capability/ledger.md`
**Rows:** GAP03
**Classification:** bounded. One wrapper script, one token, one doc line.

## Task status

- [ ] 1. Write `scripts/check-guards.sh` and declare the `guards` token
- [ ] 2. Prove it bites for both kinds of guard, name it in `CLAUDE.md`, close
      the slice

## Definition of Done

- [ ] Every guard listed in `GAP03` is reachable from a command declared in
      `.jk/validations.yml`
- [ ] Running that command on a tree that breaks one of them fails, shown for at
      least one guard of each kind — a `check-*.sh` and a `check-*.mjs`
- [ ] `CLAUDE.md` names the command, as it now names the doc-discipline one

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |
| `guards` | `bash scripts/check-guards.sh` (after task 1) |

## Context both tasks need

The eleven, measured against `.jk/validations.yml`, `scripts/pre-push-check.sh`,
`.pre-commit-config.yaml` and `scripts/check-doc-conformance.sh` together:

| Guard | CI job |
|---|---|
| `check-spdx-headers.sh` + `.mjs` | code-quality |
| `check-personal-paths.sh` + `.mjs` | code-quality |
| `check-site-readme.sh` + `.mjs` | code-quality |
| `check-scripts-readme.sh` + `.mjs` | code-quality |
| `check-sample-manifest.sh` (`--strict`, `--coverage`) | site-lint |
| `check-site-assets.sh` | site-lint |
| `check-bridge-schema-coverage.mjs` | site-lint |

- **Run them the way CI runs them.** `check-sample-manifest.sh` is invoked twice
  with different flags; the script must do the same or it checks less than CI.
- **Do not stop at the first failure.** A developer wants every broken guard in
  one run, not one per invocation. Collect failures, print them together, and
  exit non-zero if any failed.
- **Name each guard as it runs**, so a failure identifies itself without the
  developer re-running them one at a time — that is the cost the one-token
  decision accepted.

## Task 1 — Write the wrapper and declare the token

**Files:** `scripts/check-guards.sh` (new), `.jk/validations.yml`,
`scripts/README.md`

1. Write `scripts/check-guards.sh` with `set -uo pipefail` — **not** `-e`, since
   the script must keep going after a failing guard to report them all. Run each
   of the eleven, recording which failed, then print a summary and exit 1 if any
   did.
2. Declare `guards` in `.jk/validations.yml` pointing at it, with a comment
   saying what it covers and why it exists: these run in CI and, before this
   slice, from no local command.
3. Add the script to `scripts/README.md`. `check-scripts-readme.sh` is one of
   the guards the new script runs, so forgetting this fails the very command
   being added — which is a good sign it is wired correctly.
4. Run `bash scripts/check-guards.sh` on a clean tree and confirm it exits 0 and
   names all eleven.
5. Run `format` and `doc-discipline`. Append evidence to
   `docs/plans/engine-capability/evidence/M007-S01.md`, tick task 1, run
   `jk-standards ledger`, commit with the slice's trailers.

## Task 2 — Prove it bites, document it, and close the slice

A wrapper whose failure path has never fired is the defect this milestone
exists to remove, so both kinds get exercised.

**Files:** `CLAUDE.md`, `docs/plans/engine-capability/ledger.md`,
`docs/plans/engine-capability/evidence/M007-S01.md`

1. **Prove a `check-*.sh` failure surfaces.** Break one deliberately — adding a
   tracked file with no SPDX header trips `check-spdx-headers.sh`, or a personal
   path trips `check-personal-paths.sh`. Run the wrapper, confirm it exits
   non-zero and names that guard. Revert and confirm `git status` is clean.
2. **Prove a `check-*.mjs` failure surfaces**, by the same method against one of
   the four contract proofs. Revert and confirm clean.
3. **Prove it does not stop at the first failure**: break two guards at once and
   confirm the summary names both. This is the behaviour `-e` would have
   silently removed.
4. In `CLAUDE.md`, beside the doc-discipline note M006 added, name
   `bash scripts/check-guards.sh` and say what it covers — the checks CI runs in
   `code-quality` and `site-lint` that no other local command runs.
5. Run the full validation set. Append evidence, tick task 2, set row GAP03 to
   `done`, tick all three definition-of-done boxes, set slice M007/S01 to
   `done`, run `jk-standards ledger`, commit with the slice's trailers.
