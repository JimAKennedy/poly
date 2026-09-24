# M001/S02 — The release proves what it ships

**Slice:** M001/S02 in `docs/plans/open-source-launch/ledger.md`
**Rows:** OS03 (the release never runs `ctest`), OS04 (no checksums, no
provenance), OS05 (pluginval fetched and run unverified in four places)
**Depends:** nothing in the ledger; in practice after M001/S01, because the
contract check both tasks extend is the same file.
**Classification:** bounded. Workflow steps and contract-test cases, in the
pattern `scripts/check-release-workflow.mjs` already establishes: every
assertion is seen red by removing what it guards. Every decision was taken in
`M001-decisions.md` before this plan was written.

## Task status

- [x] 1. The release runs the tests on what it packages
- [x] 2. The release publishes checksums and provenance
- [x] 3. Every pluginval download is verified, and the slice closes

## Definition of Done

Copied verbatim from the slice:

- [x] The release build runs the unit and golden tests on the exact
      configuration it packages, before packaging
- [x] Every Release carries a `SHA256SUMS` asset and a build-provenance
      attestation for each zip
- [x] Every pluginval download in every workflow is verified against a pinned
      digest before it runs
- [x] `scripts/check-release-workflow.mjs` asserts all three, and each new
      assertion is seen red by removing what it guards

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `guards` | `bash scripts/check-guards.sh` |

## What was measured

| | |
|---|---|
| `release.yml` build job steps | Configure (79) → Build (81) → Run pluginval macOS (90) / Windows (106) → Codesign (135) → Notarize (165) → Staple (185) → package macOS (201) / Windows (213) → Upload (227) |
| `release.yml` release job | `permissions: contents: write`; downloads artifacts; `gen-release-notes.mjs`; `softprops/action-gh-release` with `files: dist/*.zip` |
| test targets in the release configuration | built — `add_subdirectory(tests)` is unconditional, `enable_testing()` at `CMakeLists.txt:274` |
| pluginval downloads | `ci.yml:434` and `:480`, `release.yml:98` and `:118`; the Windows steps run under `shell: bash` |
| pluginval 1.0.4 digests | macOS `3c4c533bda0c5059eea3ddaea752d757ee2025041f0f47e6bcb0e87f6082b29f`; Windows `c08e61ce3b96db41636f8ec7e76f4c7e2c13ebdac7fa1b5a1f52b4f32ec715ab` |
| provenance action | `actions/attest-build-provenance` v4.2.2 = `4d101475d8b20a2381f78447822ac1eab6504dd8` |

---

## Task 1 — The release runs the tests on what it packages

**Consumes:** nothing. **Produces:** OS03 closed.

Files: modify `.github/workflows/release.yml`, `scripts/check-release-workflow.mjs`.

1. Add a contract test: a step named `Run tests` exists in the build job,
   runs `ctest --test-dir build --build-config Release --output-on-failure`,
   and its position is after `Build` and before both `Run pluginval` steps
   (compare string offsets in the workflow text, as the existing
   `pluginval step PRECEDES packaging` test does). Run: red.
2. Add the step to `release.yml` after Build, unconditional, so it runs on
   both legs. Run the contract check: green. Remove the step and confirm red;
   restore it.
3. Run `format`, `guards`. Append evidence. Commit with `Slice: M001/S02`,
   `Rows: OS03`, closing OS03.

## Task 2 — The release publishes checksums and provenance

**Consumes:** task 1. **Produces:** OS04 closed.

Files: modify `.github/workflows/release.yml`, `scripts/check-release-workflow.mjs`.

1. Add contract tests: the release job's `permissions` block carries
   `contents: write`, `id-token: write` and `attestations: write`; a step
   writes `dist/SHA256SUMS` with `sha256sum` over `dist/*.zip`; a step uses
   `actions/attest-build-provenance@4d101475d8b20a2381f78447822ac1eab6504dd8`
   with `subject-path` covering `dist/*.zip`; and `gh-release`'s `files`
   includes `dist/SHA256SUMS`. Run: red.
2. Add the permissions, the checksum step, the attestation step, and the
   file, between `Download all release assets` and `Generate release notes`.
   Keep the comment beside `permissions` honest about why each grant exists.
   Run the contract check: green; remove the attestation step and confirm
   red; restore it.
3. Run `format`, `guards`. Append evidence. Commit with `Slice: M001/S02`,
   `Rows: OS04`, closing OS04.

## Task 3 — Every pluginval download is verified, and the slice closes

**Consumes:** task 2. **Produces:** OS05 closed, and the slice.

Files: modify `.github/workflows/ci.yml`, `.github/workflows/release.yml`,
`scripts/check-release-workflow.mjs`,
`docs/plans/open-source-launch/ledger.md`,
`docs/plans/open-source-launch/evidence/M001-S02.md`.

1. Add a contract test over **both** workflow files: every `curl … pluginval`
   line is followed, before any `unzip`, by an `echo "<digest>  pluginval.zip" | shasum -a 256 -c`
   line, and the digest is the pinned constant for that platform. Run: red
   on all four sites.
2. Insert the check after each `curl`, with the digest on the line beside the
   version so the two are updated together, and a one-line comment naming
   where the digest came from. Run the contract check: green; alter one digit
   of one digest and confirm red; restore it.
3. Run `format`, `guards`. Append the evidence, tick all four
   definition-of-done boxes, close OS05, set the slice `done`, run
   `jk-standards ledger`, and commit with `Slice: M001/S02`, `Rows: OS05`.
