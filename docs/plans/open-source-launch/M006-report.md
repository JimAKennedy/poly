# M006 — A release is cut, and says whether it is signed

**Review-gate report.** Generated from the ledger, `git log`, the two GitHub
runs and `M006-decisions.md`. `/jk:ship` is the next step and it is the
owner's to take.

**Vision:** The pipeline has run to completion against a real tag, what a
stranger meets on each platform is recorded verbatim, and an unsigned build
cannot ship without that being a deliberate, recorded choice.

**Branch:** `milestone/M006-first-cut` · **Ledger:** `docs/plans/open-source-launch/ledger.md`

## The milestone ran in two passes

The tag is cut from `main`, so the workflow changes had to merge first.
Pass one landed S02 and S01's first two tasks and shipped as
[#347](https://github.com/JimAKennedy/poly/pull/347) (`--slice M006/S02`),
squashed into `main` as `36b2283`. Pass two pushed the tag from that commit
and recorded what happened; it is the one commit now on this branch. Both
passes were decided on 2026-09-29 and are in the decisions file, as is the
re-scoping of OS33 from a by-hand install to a workflow on fresh hosted
runners, and the swap that put S02 before S01.

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M006/S02 | An unsigned build cannot ship quietly | OS34 | done |
| M006/S01 | The first pre-release exists | OS31, OS32, OS33 | done |

Every row is `done`.

## Definition of done

- [x] With no signing secrets, the release job fails, unless a repository
      variable named for the purpose explicitly allows an unsigned release
- [x] The Release body states whether each artifact is signed and notarized
- [x] The contract check asserts both, each seen red
- [x] A tag with a pre-release suffix publishes a Release marked pre-release,
      and the contract check asserts the mapping
- [x] Both zips were downloaded, verified against `SHA256SUMS` and the
      provenance attestation, and installed on a fresh hosted runner per
      platform by `release-verify.yml`, whose report records the OS version,
      the quarantine and Gatekeeper verdicts on macOS, the Mark-of-the-Web and
      Authenticode verdicts on Windows, and pluginval loading the bundle from
      the installed location
- [x] Anything the first cut broke is a row here or an issue, not a note

## What changed

**An unsigned build cannot ship quietly** (OS34). Each release leg records
whether it signed and refuses to package an unsigned bundle unless
`ALLOW_UNSIGNED_RELEASE` is exactly `true`; the release job appends
`Signed: macOS <yes|no> (notarized <yes|no>) · Windows <yes|no>` from the
records. The variable is `true` on the repository for this deliberately
unsigned pre-release and comes out when M007 signs.

**A release candidate lands as a pre-release** (OS32). `prerelease` follows
the tag's hyphen; the notes generator takes the base version's musician
block and links the changelog at the full tag.

**A workflow measures what a clean machine sees** (OS33).
`release-verify.yml` installs a Release's zips on fresh `macos-14` and
`windows-2022` runners, verifies checksums and attestation, records the
verdicts that produce the platform's dialogs, clears the quarantine the
README's way, runs pluginval on the installed bundle, and uploads a report.
Eleven contract cases lock it.

**The first release exists** (OS31).
[v0.2.0-rc.1](https://github.com/JimAKennedy/poly/releases/tag/v0.2.0-rc.1):
`isPrerelease: true`, `poly-v0.2.0-rc.1-macos-universal.zip` (705,854
bytes), `poly-v0.2.0-rc.1-windows-x64.zip` (751,852 bytes), `SHA256SUMS`;
a 309-word body ending `Signed: macOS no (notarized no) · Windows no`.
Release run [36648237270](https://github.com/JimAKennedy/poly/actions/runs/36648237270),
verifier run [36648752906](https://github.com/JimAKennedy/poly/actions/runs/36648752906),
both green.

**What the clean machines measured.** macOS 14.8.9: the bundle is ad-hoc
linker-signed; `spctl` says `rejected, no usable signature` both with the
quarantine attribute and after the README's `xattr -dr`; pluginval then
loads it, `SUCCESS`. Windows Server 2022: the zip carries the
Mark-of-the-Web, extraction does not propagate it to the binary,
Authenticode says `NotSigned`, pluginval `SUCCESS`. Both reports are quoted
in full in `evidence/M006-S01.md`.

## Validation

On the head this report describes, `ea11d98`, and on `36b2283` for the
tokens that ran before the tag.

| Token | Command | Result | On |
|---|---|---|---|
| `format` | `pre-commit run --all-files` | pass | `ea11d98` |
| `guards` | `bash scripts/check-guards.sh` | pass — 20 guard invocations; release contract 37/37, verifier contract 11/11 | `ea11d98` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` | pass | `ea11d98` |
| `ledger` | `jk-standards ledger` | pass — 6 conform | `ea11d98` |
| pre-push gate | `scripts/pre-push-check.sh` | pass, on the reset head before the tag | `36b2283` |
| release run | `release.yml` on `v0.2.0-rc.1` | success, both legs and the release job | `36b2283` |
| verifier run | `release-verify.yml` on `v0.2.0-rc.1` | success, both jobs | `36b2283` |

Every contract case this milestone added was red before it was trusted: the
gate and record cases on the old `release.yml`, the pre-release cases on the
old generator and publish step, and the verifier's eleven with no workflow.

## Traceability

**Pass one**, squashed into `main` as `36b2283` by #347; hashes as they were
on the branch before the squash:

| Commit | Slice | Rows | Subject |
|---|---|---|---|
| `fdb10be` | M006/S02 | | docs(plans): front-load M006's decisions, amend its shape, and plan both slices |
| `9390f1f` | M006/S02 | OS34 | ci(release): an unsigned build cannot ship quietly, and the body says what was signed |
| `8b101b4` | M006/S01 | OS32 | ci(release): a hyphen-suffixed tag is a pre-release, and takes its notes from its base version |
| `870d8d3` | M006/S01 | | ci: release-verify.yml measures what a clean machine sees of a published Release |
| `e1a3cbc` | M006/S02 | | docs: the changelog records M006/S02 and the tooling that precedes the tag |

**Pass two**, on this branch:

| Commit | Slice | Rows | Subject |
|---|---|---|---|
| `ea11d98` | M006/S01 | OS31, OS33 | release: v0.2.0-rc.1 exists, and a clean machine's verdicts on it are on record |

**No untraced commits** in either pass.

## What a reviewer should look at twice

### A public pre-release now exists, unsigned

`v0.2.0-rc.1` is on the Releases page, marked pre-release, with a body that
says it is unsigned on both platforms. Anyone can download it. The README's
quarantine instruction is what a macOS user must follow; the Windows zip
carries the Mark-of-the-Web and its extracted binary does not.

### Gatekeeper rejects the bundle regardless of the quarantine flag

The verifier shows `spctl` rejecting the ad-hoc-signed bundle before and
after the clear, while pluginval loads it after the clear. The report does
not show whether pluginval would have loaded it *before* the clear, so the
README's instruction is proved sufficient, not necessary. Issue
[#349](https://github.com/JimAKennedy/poly/issues/349) adds that
measurement.

### The verifier's automatic trigger is dead

`release: published` never fires for Releases created by `release.yml`'s
own token. Only the dispatched run exists. Issue
[#348](https://github.com/JimAKennedy/poly/issues/348) names the two fixes.
Until one lands, verifying a Release means dispatching the workflow.

### `ALLOW_UNSIGNED_RELEASE=true` is live

Set through the API for this cut. The gate is real but currently open on
purpose; M007/S02 removes the variable when signing is provisioned.

### Three judgment calls in flight

The branch was reset to `main` rather than rebased after the squash merge,
because the two trees were identical; the merged `--slice` PR was not read
as the milestone's PR for the orient rule; and two verifier contract
failures during development were corrected in the contract, not the
workflow. All in the decisions file and the evidence.

### The changelog heading still says `unreleased`

By M001's decision, `## [0.2.0] - unreleased` is dated when 0.2.0 itself is
tagged; the release candidate maps to it as is.

## Decisions

Copied from `M006-decisions.md` so the report stands alone.

# M006 — decisions

Every question `/jk:auto` asked before running, every answer, and every choice
taken on the owner's behalf. Append-only.

## 2026-09-29 — planning M006/S02 and M006/S01

Why this run targets M006: `/jk:auto` was re-issued with no argument
straight after the report that M005's remaining slice is blocked on
M006/S01. The first in-progress milestone was M005, and it could not move;
the milestone that unblocks it is this one.

Measured before asking, on `main` at `19e7aa3`. One tag exists,
`v0.1.0-doccov-baseline`, and no Release. The repository's only secret is
`OPENROUTER_API_KEY` and it has no variables, so every signing step in
`release.yml` skips and the leg ships an unsigned zip without saying so.
`softprops/action-gh-release` is called without `prerelease:`.
`scripts/gen-release-notes.mjs` matches the bracketed changelog token
exactly, so `0.2.0-rc.1` finds no section and exits 1 — a pre-release tag
would fail the release today. M005/S01, which makes the generator emit the
musician block, is `done` on `milestone/M005-release-notes` and unmerged;
its generator change and this milestone's would conflict. Hosted
`macos-14` and `windows-2022` runners are provisioned fresh for every job.

- **Q:** M005/S01 is unmerged and M006's generator change conflicts with
  it; how are the two sequenced? — **A:** ship M005/S01 first.
- **Decision:** this run plans M006 and halts; the owner runs
  `/jk:ship --slice M005/S01`; `/jk:auto M006` then rebases this branch onto
  `main` and executes — **Why:** the first Release body must be the musician
  block, and a milestone never stacks on an unmerged one.
- **Q:** the first pre-release tag's name, and who pushes it? — **A:**
  `v0.2.0-rc.1`, the run pushes it.
- **Decision:** the run pushes `v0.2.0-rc.1` when it reaches that task, as
  an authorised step recorded here; the generator maps a hyphen-suffixed
  version to its base section and links the changelog at the full tag —
  **Why:** the owner authorised the outward-facing step up front, which is
  what front-loading is for.
- **Q:** swap S01 and S02 so the first cut exercises the unsigned gate and
  its body carries the Signed line? — **A:** swap.
- **Decision:** S02 depends on M001/S02 alone and S01 on S02; the ledger is
  amended; the run sets the `ALLOW_UNSIGNED_RELEASE` repository variable to
  `true` through the API before the tag, recorded in the evidence — **Why:**
  one cut instead of two, and the first Release says "Signed: no" in so many
  words.
- **Q:** OS33 asks for both zips installed by hand on a clean machine per
  platform with every dialog recorded; the owner asked for no manual work
  and a check that runs repeatedly — automated users, or standing test
  users cleaned down? — **A:** hosted runners as the clean machines.
- **Decision:** `release-verify.yml`, dispatched with a tag, downloads the
  Release assets on fresh `macos-14` and `windows-2022` runners, verifies
  `SHA256SUMS` and the provenance attestation, installs the bundle into the
  user's VST3 folder, measures the verdicts that produce the dialogs — the
  quarantine attribute and `spctl --assess` on macOS, the Mark-of-the-Web
  and `Get-AuthenticodeSignature` on Windows — runs pluginval against the
  installed bundle, and uploads a report; the run dispatches it after the
  tag and reads the report into the evidence; OS33's DoD and verification
  are amended to say so — **Why:** a hosted runner is a fresh VM every time,
  which is what "clean" meant, with no user to create or clean down; a
  dialog cannot be shown headless, but the verdict that triggers it can be
  measured, and the dialog's text is documented by the platform vendor.

### Taken on the owner's behalf

- **The unsigned gate is one step per leg**, after the signing steps: it
  fails unless the leg signed the artifact or `vars.ALLOW_UNSIGNED_RELEASE`
  is `true`. Windows has no signing step, so its gate reads the same
  variable until M008 adds one. Each leg records `signed=yes|no` (macOS also
  `notarized=yes|no`) in a small file uploaded with its zip.
- **The Signed lines are appended by the release job**, not written into
  the changelog: `Signed: macOS no · Windows no` beneath the musician block,
  from the legs' files, so the body states what this run did rather than
  what a maintainer typed.
- **`release-verify.yml` runs on `workflow_dispatch` with a `tag` input**
  and also on `release: published`, so every future Release is verified
  without anyone remembering to dispatch it. It downloads with
  `gh release download`, which does not set a quarantine attribute, so the
  macOS job applies `com.apple.quarantine` to the zip itself before
  extracting, as a browser download would, and reports `spctl` on the
  bundle both with and without the attribute.
- **The verification workflow has its own contract**,
  `scripts/check-release-verify-workflow.mjs`, wired into `guards`,
  because a verifier nothing checks is the gap this programme keeps
  finding.
- **Anything the first cut breaks** becomes a row or an issue, per the DoD;
  the run halts on a red release run and reports it rather than re-tagging.
- **The tag is pushed from the merged `main`**, not from the milestone
  branch, and only after M006/S02 and M005/S01 are on `main`; the run
  checks both before pushing.
- **The milestone ships in two passes.** The tag is cut from `main`, so the
  workflow changes must merge before it: after S02 and S01's first two tasks
  land, the owner runs `/jk:ship --slice M006/S02`, then `/jk:auto M006`
  rebases the branch, runs the tag task on `main`, and `/jk:ship` closes
  the milestone with the evidence.
- **The changelog heading stays `unreleased` for the release candidate.**
  M001 decided the date is written when 0.2.0 itself is tagged; the
  generator maps `0.2.0-rc.1` to the `0.2.0` section whatever the heading's
  date suffix says.

## 2026-09-29 — judgment calls when resuming for the tag

- **The merged `--slice` PR does not end the run.** The orient rule stops
  when a PR has merged for the branch; that rule is for a milestone's PR,
  and #347 was the `--slice M006/S02` pass this file planned on 2026-09-29.
  The milestone is still `in-progress` with S01's tag task open, so the run
  continues to it.
- **The branch was reset to `origin/main`, not rebased.** After the squash
  merge every one of the branch's five commits was already in `main`'s
  content, so replaying them conflicted on the first; `git diff` between the
  two trees was empty, and the reset discarded nothing. Obviously right: a
  rebase that can only reproduce what `main` already holds has nothing to
  carry.
