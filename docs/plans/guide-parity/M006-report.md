---
class: gated
---

# M006 — The e2e gate tests the server it started

> `scripts/site-verify-local.sh` either serves the site it just built or refuses to run, and leaves nothing listening when it exits.

Ledger: `docs/plans/guide-parity/ledger.md` · branch
`milestone/M006-preview-server` · 9 commits including this report, **none untraced**

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M006/S01 | The preview server is the script's own | GP15 | done |

## What changed

- **Two causes, not one.** The row was written from central-africa M006's
  report and blamed the cleanup for killing the wrapper. Planning measured
  Astro 7.3.8 detaching `astro preview` whenever an AI agent runs it, and
  replaced the row with that. Task 1's live test then went red with the fix
  for it alone: that planning measurement had run in zsh, which execs a
  subshell's last command, while the script's `/bin/bash` forks it, so the
  PID the script held was never the server's, for anyone. The ledger,
  plan and decisions record both causes and the correction.
- **A third cause, found by CI on the ship's PR (Task 4).** `site-e2e`
  passed every gate, then the new cleanup check failed: the server was still
  listening. On Linux npm runs the bin as `sh -c 'astro' …`, and dash forks
  it where macOS's `sh` execs it, so the SIGTERM `npm exec` forwarded killed
  that shell and orphaned the server to PID 1. Reproduced under Debian in a
  container; `preview_stop` now signals the whole tree, collected before the
  first signal, and counts a zombie as stopped.
- **`scripts/lib/preview-server.sh`** starts the preview with
  `exec npx astro preview … --ignore-lock`, stops it, and names whatever
  holds a port (`lsof`, with a `curl` fallback).
- **`site-verify-local.sh`** starts and stops its preview only through the
  helper. It refuses a held port before starting, naming the holder, and its
  cleanup fails the run if anything is still listening. Cleanup now runs on
  EXIT, with INT and TERM exiting 130 and 143, so an interrupted run cannot
  report success.
- **`scripts/lib/preview-server.test.mjs`**, 6 cases, runs in
  `check-guards.sh` (25 invocations, from 24) and in CI's code-quality job.
  Its live case sets `CLAUDECODE=1` and is red with either `exec` or
  `--ignore-lock` removed. It skips with its reason where `site/dist` is not
  built, which is CI's code-quality job. Its contract case is red if the
  script starts astro itself or drops either check. A sixth case, added in
  Task 4, puts a listener below a forking shell on every OS and asserts the
  port is free the moment `preview_stop` returns; it needs no `site/dist`, so
  it runs in CI's code-quality job.
- **`CLAUDE.md`** gains "A Backgrounded Server Must Be the Process You Stop"
  in Key Conventions.

## Definition of done

**M006/S01 — The preview server is the script's own**

- [x] The script refuses to start when its preview port is already in use,
      naming the process that holds it
- [x] No preview server is left listening after the script exits, whether it
      passes, fails or is interrupted
- [x] A test fails if either regresses

## Validation

From `evidence/M006-S01.md`, each task's last run:

| Task | Token | Result |
|---|---|---|
| 1 | `guards` · `format` | exit 0, 25 invocations, the helper's 4 cases pass, none skipped · exit 0 |
| 2 | `guards` · `format` | exit 0, contract case red first then 5 pass · exit 0 |
| 3 | `e2e`, port held | exit 1 at `[4/5]`, naming the holder's PID |
| 3 | `e2e` | exit 0, every gate PASS, 4322 free afterwards |
| 3 | `e2e`, SIGINT mid-gate | exit 130, 4322 free afterwards |
| 3 | `format` · `guards` · `doc-discipline` | exit 0 · exit 0 · exit 0 |
| 4 | helper test, macOS · Linux | forking case red first on macOS; it and the live case red under Linux against the Task 1 helper; 6 pass on both after |
| 4 | `e2e` | exit 0, every gate PASS, 4322 free afterwards |
| 4 | `format` · `guards` · `doc-discipline` | exit 0 · exit 0, 25 invocations · exit 0 |

## Look at twice

- **The planning finding was wrong, and the owner's row decision was made on
  it.** You approved "correct the row to the measured cause" when that cause
  was half the story. The row now carries both causes. The decisions file
  keeps the wrong planning entry and the correction below it, because it is
  append-only.
- **`exec` is beyond the approved design**, recorded as a judgment call: one
  word inside `preview_start`, without which the design missed the
  definition of done.
- **The trap change** (EXIT only, INT → 130, TERM → 143) is a second
  judgment call, beyond the plan's step.
