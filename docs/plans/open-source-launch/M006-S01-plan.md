# M006/S01 — The first pre-release exists

**Slice:** M006/S01 in `docs/plans/open-source-launch/ledger.md`
**Rows:** OS31 (no release has ever been cut), OS32 (every tag publishes as
a full release), OS33 (what a clean machine shows has never been observed)
**Depends:** M001/S01, M001/S02, M006/S02.
**Decisions consumed:** `M006-decisions.md`, 2026-09-29 — the tag is
`v0.2.0-rc.1` and the run pushes it; OS33 is measured by
`release-verify.yml` on hosted runners; M005/S01 is on `main` before the
tag.

## Task status

- [x] Task 1 — A hyphen-suffixed tag publishes as a pre-release from its
      base version's notes, and the contract asserts it (OS32)
- [x] Task 2 — `release-verify.yml` installs a Release on fresh runners and
      reports what a clean machine measures, with its own contract (OS33)
- [x] Task 3 — The tag is pushed, the Release exists, the verifier's report
      is in the evidence, and the slice closes (OS31, OS33)

## Definition of Done

- [x] A tag with a pre-release suffix publishes a Release marked pre-release,
      and the contract check asserts the mapping
- [x] Both zips were downloaded, verified against `SHA256SUMS` and the
      provenance attestation, and installed on a fresh hosted runner per
      platform by `release-verify.yml`, whose report records the OS version,
      the quarantine and Gatekeeper verdicts on macOS, the Mark-of-the-Web and
      Authenticode verdicts on Windows, and pluginval loading the bundle from
      the installed location
- [x] Anything the first cut broke is a row here or an issue, not a note

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `guards` | `bash scripts/check-guards.sh` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |

## Task 1 — A hyphen-suffixed tag publishes as a pre-release from its base version's notes, and the contract asserts it (OS32)

**Files:** modify `scripts/gen-release-notes.mjs`,
`scripts/check-release-workflow.mjs`, `.github/workflows/release.yml`,
`RELEASING.md`.

1. Contract: `a hyphen-suffixed tag publishes as a pre-release (OS32)` —
   the publish step carries `prerelease: ${{ contains(github.ref_name, '-') }}`;
   and `gen-release-notes maps a pre-release version to its base section
   and links the full tag (OS32)` — `node scripts/gen-release-notes.mjs
   0.2.0-rc.1` exits 0, its body equals the body for `0.2.0` except the
   closing link, which names `v0.2.0-rc.1`. Run: both red.
2. Generator: split the version at the first `-`; the section is the base;
   the link uses the version as given. Update the header comment.
   `release.yml`: the `prerelease:` input as above, with a comment naming
   OS32.
3. Run the contract: green. `RELEASING.md`: one sentence — a tag with a
   hyphen suffix is a pre-release and takes its notes from the base
   version's section.
4. Run `format`, `guards`, `doc-discipline`. Evidence. Tick. Commit with
   `Rows: OS32`.

## Task 2 — `release-verify.yml` installs a Release on fresh runners and reports what a clean machine measures, with its own contract (OS33)

**Files:** create `.github/workflows/release-verify.yml`,
`scripts/check-release-verify-workflow.mjs`; modify `scripts/check-guards.sh`,
`scripts/README.md`, `RELEASING.md`.

1. Write `scripts/check-release-verify-workflow.mjs` (node:test, structural
   matches as the release contract does) asserting the workflow: triggers on
   `workflow_dispatch` with a required `tag` input and on `release` with
   `types: [published]`; top-level `permissions: contents: read`; two jobs
   on `macos-14` and `windows-2022`; each downloads with
   `gh release download` the tag's zip for its platform plus `SHA256SUMS`,
   runs `sha256sum -c` (`shasum -a 256 -c` on macOS) and
   `gh attestation verify … --owner JimAKennedy`; each installs the bundle
   into the user's VST3 folder (`~/Library/Audio/Plug-Ins/VST3` and
   `$env:LOCALAPPDATA\Programs\Common\VST3` respectively); the macOS job
   applies `com.apple.quarantine` with `xattr -w` and records `xattr -l`,
   `codesign -dv` and `spctl --assess --type install` verdicts; the Windows
   job records the `Zone.Identifier` stream and `Get-AuthenticodeSignature`
   status; each runs pluginval 1.0.4 (pinned, digest-verified as in
   `ci.yml`) at strictness 8 against the installed path; each writes
   `report-<platform>.md` and uploads it with `actions/upload-artifact`.
   Run: red (no workflow).
