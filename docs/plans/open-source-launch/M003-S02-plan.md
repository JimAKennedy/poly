# M003/S02 — The editor is exercised on both platforms

**Slice:** M003/S02 in `docs/plans/open-source-launch/ledger.md`
**Rows:** OS16 (pluginval skips its GUI tests on all four legs)
**Depends:** nothing.
**Decisions consumed:** `M003-decisions.md`, 2026-09-29 — flip both legs now;
macOS is proved locally, Windows by the milestone PR's run; the local
pre-push gate keeps its flag.

## Task status

- [x] Task 1 — pluginval runs its GUI tests on all four CI legs, the contract
      asserts it, and the slice closes (OS16)

## Definition of Done

- [x] pluginval runs its GUI tests on the macOS CI leg, or the evidence records
      why a hosted runner cannot and what covers the editor instead
- [x] The same is true of the Windows leg
- [x] The release workflow matches whatever CI settles on

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `unit` | `cmake --build build --config Release --parallel && ctest --test-dir build --build-config Release --output-on-failure` |
| `guards` | `bash scripts/check-guards.sh` |

The task's own check, run first: `node --test scripts/check-release-workflow.mjs`.

## Task 1 — pluginval runs its GUI tests on all four CI legs, the contract asserts it, and the slice closes (OS16)

**Files:** modify `.github/workflows/ci.yml`, `.github/workflows/release.yml`,
`scripts/check-release-workflow.mjs`, `docs/plans/open-source-launch/ledger.md`;
create `docs/plans/open-source-launch/evidence/M003-S02.md`.

1. In `scripts/check-release-workflow.mjs`, change the test named
   `pluginval strictness/flags are locked on BOTH legs (level 8, skip-gui, 120s)`
   to `pluginval strictness/flags are locked on BOTH legs (level 8, GUI tests on, 120s)`:
   the `--skip-gui-tests` count must be 0, with a message naming OS16 — the
   editor is a WebView, the likeliest source of a host crash on open, close
   and reopen, and skipping its tests on the configuration that ships would
   hide exactly that.
2. Run `node --test scripts/check-release-workflow.mjs`: that test is red
   (two matches). Record it.
3. Remove the `--skip-gui-tests \` line from both pluginval steps in
   `release.yml` and both in `ci.yml`. Above each `ci.yml` step add a
   two-line comment: OS16 — GUI tests on, macOS proved locally at strictness
   8 on 2026-09-29, Windows proved by this workflow's run on the M003 PR.
   Keep `--strictness-level 8` and `--timeout-ms 120000` as they are.
4. Run the contract: green, 32 of 32. Run
   `node --test scripts/check-workflow-hygiene.mjs`: still green.
5. Run pluginval locally against `build/VST3/Release/poly_plugin.vst3` at
   strictness 8 with no `--skip-gui-tests` and `--timeout-ms 120000`; it
   exits 0 and the log names the Editor, Open editor whilst processing and
   Editor Automation tests as completed. Record the exit code and those three
   lines in the evidence as the macOS proof.
6. Run `format`, `unit`, `guards`. Write the evidence: the red contract, the
   flip, the local macOS run, and that the Windows proof is the milestone
   PR's `pluginval-windows` job — with the fallback the DoD allows if it is
   red. Tick the task box and every DoD box here and in the ledger; set the
   slice `done`; OS16 `done`. Run `jk-standards ledger`. Commit with
   `Rows: OS16`.

## Self-review

| DoD | Task |
|---|---|
| GUI tests on the macOS leg | 1 — flag removed; local run at strictness 8 is the proof |
| The same on Windows | 1 — flag removed; the PR's run is the proof, by the owner's decision, with the DoD's fallback named |
| The release workflow matches CI | 1 — both `release.yml` legs flipped in the same commit; the contract asserts zero flags |

| Row | Task | Verification produced |
|---|---|---|
| OS16 | 1 | the flag is removed on every leg; the macOS run is green; Windows is the PR's job; the local gate's retained flag is recorded in the decisions file |

No placeholders.
