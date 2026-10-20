---
class: gated
---

# M006 — decisions

Append-only. Every question asked while planning or executing guide-parity
M006, its answer, and every choice made on the owner's behalf.

## 2026-10-20 — planning M006/S01

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

## 2026-10-20 — executing M006/S01

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

## 2026-10-20 — shipping M006, CI failure

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
