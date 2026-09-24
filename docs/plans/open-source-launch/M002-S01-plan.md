# M002/S01 — Workflows hold least privilege

**Slice:** M002/S01 in `docs/plans/open-source-launch/ledger.md`
**Rows:** OS06 (no top-level `permissions:`), OS07 (no `concurrency:`
group), OS08 (a third-party checkout at a moving ref)
**Depends:** nothing.
**Decisions consumed:** `M002-decisions.md`, 2026-09-24 — the guard is a
local `scripts/` contract; `pr-af-review.yml` is pinned, not deleted; the
concurrency shape and the two workflows that gain a top-level block are
recorded there.

## Task status

- [x] Task 1 — The guard exists, is red on its fixtures, and every workflow
      declares a top-level `permissions:` block (OS06)
- [x] Task 2 — A superseded PR push cancels the run it replaces (OS07)
- [ ] Task 3 — The PR-AF checkout names a commit (OS08), and the slice closes

## Definition of Done

- [ ] Every workflow declares a top-level `permissions:` block, and jobs elevate
      locally only where they write
- [ ] A superseded push to a PR cancels the run it replaces
- [ ] No workflow checks out or executes third-party code at a moving ref
- [ ] A guard fails on a workflow with no top-level `permissions:` and on an
      unpinned third-party checkout, each seen red — the three jk-standards
      workflow checks all pass on today's tree, so none of them is that guard

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `guards` | `bash scripts/check-guards.sh` |