- **GP15's verification was replaced** at your decision: the helper's live
  test and the e2e self-check replace "a test runs the script".
- **The live case does not run in CI.** CI's code-quality job does not
  build the site, so the case skips there with its reason. It runs wherever
  `site/dist` exists, and the e2e self-check runs in CI's site-e2e job on
  every push.
- **Task 4 landed after you reviewed this report**, at your decision to fix
  on the PR's branch rather than re-ship. It is the one change to read
  again: `preview_stop` and the new test case.
- **The zombie check is a judgment call** made while verifying Task 4: in a
  container whose PID 1 never reaps, the stopped orphan stayed a zombie that
  `kill -0` called alive, so every stop waited 5 s and sent SIGKILL. A real
  runner reaps; the check makes the stop right in either.
- **The Linux evidence is from a container**, `node:24-bookworm` under
  podman, not CI's Ubuntu runner. The PR's next `site-e2e` run is the
  measurement on the runner itself.
- **`lsof` on CI's Ubuntu runner** is present: the first CI run's
  code-quality job passed the held-port and holder cases, and `site-e2e`'s
  failure named the leftover's PID through it.
- **Windows:** Astro never auto-backgrounds there, and the script is
  bash-only. No change.

## Traceability

- `827468a` docs(plans): plan guide-parity M006/S01, the preview server is the script's own — Slice: M006/S01
- `37513a3` test(scripts): the preview server the e2e gate starts is its own — Slice: M006/S01; Rows: GP15
- `bac18fe` fix(scripts): the e2e gate refuses a held port and leaves nothing listening — Slice: M006/S01; Rows: GP15
- `6afe0e1` docs: a backgrounded server must be the process you stop — Slice: M006/S01; Rows: GP15
- `50d8d6a` docs(plans): write the guide-parity M006 report — Slice: M006/S01
- `53e0566` docs(plans): close guide-parity M006 in the ledger and record it in the changelog — Slice: M006/S01
- `c3954e8` docs(plans): add M006/S01 Task 4 — preview_stop stops the whole tree — Slice: M006/S01; Rows: GP15
- `980c307` fix(scripts): preview_stop stops every process it started — Slice: M006/S01; Rows: GP15

This report's update carries `Slice: M006/S01`.

Untraced commits: none.

## Decisions

Verbatim from `M006-decisions.md`, with headings demoted one level.

### 2026-10-20 — planning M006/S01

- **Finding:** the ledger row, written from central-africa M006's report,
  says the script's `cleanup` kills the subshell rather than the server. That
  is not the cause. Measured 2026-10-20 against `site/dist/` on port 4399:
  started the way the script starts it, `astro preview` (Astro 7.3.8) was
  listening with parent PID 1 in its own process group, the wrapper had
  already exited, and its log read "Preview server running … Stop: astro
  preview stop". `node_modules/astro/dist/cli/preview/index.js` backgrounds
  the server, adds `--json` and writes a lock file whenever `isRunByAgent()`
  is true (the `am-i-vibing` package; `CLAUDECODE` and `AI_AGENT` were set in
  this session), except on Windows or with `--ignore-lock`. Started with
  `--ignore-lock`, or with those variables unset, the server was the
  wrapper's child and the script's existing `kill` of the wrapper stopped it.
  So the leak happens only when an agent runs the script, which is why
  nobody saw it by hand or in CI. `astro dev` has the same logic
  (`cli/dev/index.js`, `ASTRO_DEV_BACKGROUND`).
- **Q:** Approve this design for M006/S01? (1) New
  `scripts/lib/preview-server.sh` with `preview_require_free_port` (refuses a
  held port, naming PID and command via lsof), `preview_start` and
  `preview_stop`. (2) `site-verify-local.sh` uses them, starts astro with
  `--ignore-lock` so it stays in the foreground under an agent, and after
  cleanup fails the run if anything is still on the port. (3)
  `scripts/lib/preview-server.test.mjs`, wired into `check-guards.sh`. —
  **A:** approve
