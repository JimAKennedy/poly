---
class: gated
---

# M006/S01 — The preview server is the script's own

**Slice:** M006/S01 in `docs/plans/guide-parity/ledger.md` (row GP15)
**Decisions:** `M006-decisions.md`, 2026-10-20 — design approved, verification
replaced, row corrected, `CLAUDE.md` gotcha approved

## Task status

- [ ] Task 1 — `scripts/lib/preview-server.sh` and its test
- [ ] Task 2 — `site-verify-local.sh` uses the helper and checks its own port
- [ ] Task 3 — the gotcha in `CLAUDE.md`, and the gate run under an agent

## Definition of Done

- [ ] The script refuses to start when its preview port is already in use,
      naming the process that holds it
- [ ] No preview server is left listening after the script exits, whether it
      passes, fails or is interrupted
- [ ] A test fails if either regresses

## Validation

| Token | Command |
|---|---|
| format | `pre-commit run --all-files` |
| e2e | `bash scripts/site-verify-local.sh` |
| guards | `bash scripts/check-guards.sh` |

`e2e` rewrites `webui/poly_engine.{js,wasm}`; restore both with
`git checkout HEAD -- webui/poly_engine.js webui/poly_engine.wasm` after every
run. This slice ships no engine change.

## Background

Astro 7.3.8 starts `astro preview` as a detached background server, with a
lock file, whenever it detects an AI agent (`isRunByAgent()` in
`site/node_modules/astro/dist/cli/agent.js`, which reads variables such as
`CLAUDECODE` and `AI_AGENT`), unless `--ignore-lock` is passed or the platform
is Windows. `scripts/site-verify-local.sh` tracks the PID of the subshell that
ran `npx astro preview`; under an agent that subshell exits at once, so
`cleanup` has nothing to kill and the server stays on port 4322. With
`--ignore-lock` the server is the subshell's child, and killing the subshell
stops it (measured 2026-10-20, `M006-decisions.md`). `--ignore-lock` also
skips Astro's own one-server check, so the script must check the port itself:
otherwise Vite may move to the next port while the readiness `curl` on 4322
answers from someone else's server.

## Task 1 — `scripts/lib/preview-server.sh` and its test

**Creates:** `scripts/lib/preview-server.sh`, `scripts/lib/preview-server.test.mjs`
**Modifies:** `scripts/check-guards.sh`, `.github/workflows/ci.yml`, `scripts/README.md`
**Consumes:** nothing. **Produces:** the four functions Task 2 calls.

The helper is sourced, never executed, and defines:

- `preview_port_holder <port>` — prints `<pid> <command>` for the process
  listening on TCP `<port>` (`lsof -nP -iTCP:<port> -sTCP:LISTEN -t`, then
  `ps -o command= -p <pid>`), or nothing when the port is free. Without
  `lsof`, falls back to `curl -s -o /dev/null http://localhost:<port>/` and
  prints `unknown (lsof not found)` if anything answers.
- `preview_require_free_port <port>` — returns 0 when the port is free;
  otherwise prints `=== FAIL: port <port> is already in use by <holder> ===`
  to stderr, plus `stop it, or set POLY_PREVIEW_PORT to another port`, and
  returns 1.
- `preview_start <site_dir> <port> <log>` — runs
  `(cd "<site_dir>" && npx astro preview --port "<port>" --ignore-lock >"<log>" 2>&1) &`
  and sets `PREVIEW_PID` to `$!`. A comment above it says why `--ignore-lock`
  is there, citing Astro's agent auto-backgrounding.
- `preview_stop` — the body of today's `cleanup` minus the log removal: kill
  `PREVIEW_PID`, wait up to a second, `kill -9`, reap.

Steps:

