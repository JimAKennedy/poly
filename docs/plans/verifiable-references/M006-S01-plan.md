# M006/S01 — The scheduled check reports

**Slice:** M006/S01 in `docs/plans/verifiable-references/ledger.md`
**Rows:** VR16 (nothing fetches anything; a Tier-A source could rot to a 404
with every gate green)
**Classification:** bounded. A script beside `fetch-references.mjs` reusing
its challenge detector and the bibliography parser, one workflow in the
repository's existing shape (the sanitizer nightly's deduplicated issue,
`check-workflow-hygiene.mjs`'s rules), and a contract test in the
`check-release-workflow.mjs` style. Decisions are in `M006-decisions.md`.

## Task status

- [x] 1. The checker classifies a URL the way the decisions say
- [x] 2. The checker reads both bibliographies and catches a dead URL in a real run
- [ ] 3. The workflow runs it weekly and reports, and a contract locks its shape

## Definition of Done

Copied verbatim from the slice:

- [ ] A scheduled job checks every URL in the bibliography and reports what it
      found, without failing a pull request
- [ ] It does not report the browser-only class as dead: 403 and 406 are
      distinguished from 404 and no-response
- [ ] It is shown to detect a genuinely dead URL, by introducing one

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `guards` | `bash scripts/check-guards.sh` |

`guards` runs the new unit tests and the workflow contract once task 3 wires
them in, plus `check-scripts-readme` and `check-workflow-hygiene` over the new
files. `site/tests/doc-conformance-wiring.test.mjs` (in `site-unit`) checks the
new guard is reachable, so `npm --prefix site test` is run as well.

## Names used throughout

- `scripts/reference-links.mjs` — the checker. Not a `check-*` file: it is a
  tool the workflow runs, not a guard, and it exits 0 on findings.
- `classifyResponse({ status, body })` and `classifyError(err)` — task 1,
  exported, pure: return one of `ok`, `blocked`, `dead`, `error`.
- `checkUrl(url, { timeoutMs, fetchImpl })` — task 1: fetch, retry once on
  no response, classify.
- `collectUrls(bibliography)` — task 2: every `http(s)` link in every entry of
  both files, deduplicated, each with the anchors that carry it.
- `scripts/reference-links.test.mjs` — the unit tests (tasks 1–2).
- `.github/workflows/reference-links.yml` — the workflow (task 3).
- `scripts/check-reference-links-workflow.mjs` — its contract (task 3).

## Task 1 — The checker classifies a URL the way the decisions say

**Files:** create `scripts/reference-links.mjs`,
`scripts/reference-links.test.mjs`.

1. Write the tests first. Start a local `node:http` server in the test with
   routes answering 200 (an article), 200 (a body containing the Anubis
   challenge marker), 301→200, 401, 403, 404, 406, 410, 429, 500, and one
   route that never answers; and a closed port for refused connections.
   Assert `checkUrl` returns: `ok`, `blocked`, `ok`, `blocked`, `blocked`,
   `dead`, `blocked`, `dead`, `blocked`, `error`, `dead` (timeout, after one
   retry), `dead` (refused). Use a 500 ms timeout in the test.
2. Run `node --test scripts/reference-links.test.mjs`; it fails — the module
   does not exist.
3. Implement: `classifyResponse` maps 2xx to `ok` unless
   `isChallengePage(body)` (imported from `fetch-references.mjs`) then
   `blocked`; 401/403/406/429 → `blocked`; 404/410 → `dead`; anything else →
   `error`. `classifyError` maps an abort/timeout, `ENOTFOUND`,
   `ECONNREFUSED`, `ECONNRESET` and `EAI_AGAIN` to `dead`, anything else to
   `error`. `checkUrl` does a GET with the User-Agent
   `poly-reference-links (+https://github.com/JimAKennedy/poly)`, follows
   redirects, reads at most the first 64 KB of the body for the challenge
   check, and retries once before classifying a no-response as `dead`.
4. Run the tests: green.
5. Commit with `Rows:` empty.

## Task 2 — The checker reads both bibliographies and catches a dead URL in a real run

**Files:** `scripts/reference-links.mjs`, `scripts/reference-links.test.mjs`,
`scripts/README.md`; the evidence file.

1. Tests first: `collectUrls` over a fixture of three entry lines returns
   every link (two in one entry), deduplicated, with each URL's anchors;
   over the real bibliographies it returns more than zero URLs and every one
   starts with `http`. The CLI, run with no network
   (`--dry-run`), prints the URL count and exits 0; with an unreadable
   bibliography path (`POLY_BIBLIOGRAPHY_ROOT` pointed at an empty temp
   directory) exits 2. Run: red.