The task's own check, run first every time: `node --test
scripts/check-workflow-hygiene.mjs`. The three jk-standards workflow checks
(`jk-standards action-pinning`, `workflow-permissions`,
`workflow-concurrency`) are run after each task as a regression check — they
pass today and must still pass.

## Shape of the guard

`scripts/check-workflow-hygiene.mjs` follows `check-version-source.mjs`: a
`node --test` file, a header comment naming the row it serves, no YAML
dependency — targeted structural matches over the workflow text, as
`check-release-workflow.mjs` does. It exports nothing; it defines three pure
functions over a workflow's text and asserts them twice each: once against
an inline fixture that must produce a finding (the red proof), once against
every file under `.github/workflows/` (the tree). The functions:

- `missingTopLevelPermissions(text)` — true when no line matches
  `/^permissions:/m`. A block at column 0 is top-level; a job-level block is
  indented and does not count.
- `unpinnedThirdPartyCheckouts(text)` — the list of `repository:` values in
  any `with:` block whose value is not `${{ github.repository }}` and does not
  start with `JimAKennedy/`, where the same step carries no `ref:` that is a
  40-character hex SHA. A step is the text between one `- uses:`/`- name:`
  line and the next.
- `concurrencyShape(text)` — an object `{ group, cancel }` from the top-level
  `concurrency:` block, or `null` when absent.

## Task 1 — The guard exists, is red on its fixtures, and every workflow declares a top-level `permissions:` block (OS06)

**Files:** create `scripts/check-workflow-hygiene.mjs`; modify
`.github/workflows/ci.yml`, `.github/workflows/sanitizers.yml`,
`scripts/check-guards.sh`, `scripts/README.md`.
**Produces for later tasks:** the guard file and its three functions; tasks
2 and 3 add one tree-level assertion each.

1. Write `scripts/check-workflow-hygiene.mjs` with the three functions and
   these tests:
   - `missingTopLevelPermissions` fires on a fixture with only a job-level
     block (`jobs:\n  a:\n    permissions:\n      contents: read`) and stays
     silent on a fixture whose first line is `permissions:`.
   - `unpinnedThirdPartyCheckouts` returns `['Agent-Field/pr-af']` for a
     fixture step with `repository: Agent-Field/pr-af` and no `ref:`, returns
     `[]` when the same step carries `ref: ` followed by 40 hex characters,
     and returns `[]` for `repository: ${{ github.repository }}`.
   - `concurrencyShape` returns `null` on a fixture without the block and
     `{ group: '${{ github.workflow }}-${{ github.ref }}', cancel:
     "${{ github.event_name == 'pull_request' }}" }` on one with it.
   - Tree: for every `*.yml` under `.github/workflows/`, a test named
     `<file> declares a top-level permissions block` asserts
     `missingTopLevelPermissions` is false.
   - Tree: `ci.yml`'s top-level block grants exactly `contents: read` — the
     block's indented lines are `contents: read` and nothing else — and the
     `secrets-scan` job still carries its own `pull-requests: write`.
2. Run `node --test scripts/check-workflow-hygiene.mjs`. Expect the fixture
   tests green and exactly two tree tests red: `ci.yml` and `sanitizers.yml`.
   Record both names in the evidence.
3. Add to `ci.yml`, between `on:` and `jobs:`:
   ```yaml
   # OS06: the token every job receives unless it says otherwise. The
   # repository default is already read-only; declaring it here makes the
   # grant visible in the tree and lets scripts/check-workflow-hygiene.mjs
   # hold it there.
   permissions:
     contents: read
   ```
   Add the same block, with the same comment, to `sanitizers.yml` between
   `on:` and `jobs:`. Leave `secrets-scan` and `notify` as they are.
4. Run the guard again: all green.
5. Wire it: in `scripts/check-guards.sh` add
   `run_guard "workflow-hygiene contract" node --test scripts/check-workflow-hygiene.mjs`
   directly after the `release-workflow contract` line, with a two-line
   comment naming OS06 and OS08. In `ci.yml`'s `code-quality` job add a step
   `- name: Workflow hygiene contract (top-level permissions, pinned third-party checkouts, PR concurrency)`
   running `node --test scripts/check-workflow-hygiene.mjs` after the
   `Scripts-readme guard contract` step. Add the script to `scripts/README.md`
   beside `check-version-source.mjs`, one bullet in the file's style.
6. Run `bash scripts/check-scripts-readme.sh` (the README guard) and
   `bash scripts/check-guards.sh`: green.
7. Run `jk-standards action-pinning`, `jk-standards workflow-permissions`,
   `jk-standards workflow-concurrency`: all pass, as before.
8. Run `format` and `guards`. Append evidence. Tick this task's box. Commit
   with `Rows: OS06`; the ledger row OS06 becomes `done`.

## Task 2 — A superseded PR push cancels the run it replaces (OS07)

**Files:** modify `.github/workflows/ci.yml`,
`scripts/check-workflow-hygiene.mjs`.
**Consumes:** `concurrencyShape` from task 1.

1. Add a tree test to the guard: `ci.yml cancels superseded pull-request
   runs and never a push to main` asserting `concurrencyShape(ci)` deep-equals
   `{ group: '${{ github.workflow }}-${{ github.ref }}', cancel:
   "${{ github.event_name == 'pull_request' }}" }`.
2. Run the guard: exactly that test red (`null` today).
3. Add to `ci.yml` directly after the `permissions:` block from task 1:
   ```yaml
   # OS07: a newer push to the same PR cancels the run it supersedes, so the
   # macOS and Windows matrix stops burning minutes on a commit nobody will
   # merge. Pushes to main are never cancelled — every one is a candidate
   # release build. The group is ref-scoped, which is what
   # jk-standards workflow-concurrency requires of a non-global group.
   concurrency:
     group: ${{ github.workflow }}-${{ github.ref }}
     cancel-in-progress: ${{ github.event_name == 'pull_request' }}
   ```
4. Run the guard: green. Run `jk-standards workflow-concurrency`: passes —
   the group is ref-scoped, so the check has nothing to say.
5. Run `format` and `guards`. Append evidence. Tick the box. Commit with
   `Rows: OS07`; OS07 becomes `done`.

## Task 3 — The PR-AF checkout names a commit (OS08), and the slice closes

**Files:** modify `.github/workflows/pr-af-review.yml`,
`scripts/check-workflow-hygiene.mjs`, `docs/plans/open-source-launch/ledger.md`.
**Consumes:** `unpinnedThirdPartyCheckouts` from task 1.

1. Add a tree test to the guard: for every workflow,
   `<file> pins every third-party checkout to a commit` asserts
   `unpinnedThirdPartyCheckouts` is empty.
2. Run the guard: exactly `pr-af-review.yml` red, naming `Agent-Field/pr-af`.
3. In `pr-af-review.yml`'s `Checkout PR-AF` step add, under `with:`, after
   `repository:`:
   ```yaml
   # OS08: pinned like every action — a third-party repository executed with
   # a secret in its environment must not float. Upstream main as of
   # 2026-09-21; bump deliberately, with a diff read.
   ref: 421fbd23bf1c5a2c3916d7046c97b5273958e1f4
   ```
4. Run the guard: green. Run `jk-standards action-pinning`: passes.
5. Read `docs/pr-af-review.md`. If it describes the checkout or the upstream
   ref, add one sentence saying the workflow pins upstream by commit and where
   to bump it. If it does not, leave it and carry a `Docs-Not-Affected`
   trailer on the commit in the form M001 used:
   `Docs-Not-Affected: docs/pr-af-review.md — pr-af-review.yml pins the upstream checkout to a commit; the trigger label, model, cost and cadence the doc describes are unchanged`.
6. Run `format` and `guards`. Run `bash scripts/check-doc-discipline.sh`
   once, even though the slice does not owe `doc-discipline`: the drift map
   pairs this workflow with its doc, and the trailer must satisfy it.
7. Tick every Definition of Done box in this plan and in the ledger; set the
   slice `done`; OS08 `done`. Append evidence with the DoD-to-task table.
   Run `jk-standards ledger`. Commit with `Rows: OS08`.

## Self-review

| DoD | Task |
|---|---|
| Every workflow declares a top-level block; jobs elevate locally only where they write | 1 — `ci.yml` and `sanitizers.yml` gain the block; `secrets-scan` and `notify` keep theirs; the tree test covers all eight files |
| A superseded PR push cancels the run it replaces | 2 |
| No workflow executes third-party code at a moving ref | 3 — the only `repository:` checkout is pinned; every `uses:` already is (`action-pinning`) |
| A guard fails on each, seen red | 1, 2, 3 — fixture tests are red by construction; each tree assertion was red before its fix |

| Row | Task | Verification produced |
|---|---|---|
| OS06 | 1 | top-level `contents: read`; `secrets-scan` keeps its block; the guard fails on a workflow without one |
| OS07 | 2 | ref-scoped group, `cancel-in-progress` for pull requests only |
| OS08 | 3 | 40-character SHA with the upstream branch and date in a comment; the guard fails without it |

Names used throughout: `missingTopLevelPermissions`,
`unpinnedThirdPartyCheckouts`, `concurrencyShape`. No placeholders.