1. Write `scripts/lib/preview-server.test.mjs` (`node:test`, an SPDX/header
   comment in the style of `scripts/check-docs-index.mjs`, run line
   `node --test scripts/lib/preview-server.test.mjs`). Each case runs
   `bash -c 'source scripts/lib/preview-server.sh; …'` with `spawnSync`.
   Cases:
   - **free port:** pick a free port (listen on 0 with `net`, read it, close),
     `preview_require_free_port <port>` exits 0 and prints nothing.
   - **held port:** a `net` server listens on a free port in the test
     process; `preview_require_free_port` exits 1, and stderr contains
     `already in use` and `String(process.pid)`.
   - **holder printed:** `preview_port_holder` on the held port prints a line
     starting with the test's PID.
   - **live lifecycle, under an agent:** skipped with the reason
     `site/dist/index.html absent — run the e2e gate or npm --prefix site run build first`
     when that file does not exist. Otherwise: pick a free port; run
     `source …; preview_start site <port> <tmplog>; for i in $(seq 1 60); do curl -sf -o /dev/null http://localhost:<port>/ && break; sleep 0.5; done; curl -sf -o /dev/null http://localhost:<port>/ || exit 3; preview_stop`
     with `env: { ...process.env, CLAUDECODE: '1' }`; assert exit 0 (it
     served); then, after 1 s, assert `preview_port_holder <port>` prints
     nothing. On failure, stop any leftover with `kill` on the holder's PID
     so a red run leaves nothing behind.
2. Run `node --test scripts/lib/preview-server.test.mjs`. Expected: every
   case fails, because the helper does not exist.
3. Write `scripts/lib/preview-server.sh` with a header in the style of
   `scripts/lib/ensure-emsdk.sh` (purpose, usage, the Astro background
   behaviour, a pointer to `M006-decisions.md`).
4. Run the test again. Expected: all pass, the live case included (this
   checkout has `site/dist/` from earlier `e2e` runs; if not, run
   `npm --prefix site run build` first so the live case is not skipped).
5. Mutation: remove `--ignore-lock` from `preview_start`, re-run. Expected:
   the live case fails on a leftover holder. Record the failure line in the
   evidence, restore the flag, and confirm the leftover was stopped
   (`lsof -nP -iTCP:<port> -sTCP:LISTEN` empty).
6. Wire it in:
   - `scripts/check-guards.sh`: `run_guard "preview-server helper" node --test scripts/lib/preview-server.test.mjs`
     with a comment naming guide-parity M006/S01 (GP15).
   - `.github/workflows/ci.yml` code-quality job, after the reference-links
     step: a step named `Preview-server helper refuses a held port and leaves nothing listening`
     running the same command.
   - `scripts/README.md`'s `lib/` entry names `lib/preview-server.sh`.
7. Run `bash scripts/check-guards.sh` (expected: exit 0, the new guard
   listed) and `pre-commit run --all-files` (exit 0).
8. Commit: `test(scripts): the preview server the e2e gate starts is its own`
   with `Plan: docs/plans/guide-parity/ledger.md`, `Slice: M006/S01`,
   `Rows: GP15`; tick Task 1 above in the same commit; append to
   `evidence/M006-S01.md`.

**Check:** `guards`.

## Task 2 — `site-verify-local.sh` uses the helper and checks its own port

**Modifies:** `scripts/site-verify-local.sh`, `scripts/lib/preview-server.test.mjs`
**Consumes:** Task 1's four functions. **Produces:** the script Task 3 runs.

Steps:

1. Add a **script contract** case to the test: read
   `scripts/site-verify-local.sh` and assert that it
   - sources `lib/preview-server.sh`;
   - calls `preview_require_free_port` before `preview_start`, and both
     before the first `npx playwright`;
   - contains no `npx astro preview` of its own (every start goes through the
     helper, so `--ignore-lock` cannot be dropped in one place only);
   - calls `preview_stop` in its `cleanup`, then
     `preview_port_holder`, and sets a non-zero exit when the holder is
     non-empty.
2. Run the test. Expected: the contract case fails on the current script;
   the rest pass.