2. Write the workflow to satisfy it, with a comment naming OS33 and the
   decision. Every action pinned by SHA (the same pins `ci.yml` uses);
   `node --test scripts/check-workflow-hygiene.mjs` must stay green.
   Verdict commands must not fail the job — they are measurements — so each
   is wrapped to record its exit code and output; only the download,
   checksum, attestation and pluginval steps may fail the job.
3. Wire the contract into `check-guards.sh` and `scripts/README.md`. Run
   `guards`: green. `RELEASING.md`: a paragraph on the verifier.
4. Run `format`, `guards`, `doc-discipline`. Evidence. Tick. Commit. No row
   closes yet: OS33 closes on the report in task 3.

## Task 3 — The tag is pushed, the Release exists, the verifier's report is in the evidence, and the slice closes (OS31, OS33)

**Files:** modify `docs/plans/open-source-launch/ledger.md`, the evidence
file. The tag and the Release, on GitHub.

1. Preconditions, all checked and recorded: the branch is rebased onto
   `origin/main` and the gate passes; `origin/main` contains M005/S01 and
   M006/S02 and this slice's tasks 1 and 2 (`git log origin/main --grep`
   for each `Slice:`); `gh variable list` shows `ALLOW_UNSIGNED_RELEASE=true`;
   `CMakeLists.txt` declares `0.2.0`. Any precondition false is a halt.
2. `git tag v0.2.0-rc.1 <origin/main sha>` and `git push origin v0.2.0-rc.1`.
   Watch `gh run list --workflow release.yml`; the run must be green. Record
   the run URL, the Release URL, the assets and their sizes, the body's
   `Signed:` line, and `prerelease: true`.
3. `gh workflow run release-verify.yml -f tag=v0.2.0-rc.1`; watch; download
   both reports; quote them in the evidence: OS version, checksum and
   attestation results, the verdicts, pluginval's result.
4. If either run is red: stop, file a row or an issue per the DoD, and
   report — do not re-tag.
5. Tick every DoD box here and in the ledger; set the slice `done`; OS31
   and OS33 `done`. Run `jk-standards ledger`. Commit with
   `Rows: OS31, OS33`.

**Sequencing.** The tag is cut from `main`, so task 3 needs tasks 1 and 2,
S02 and M005/S01 all merged first. The milestone therefore ships in two
passes. After task 2 the run halts; the owner runs `/jk:ship --slice
M006/S02`, whose scope is S02 (done) and whose traceability lists S01's two
commits as traced work; then re-issues `/jk:auto M006`, which rebases this
branch onto the updated `main`, re-runs the gate, finds task 3 and runs it.
Task 3's evidence commit lands on the rebased branch and `/jk:ship` closes
the milestone. The changelog heading stays `## [0.2.0] - unreleased` for
the release candidate: M001 decided the date is written when 0.2.0 itself
is tagged, and the generator maps `0.2.0-rc.1` to that section regardless.

## Self-review

| DoD | Task |
|---|---|
| A suffixed tag publishes as pre-release; the contract asserts the mapping | 1 |
| Both zips verified and installed on fresh runners with the measured verdicts and pluginval | 2 builds the verifier; 3 runs it and records the report |
| Anything broken is a row or an issue | 3 |

| Row | Task | Verification produced |
|---|---|---|
| OS31 | 3 | the run, the assets and their sizes |
| OS32 | 1 | `prerelease: true` for a hyphen tag; the contract red without it |
| OS33 | 2, 3 | the verifier's report per platform, quoted; the contract locks the workflow |

No placeholders.
