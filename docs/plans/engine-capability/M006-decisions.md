---
class: gated
---

# M006 — Decisions

Append-only. One entry per planning session or in-flight judgment call, so the
milestone's review can see what shaped it without reconstructing it from diffs.

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