- **Q:** GP15's verification says a test "runs the script and asserts nothing
  is listening afterwards". The script takes minutes (WASM and Astro build).
  Replace that with a live test that starts and stops a real astro preview
  through the lib under agent environment variables (skipped with a stated
  reason when `site/dist` is absent, as in CI's guards job), plus the e2e
  script checking its own port after every run? — **A:** yes, replace it
- **Q:** The row blames the cleanup for killing the wrapper rather than the
  server; the measured cause is Astro detaching itself under an agent. How
  should the ledger record that? — **A:** correct the row's Item to the
  measured cause and add a dated "corrected at planning" paragraph to the
  milestone
- **Q:** Should the gotcha also go into `CLAUDE.md`'s Key Conventions? —
  **A:** yes
- **Decision:** classified **bounded** — **Why:** it changes one existing
  script and adds a sourceable helper beside the one `scripts/lib/` already
  has; no interface outside `scripts/` changes.
- **Decision:** the live test forces agent detection by setting
  `CLAUDECODE=1` in the child's environment rather than relying on the
  caller's — **Why:** the regression only shows under an agent, so a test
  that inherits a human's environment would pass with `--ignore-lock`
  removed.
- **Decision:** the new test also runs in CI's code-quality job, beside the
  other `node --test scripts/…` contract steps — **Why:** `check-guards.sh`
  mirrors those steps, and a guard only `check-guards.sh` runs is the gap
  M007 closed.

### 2026-10-20 — executing M006/S01

- **Finding, correcting the planning finding above:** "with `--ignore-lock`,
  or with those variables unset, the server was the wrapper's child and the
  script's existing `kill` of the wrapper stopped it" is wrong for the
  script. That measurement ran in zsh, which execs the last command of a
  subshell; the script runs under `/bin/bash` 3.2, which forks it. Measured
  under bash with `--ignore-lock`: the PID held is a bash subshell, its child
  is `npm exec`, and that child's is the server; killing the subshell left
  the server listening. With `exec` in the subshell, the PID held is
  `npm exec`, which forwards SIGTERM, and the server stopped. So there are two
  causes, and the original row's "kills that subshell's PID, not the server"
  was right about the first: it leaks for everyone who runs the script, and
  under an agent Astro's detaching would defeat even the right PID. The
  helper uses both `exec` and `--ignore-lock`, and the ledger's correction
  paragraph and GP15 now say so. Caught by the live test, which went red with
  `--ignore-lock` alone.
- **Judgment call:** `preview_start` runs `exec npx astro preview …` in its
  subshell, beyond the approved design's `--ignore-lock` — **Why:** without
  it the approved design does not meet the slice's definition of done
  (measured above); it changes one word inside a function the design already
  named, and both mutations (no `exec`; no `--ignore-lock`) turn the live
  case red.
- **Judgment call:** the plan's Background and Task 1 step 3 are corrected
  for `exec` while Task 1 is in flight — **Why:** a plan left wrong misleads
  whoever reads it next; the evidence is the record of what ran.
- **Judgment call:** `site-verify-local.sh` traps `cleanup` on EXIT only,
  with INT and TERM traps that `exit 130` and `exit 143`, rather than
  `trap cleanup EXIT INT TERM` — **Why:** Task 2's `cleanup` exits with the
  `$?` it was entered with, and on an interrupt that is whatever the
  interrupted command last returned, so an interrupted run could have exited
  0; the slice's definition of done names the interrupted case.

### 2026-10-20 — shipping M006, CI failure

- **Finding:** PR #389's `site-e2e` passed every gate, then failed the new
  cleanup check: `node …/astro preview --port 4322 --ignore-lock` was still
  listening. Reproduced under Debian (`node:24-bookworm`): npm runs the bin
  as `sh -c 'astro' preview …`; dash forks `astro` (macOS's `sh`, bash 3.2,
  execs it), so the SIGTERM `npm exec` forwards reaches only that `sh`, and
  the server is orphaned to PID 1 and still listening 3 s later. A third
  cause, invisible on macOS.
- **Q:** Fix on this branch as a new task in M006/S01 and push to #389, close
  #389 and re-ship, or look first? — **A:** fix on this branch
- **Decision:** `preview_stop` signals `PREVIEW_PID` and every descendant,
  collected before the first signal, rather than killing whatever holds the
  port — **Why:** it stops what the script started and nothing else; the
  port check after cleanup stays as the backstop.
- **Decision:** the slice and milestone go back to `in-progress` for Task 4
  and return to `done` in its commit — **Why:** the ledger must not say done
  while a plan box is open.
- **Judgment call:** `preview_stop` counts a zombie as stopped — **Why:**
  measured under Linux, an orphan in a container whose PID 1 never reaps
  stays a zombie that `kill -0` reports alive, so every stop waited its full
  5 s and sent SIGKILL to a dead process; a zombie holds no port.
- **Judgment call:** `CLAUDE.md`'s convention gains the Linux cause as a
  third bullet — **Why:** it is the same gotcha's third form, and the one no
  macOS measurement can show.