3. Edit `scripts/site-verify-local.sh`:
   - `source "${SCRIPT_DIR}/lib/preview-server.sh"` beside the
     `ensure-emsdk.sh` source line;
   - step `[4/5]`: `preview_require_free_port "${PREVIEW_PORT}" || exit 1`,
     then `preview_start "${SITE_DIR}" "${PREVIEW_PORT}" "${PREVIEW_LOG}"`;
   - `cleanup`: capture `$?` first as `rc`, call `preview_stop`, then
     `holder="$(preview_port_holder "${PREVIEW_PORT}")"`; when non-empty,
     print `=== FAIL: a preview server is still listening on :<port> after cleanup: <holder> ===`
     to stderr and set `rc=1`; remove the log; `exit "${rc}"`;
   - the header's step 5 and step 8 say the port is checked before and after.
4. Run the test (expected: all pass), `bash -n scripts/site-verify-local.sh`
   (exit 0), `bash scripts/check-guards.sh` and `pre-commit run --all-files`
   (both exit 0).
5. Commit: `fix(scripts): the e2e gate refuses a held port and leaves nothing listening`
   with the slice's trailers and `Rows: GP15`; tick Task 2; append evidence.

**Check:** `guards`, `format`.

## Task 3 — the gotcha in `CLAUDE.md`, and the gate run under an agent

**Modifies:** `CLAUDE.md`
**Consumes:** Task 2's script.

Steps:

1. Add a Key Conventions subsection, `### Astro Backgrounds Its Servers Under an Agent`,
   after "The e2e Gate Rewrites Committed Artifacts": Astro 7.3.8 detaches
   `astro preview` and `astro dev` into a locked background server when it
   detects an AI agent (`CLAUDECODE`, `AI_AGENT`), so a script that tracks
   the PID it started has nothing to stop; scripts pass `--ignore-lock`,
   which keeps the server in the foreground and skips Astro's one-server
   lock, so the script must check the port itself, as
   `scripts/lib/preview-server.sh` does. Name the symptom: a server left on
   4322 after `e2e`, and `astro preview stop` as the manual cleanup.
2. Held-port refusal, end to end: start a listener on 4322
   (`node -e "require('net').createServer().listen(4322)" &`), run
   `bash scripts/site-verify-local.sh`, and expect it to exit 1 at step
   `[4/5]` with the holder's PID in the message, after building; stop the
   listener. Record the message in the evidence.
3. Run `bash scripts/site-verify-local.sh` from this session, so under an
   agent. Expected: exit 0, every gate PASS, and afterwards
   `lsof -nP -iTCP:4322 -sTCP:LISTEN` prints nothing. Restore the WASM
   artifacts.
4. Interrupted run: start the script in the background, wait until its log
   prints `preview ready`, send it SIGINT, and expect it to exit non-zero
   with nothing listening on 4322 afterwards. Restore the WASM artifacts.
5. Run `pre-commit run --all-files`, `bash scripts/check-guards.sh` and
   `bash scripts/check-doc-discipline.sh` (all exit 0; the last because
   `CLAUDE.md` changed).
6. Commit: `docs: Astro backgrounds its servers when an agent runs it` with
   the slice's trailers and `Rows: GP15`; tick Task 3; append evidence; set
   GP15 and the DoD boxes in the ledger, and the slice to `done`.

**Check:** `e2e`, `format`, `guards`.

## Self-review

- **DoD 1** (refuses a held port, names the holder): Task 1's
  `preview_require_free_port` and its held-port case; Task 2 wires it before
  the start; Task 3 step 2 proves it end to end.
- **DoD 2** (nothing left after pass, fail or interrupt): `--ignore-lock` in
  Task 1 and the live case; Task 2's `cleanup`, which runs on `EXIT INT TERM`
  and fails on a leftover; Task 3 steps 2–4 cover fail, pass and interrupt.
- **DoD 3** (a test fails on regression): the live case fails with
  `--ignore-lock` removed (Task 1 step 5); the contract case fails if the
  script bypasses the helper or drops the checks (Task 2 step 2); both run in
  `guards` and CI.
- **GP15's Verification** (as amended 2026-10-20): the held-port case and the
  live lifecycle case under agent variables are Task 1; the e2e self-check is
  Task 2; each went red first (Task 1 steps 2 and 5, Task 2 step 2).
- Placeholders: none. Names: `preview_port_holder`,
  `preview_require_free_port`, `preview_start`, `preview_stop`, `PREVIEW_PID`
  are used the same way in every task.
