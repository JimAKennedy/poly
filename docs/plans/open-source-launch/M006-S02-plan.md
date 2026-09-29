# M006/S02 — An unsigned build cannot ship quietly

**Slice:** M006/S02 in `docs/plans/open-source-launch/ledger.md`
**Rows:** OS34 (the signing steps skip when the secrets are absent, and the
output does not say whether it signed)
**Depends:** M001/S02 (done). Runs before M006/S01 by the amendment of
2026-09-29.
**Decisions consumed:** `M006-decisions.md`, 2026-09-29 — one gate step per
leg; Signed lines appended by the release job; `ALLOW_UNSIGNED_RELEASE` set
through the API.

**Before the first task:** if `main` has moved since this branch was cut
(M005/S01 merging is expected), `git rebase origin/main` and re-run
`bash scripts/pre-push-check.sh`; a rebase that compiles is not a rebase
that passes.

## Task status

- [ ] Task 1 — Each leg fails unsigned unless the variable allows it, the
      body says whether each artifact is signed, and the contract asserts
      both (OS34); the slice closes

## Definition of Done

- [ ] With no signing secrets, the release job fails, unless a repository
      variable named for the purpose explicitly allows an unsigned release
- [ ] The Release body states whether each artifact is signed and notarized
- [ ] The contract check asserts both, each seen red

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `guards` | `bash scripts/check-guards.sh` |

The task's own check, run first: `node --test scripts/check-release-workflow.mjs`.

## Task 1 — Each leg fails unsigned unless the variable allows it, the body says whether each artifact is signed, and the contract asserts both (OS34); the slice closes

**Files:** modify `.github/workflows/release.yml`,
`scripts/check-release-workflow.mjs`, `RELEASING.md`,
`docs/plans/open-source-launch/ledger.md`; create
`docs/plans/open-source-launch/evidence/M006-S02.md`. The repository
variable, through the API.

1. Add to `check-release-workflow.mjs`:
   - `each leg refuses to ship unsigned unless ALLOW_UNSIGNED_RELEASE is
     true (OS34)`: a step named `Require signing or an explicit allowance`
     appears twice, each referencing `vars.ALLOW_UNSIGNED_RELEASE` and
     `exit 1`, and each placed after the last signing step of its leg and
     before its packaging step.
   - `each leg records whether it signed, and the release job says so in
     the body (OS34)`: both legs write a `signed-*.txt` file uploaded with
     the zip; the release job has a step `Append signing status to the
     body` that reads them and appends a line beginning `Signed:` to
     `release-notes.md` before `Publish GitHub Release`.
   Run: both red.
2. `release.yml`, macOS leg: the codesign step sets an output
   `signed=yes` (a `$GITHUB_OUTPUT` line at its end) and the staple step
   `notarized=yes`; after the staple step, a step `Record signing status
   (macOS)` writes `signed=<yes|no>` and `notarized=<yes|no>` from those
   outputs (default `no`) to `signed-macos-universal.txt`; then `Require
   signing or an explicit allowance` fails with a message naming
   `ALLOW_UNSIGNED_RELEASE` when `signed` is `no` and
   `vars.ALLOW_UNSIGNED_RELEASE != 'true'`. Windows leg: `Record signing
   status (Windows)` writes `signed=no` (no signing step exists until M008)
   and the same gate. Both `signed-*.txt` files join the `Upload release
   asset` step's `path`.
3. Release job: after `Generate release notes`, `Append signing status to
   the body` reads `dist/signed-*.txt` and appends a blank line and
   `Signed: macOS <yes|no> (notarized <yes|no>) · Windows <yes|no>` to
   `release-notes.md`; the `files:` of the publish step must not include the
   `signed-*.txt` files (they are not assets), so add `dist/signed-*.txt`
   removal or narrow the glob — narrow: `dist/*.zip` and `dist/SHA256SUMS`
   are already the only globs, and `sha256sum *.zip` ignores the text
   files, so nothing changes there.
4. Run the contract: green. Run `node --test scripts/check-workflow-hygiene.mjs`:
   still green. Parse the YAML.
5. `gh variable set ALLOW_UNSIGNED_RELEASE --body true`; read it back with
   `gh variable list`. Record in the evidence with the reason: the first cut
   is unsigned by design, M007/S02 signs, and the variable is removed then.
6. `RELEASING.md`: a paragraph on the gate and the variable.
7. Run `format`, `guards`. Tick the task box and every DoD box here and in
   the ledger; set the slice `done`; OS34 `done`. Evidence. Run
   `jk-standards ledger`. Commit with `Rows: OS34`.

## Self-review

| DoD | Task |
|---|---|
| Fails unsigned unless the variable allows | 1, held by the contract |
| The body states signed and notarized per artifact | 1, held by the contract |
| The contract asserts both, each seen red | 1 |

| Row | Task | Verification produced |
|---|---|---|
| OS34 | 1 | the gate step per leg; the Signed line; the contract's two cases |

No placeholders.
