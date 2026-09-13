---
class: gated
---

# M006 — Review report

Status: current (2026-09-13)

Generated from `docs/plans/engine-capability/ledger.md`, git, and
`M006-decisions.md` for the review that precedes `/jk:ship`.

**Vision:** A developer can run every check CI will run, and every test in the
tree runs somewhere in CI — so a green local gate means something, and a test
file cannot be proven only on the machine that wrote it.

**Branch:** `milestone/M006-gate-parity`, cut from `main` at `0e62d14`.

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M006/S01 | `doc-drift` is runnable locally | GAP01 | done |
| M006/S02 | Every site test runs in CI | GAP02 | done |

## Definition of done

**M006/S01**

- [x] A developer can run the `doc-drift` check against the default branch with
      a documented command, without knowing to set an environment variable by
      hand
- [x] The `doc-discipline` token no longer reports success while silently
      skipping a check CI enforces
- [x] A run genuinely unable to determine a base still explains why rather than
      failing

**M006/S02**

- [x] Every `site/tests/*.test.mjs` file runs in at least one CI job
- [x] A test file added to that directory cannot silently go unrun
- [x] #272's counts are corrected to what the tree holds, or the issue is closed
      by this work

## Validation

Run on the current head, not when each slice landed.

| Token | Command | Result |
|---|---|---|
| `format` | `pre-commit run --all-files` | pass |
| `site-unit` | `npm --prefix site test` | pass, 278 tests |
| `doc-conformance` | `bash scripts/check-doc-conformance.sh` | pass, 262 tests (20 files, up from 19) |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` | pass, doc-drift reporting |

## Traceability

Every commit carries a `Slice:` trailer. **No untraced commits.**

| Commit | Subject | Slice | Rows |
|---|---|---|---|
| `af51b2d` | docs(plans): plan M006's two slices and record its decisions | M006/S01, M006/S02 | — |
| `62838de` | feat(scripts): run doc-drift locally, and refuse to pass if it did not run | M006/S01 | — |
| `5a07aa8` | fix(scripts): drop the doc-drift assertion, which could not fire and was wrong | M006/S01 | — |
| `36f083c` | docs: name the doc-gate command in CLAUDE.md, and close M006/S01 | M006/S01 | GAP01 |
| `7e12a0e` | ci(site-lint): run the whole site test suite | M006/S02 | — |
| `12161ce` | test(site): guard that CI runs the whole site suite, closing M006/S02 | M006/S02 | GAP02 |
| `79659da` | fix(scripts): run the wiring test from the runner it guards | M006/S02 | GAP02 |

Two of the seven are corrections to earlier commits on this same branch —
`5a07aa8` and `79659da`. They are the milestone's real content and are described
below rather than buried.

## What a reviewer should look at twice

1. **I built S01 on a measurement I took wrongly, and shipped it before catching
   it.** `62838de` claimed `jk-standards all --base <unresolvable>` prints
   nothing and exits 0, and justified a "prove it ran" assertion from that. It
   exits **2**. The bad reading came from `… | tail -3; echo $?`, which reports
   `tail`'s status. Measured directly: no base → exit 0 and a `skipped` line
   (the real bug); unresolvable base → exit 2, loud; good base → exit 0 and
   doc-drift reports.
2. **That correction exposed a worse defect in my own code.** The assertion
   matched any line beginning `doc-drift` — and the *skip* message begins
   `doc-drift`. So it would have passed on a skip: the thing it claimed to
   prevent. It also could never fire. An unreachable check that would be wrong
   if reached, inside the milestone about checks that pass without running.
   Removed in `5a07aa8`.
3. **A `git reset --hard` destroyed the fix and I proved the old code.** Proof 1
   ends by dropping a scratch commit; that discarded the uncommitted rewrite
   with it, so all three proofs ran against the code they were meant to replace.
   It surfaced only because proof 3 printed the old wording. The correction was
   then committed *before* re-running.
4. **The guard I added in S02 was circular.** It asserts CI runs the whole site
   suite, but was executed only by the CI step it guards — delete the step and
   the guard leaves CI with it, silently. `79659da` puts it in the runner's
   `TESTS` array, and the fix is proved: with the step deleted,
   `doc-conformance` now fails with the guard's message.
5. **The unprotected-file count in the GAP02 row was wrong when committed.** 7
   of 23, not 6. The 6 came from a `grep` over the runner that matched a
   *comment* naming `doc-conformance-wiring.test.mjs` — which is itself the
   seventh file. #272's figure of 7 was right; only its total of 21 had drifted.
   Corrected in `79659da`; the pull request should correct #272's total rather
   than its count.
6. **CI still calls `jk-standards all` directly**, not the new wrapper, and that
   is deliberate: CI already supplies `GITHUB_BASE_REF`, so doc-drift runs
   there. Repointing a working CI job would be change without behavioural gain.
   Parity is in effect, not in the literal command.

## Decisions

Verbatim from `M006-decisions.md`.

## 2026-09-13 — planning M006

- **Decision:** Both slices classified **bounded**; no design document.
  — **Why:** S01 adds one wrapper script and repoints one token. S02 adds one
  step to an existing CI job and one assertion. Neither introduces a subsystem.

- **Q:** Passing `--base` is not enough — `doc-drift` prints nothing and exits 0
  on an unresolvable base, so a naive token would silently skip exactly as it
  does today. What shape should the fix take? — **A:** A wrapper that proves the
  check ran.
- **Decision:** `scripts/check-doc-discipline.sh` determines the base, runs
  `jk-standards all --base`, and then asserts a `doc-drift` line actually
  appeared in the output, failing loudly if it did not. The `doc-discipline`
  token points at it. — **Why:** This was measured, not assumed:
  `jk-standards all --base origin/nonexistent-ref` printed no `doc-drift` line
  at all and exited 0. A token reading `jk-standards all --base origin/main`
  would therefore pass while the check never ran, on any clone where that ref is
  not fetched — the present bug with a different trigger. Asserting the check
  *ran* is the only arm that cannot reproduce it.
- **Decision:** When no base can be determined at all, the wrapper explains and
  exits 0 rather than failing. — **Why:** The slice's definition of done
  requires it: a detached HEAD or a shallow clone should not be made unusable.
  The distinction that matters is between *explaining* and *silently skipping* —
  the present behaviour prints a skip line that reads like a pass in a wall of
  green, and the wrapper's own failure arm covers the case where a base exists
  and the check still did not run.

- **Q:** Where should the site suite run in CI? — **A:** Extend the `site-lint`
  job.
- **Decision:** `npm --prefix site test` becomes a step in `site-lint` rather
  than a new `site-unit` job. — **Why:** That job already runs `npm ci` in
  `site/` and already executes a test suite through
  `scripts/check-doc-conformance.sh`, so the deps are installed and no
  aggregation-gate wiring changes. A new job would cost a second runner doing
  its own checkout and install for a suite that takes under a second. The job's
  name understates what it does, but it did before this change too.
- **Decision:** No bespoke "no orphaned test" guard is written. — **Why:** The
  site test script is `node --test tests/**/*.test.mjs`, a directory glob, so
  once CI runs it every file in `site/tests` runs by construction and a new file
  is picked up automatically. The definition-of-done item about a file not
  silently going unrun is satisfied structurally rather than by a check that
  would itself need proving. What *is* worth guarding is the CI step's continued
  existence, which S02's second task covers.

## 2026-09-13 — executing M006/S02 task 2 (in-flight correction)

- **Decision:** `site/tests/doc-conformance-wiring.test.mjs` added to
  `check-doc-conformance.sh`'s `TESTS` array. — **Why:** The guard asserting CI
  runs the whole site suite was executed only by the CI step it guards, so
  deleting that step would have removed the guard from CI too and nothing would
  have failed. It now runs via `doc-conformance` independently. Resolved in
  flight because a circular guard is not a guard, and this milestone is about
  exactly that failure mode.
- **Correction:** the unprotected-file count is 7 of 23, not the 6 reported
  earlier in the run and written into the GAP02 row. The 6 came from a `grep`
  over the whole runner script, which matched a *comment* naming
  `doc-conformance-wiring.test.mjs`; measured against the `TESTS` array it is 7,
  and the seventh is that file itself. #272's figure of 7 was right all along —
  only its total of 21 had drifted to 23. Row and evidence corrected.