2. Implement the CLI: `node scripts/reference-links.mjs [--dry-run]
   [--extra-url <url>]... [--json <path>] [--summary <path>]`. It checks every
   collected URL four at a time; `--extra-url` adds URLs attributed to
   `(injected)`; `--json` writes `{ checked, counts, findings[] }`;
   `--summary` writes Markdown: a counts line, then a table per class other
   than `ok` (URL, status or error, anchors). It always exits 0 after a run.
3. Add the script and its test to `scripts/README.md`.
4. Run the tests: green.
5. **The real run.** Run
   `node scripts/reference-links.mjs --extra-url https://poly.jk.digital/this-reference-does-not-exist --json <tmp>/links.json --summary <tmp>/links.md`
   and confirm the injected URL is classified `dead` and that the browser-only
   hosts M003 recorded (Academia.edu, ResearchGate, `emusicology.org`) come out
   `blocked`, not `dead`. Record the counts line, the injected URL's row, and
   every real `dead` or `error` finding in the evidence — those are the first
   report, not failures of the slice.
6. Run `guards`, `format`. Commit with `Rows:` empty.

## Task 3 — The workflow runs it weekly and reports, and a contract locks its shape

**Files:** create `.github/workflows/reference-links.yml`,
`scripts/check-reference-links-workflow.mjs`; modify
`scripts/check-guards.sh`, `.github/workflows/ci.yml` (code-quality job),
`scripts/README.md`; the ledger; the evidence file.

1. Write the contract test first, `node --test`-style over the workflow's
   text, each rule asserted on an inline fixture that must fail and on the
   real file:
   - triggers: `schedule` with a weekly cron, `workflow_dispatch` with an
     `extra_url` input, and `pull_request` limited by `paths` to the two
     bibliographies, `scripts/reference-links.mjs` and the workflow itself;
   - top-level `permissions: contents: read`; `issues: write` appears only on
     the `report` job, and that job's `if:` excludes `pull_request`;
   - the check step runs `node scripts/reference-links.mjs` with `--json` and
     `--summary`, appends the summary to `$GITHUB_STEP_SUMMARY`, and uploads
     the JSON as an artifact;
   - no step can fail the job on findings: the checker's exit status is the
     only thing that can, and the contract asserts no `--strict` or
     `exit 1` on findings.
   Run it: red — the workflow does not exist.
2. Write `reference-links.yml`: `concurrency` ref-scoped with
   `cancel-in-progress` for PRs (the hygiene rule); job `check` on
   `ubuntu-latest` with Node 24, running the checker (passing
   `--extra-url` when the dispatch input is set) and uploading
   `reference-links.json`; job `report`, `needs: check`,
   `if: github.event_name != 'pull_request'`, `permissions: issues: write`,
   downloading the artifact and using `actions/github-script` at the SHA the
   sanitizer workflow pins: if any finding is `dead`, comment on the open
   issue labelled `dead-reference` or open one titled "Dead references"; if
   none is `dead` and such an issue is open, comment "all clear" with the run
   link and close it. Every `uses:` is SHA-pinned.
3. Wire `check-reference-links-workflow.mjs` and
   `scripts/reference-links.test.mjs` into `check-guards.sh` and the
   `code-quality` job in `ci.yml`, beside the release-workflow contract.
   Add the contract and the workflow to `scripts/README.md`.
4. Run the contract (green), `node --test scripts/check-workflow-hygiene.mjs`,
   `guards`, `format`, and `npm --prefix site test` (the reachability test).
5. Evidence; tick the DoD; set VR16 and the slice `done`; run
   `jk-standards ledger`. Commit with `Rows: VR16`.

## Self-review

- **DoD 1** (a scheduled job checks every URL and reports without failing a
  PR) — task 3's workflow and contract; "every URL" is task 2's
  `collectUrls`; "without failing a pull request" is the exit-0 rule and the
  report job's `if:`, both contract-asserted.
- **DoD 2** (403 and 406 distinguished from 404 and no-response) — task 1's
  tests, one route each, and task 2's real run naming the browser-only hosts.
- **DoD 3** (shown to detect a genuinely dead URL by introducing one) — task 2
  step 5, a real run with a dead URL injected; and, after merge, the dispatch
  input does the same in CI.
- **VR16** — "The job runs and reports; mutation-proved by breaking a URL and
  seeing it named": task 2 step 5 locally, and the PR run at ship.
- **Names** — as listed above, used consistently.
- **Placeholders** — none.
