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
