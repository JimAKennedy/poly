# M006 — A dead reference is found by a check

**Review-gate report.** Generated from the ledger, `git log` and
`M006-decisions.md`. `/jk:ship` is the next step and it is the owner's to take.

**Vision:** Link rot is reported by a scheduled job rather than discovered by a
reader.

**Branch:** `milestone/M006-liveness` · **Ledger:** `docs/plans/verifiable-references/ledger.md`

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M006/S01 | The scheduled check reports | VR16 | done |

## Definition of done

- [x] A scheduled job checks every URL in the bibliography and reports what it
      found, without failing a pull request
- [x] It does not report the browser-only class as dead: 403 and 406 are
      distinguished from 404 and no-response
- [x] It is shown to detect a genuinely dead URL, by introducing one

## The headline

**Every URL the bibliographies print is now checked weekly, and a bot
refusal is never mistaken for a dead link.** `scripts/reference-links.mjs`
classifies each URL as ok, blocked, dead or error;
`.github/workflows/reference-links.yml` runs it every Monday, on demand with a
URL to inject, and on PRs that touch what it checks. It is advisory: it exits
0 whatever it finds, and only scheduled and manual runs keep one "Dead
references" issue current.

**The first real run found a genuinely dead reference.** Of 58 URLs: 37 ok,
18 blocked (Academia.edu, ResearchGate, Cambridge, Oxford Academic and twelve
DOIs to bot-guarded publishers — all 403, none called dead), 1 to check by
hand (Grove's redirect loop behind `fr-powers-1980`), and 2 dead: the URL
injected to prove detection, and **`fr-silverman-2007`'s DOI**, whose resolver
target never accepts a connection.

## Validation

| Token | Result |
|---|---|
| `format` | exit 0 |
| `guards` | exit 0, 22 guard invocations (the workflow contract and the classifier tests among them) |
| classifier tests | 18 pass, against a local HTTP server |
| workflow contract | 4 pass; its fixture breaks every rule |
| `npm --prefix site test` | 398 pass, including guard reachability |
| real local run | 58 URLs; the injected URL dead (404) |

## Traceability

Every commit on the branch carries `Plan:` and `Slice:` lines.

- `e0337e1` M006 planning — a dead reference is found by a check: decisions and plan — Slice M006/S01
- `df568a6` M006/S01 task 1 — the checker classifies a URL the way the decisions say — Slice M006/S01
- `8db91a2` M006/S01 task 2 — the checker reads both bibliographies and catches a dead URL — Slice M006/S01
- `dd006a0` M006/S01 task 3 — the workflow runs it weekly and reports, and a contract locks its shape — Slice M006/S01, Rows VR16

**Untraced commits:** none.

## What a reviewer should look at twice

- **The workflow has not yet run in CI.** Its first run is this milestone's
  PR (the `pull_request` trigger), report-only; the scheduled run and the
  issue path run first after merge. A manual run with `extra_url` set to a
  dead URL re-proves detection in CI.
- **One real finding to act on:** `fr-silverman-2007`'s DOI is dead. The
  owner saved the free issue from `muzikologija-musicology.com` in M003; the
  citation could point there. Not changed here — correcting a citation is
  outside this slice — and the first scheduled run will open the issue for it.
- **The checker's first real run corrected it.** undici reports a connect
  timeout as a `TypeError`, which had been filed as "check by hand"; it now
  counts as the no-response it is (decisions file).
- **An honest User-Agent costs some `blocked` results** that a browser-like
  one might avoid; blocked is the correct class for a host that refuses an
  honest agent, and it is never reported as dead.

## Decisions

## 2026-10-05 — planning M006/S01

Measured before asking, on `main` at `bfecf98`. Nothing in the repository
fetches a bibliography URL on a schedule. `scripts/fetch-references.mjs`
(M003) already fetches and recognises bot challenges (`isChallengePage`), and
`site/src/data/references-bibliography.mjs` parses both bibliographies. The
sanitizer nightly (`.github/workflows/sanitizers.yml`) already opens or
updates one issue on failure, deduplicated, from a job that alone holds
`issues: write`. `scripts/check-workflow-hygiene.mjs` requires every workflow
to declare top-level permissions, to cancel superseded PR runs in a
ref-scoped concurrency group, and to pin third-party actions by SHA.

- **Q:** Where should the job report? — **A:** a job summary and artifact on
  every run, plus one deduplicated "Dead references" issue.
- **Q:** How often? — **A:** weekly.
- **Q:** Which URLs? — **A:** both bibliographies.
- **Q:** How is detection of a dead URL proved? — **A:** classifier tests
  against a local HTTP server, a real local run with a dead URL injected,
  and a report-only run on PRs that touch the bibliographies or the checker.

### Taken on the owner's behalf

- **Classes.** `ok` — 2xx after redirects. `blocked` — 401, 403, 406, 429, or
  a 200 that is a bot challenge (`isChallengePage`): reachable by a person,
  never reported as dead. `dead` — 404, 410, DNS failure, refused
  connection, or no response within the timeout, retried once before it
  counts. `error` — any other status, 5xx included: reported separately as
  "check by hand", not as dead, because a server error is usually transient.
- **Every URL in an entry is checked**, not only its first route: the
  bibliography promises each link it prints. URLs are deduplicated across
  the two files, and each finding names every anchor that carries the URL.
- **The checker never fails on findings.** It exits 0 whatever it finds and
  2 only if it cannot run (a bibliography missing or unparsable), so the
  workflow's own status means "the check ran", never "a link is dead".
- **Polite fetching.** An honest User-Agent naming the repository, four
  requests at a time, a 20-second timeout, GET with the body discarded (many
  hosts mishandle HEAD). A host that blocks an honest agent is reported as
  `blocked`, which is the correct class for it.
- **Issues only from scheduled and manual runs.** The PR-triggered run writes
  the summary and artifact and touches no issue, so a PR never opens one.
  The issue is closed with a comment when a later run finds nothing dead.
- **Schedule:** Mondays 04:00 UTC, plus `workflow_dispatch` with an optional
  `extra_url` input for re-proving detection after merge.
- **CI evidence of a real run lands at ship.** Before the PR exists the
  workflow cannot run in CI; the slice proves detection by the tests and a
  real local run, proves the workflow's shape by a contract test, and the PR
  run is recorded in the PR and the report at `/jk:ship`.

### Deferred

- None.

## 2026-10-05 — judgment calls during M006/S01 task 1

- **The two README entries landed in task 1, not task 2.** The plan put
  `scripts/README.md` in task 2, but `check-scripts-readme` (in `guards`)
  fails on any script without an entry, so task 1 could not commit green
  without them. Obviously right: the guard requires the entry in the same
  change that adds the script.

## 2026-10-05 — judgment calls during M006/S01 task 2

- **undici's own timeouts count as no-response.** The first real run filed
  Silverman's DOI, which redirects to `doiserbia.nb.rs` and never gets a
  connection, as "check by hand": undici reports a connect timeout as a
  `TypeError` with `UND_ERR_CONNECT_TIMEOUT`, not as an abort. Connect,
  headers and body timeouts now retry once and then count as `dead`, which is
  the decisions file's "no response within the timeout". Obviously right: the
  class was already decided; the code missed one way of saying it.
- **A redirect loop stays "check by hand", and says so.** Powers's Grove DOI
  loops ("redirect count exceeded"), typical of a cookie wall but also of a
  broken site, so it is not called blocked or dead; the report now prints the
  cause instead of "TypeError".
