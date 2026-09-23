# M001 — decisions

Every question `/jk:auto` asked before running, every answer, and every choice
taken on the owner's behalf. Append-only.

## 2026-09-23 — planning M001/S01 and M001/S02

Measured before asking, on `main` at `4ca409d`: the built bundle's
`moduleinfo.json` carries the module version `0.1.0` from CMake and both
class versions `1.0.0` from `plugids.h`, so the disagreement is visible in one
file a build produces. The VST3 SDK ships
`smtg_target_configure_version_file`, which generates `projectversion.h`
(`FULL_VERSION_STR` and friends) from the CMake project version. The release
configuration already builds the test targets, so a `ctest` step needs no
build change. `gen-release-notes.mjs` matches the bracketed heading token
exactly and case-insensitively, so `## [0.2.0] - unreleased` is found by
`0.2.0`. The pluginval 1.0.4 zips hash to
`3c4c533bda0c5059eea3ddaea752d757ee2025041f0f47e6bcb0e87f6082b29f` (macOS)
and `c08e61ce3b96db41636f8ec7e76f4c7e2c13ebdac7fa1b5a1f52b4f32ec715ab`
(Windows), computed from two fresh downloads on 2026-09-23. The latest
`actions/attest-build-provenance` is `v4.2.2` at
`4d101475d8b20a2381f78447822ac1eab6504dd8`.

- **Q:** What is the first version number — 0.2.0, 1.0.0, or reuse 0.1.0? —
  **A:** 0.2.0.
- **Decision:** `project(poly VERSION 0.2.0)`; June's `## [0.1.0]` section
  stays as history — **Why:** honest about pre-1.0 status, keeps Keep a
  Changelog's released sections immutable, and the drop from the 1.0.0 the
  plugin reports today is harmless because nothing was ever released.
- **Q:** When does `[Unreleased]` become the versioned section the release
  body is built from — now, dated at the cut, or at the cut in M006? —
  **A:** now, dated at the cut.
- **Decision:** the heading becomes `## [0.2.0] - unreleased`; M006 replaces
  `unreleased` with the date when it tags — **Why:** OS02's check that the
  generator prints a section describing the current tree passes today, and a
  contract test can insist a tagged section carries a real date.

### Taken on the owner's behalf

- **The version reaches `plugids.h` through the SDK's own helper**, not a
  hand-rolled `configure_file`: `smtg_target_configure_version_file(poly_plugin)`
  and `kPolyVersionString = FULL_VERSION_STR`. Same result, one fewer template
  to maintain.
- **The npm manifests lose their `version` fields** and `site/package.json`
  gains `"private": true` (webui already has it). Neither is published; a
  version on a private package is a number nobody reads and one more place to
  drift.
- **No empty `[Unreleased]` section is added** after the rename. Every
  upcoming change is in 0.2.0 until it is cut; M006 opens a fresh
  `[Unreleased]` when it dates the section. The contract test that proved the
  generator on `Unreleased` now proves it on the CMake version instead, which
  is the tie OS02 wants: the section for the version the build declares must
  exist.
- **`ctest` runs after Build and before the validator and pluginval**, on
  both legs, with `--build-config Release` for the Visual Studio generator.
- **Provenance uses `actions/attest-build-provenance` pinned by SHA**, with
  the release job's permissions raised to `contents: write`,
  `id-token: write`, `attestations: write` — the minimum the action documents.
  `SHA256SUMS` is generated with `sha256sum` in the release job and attached
  beside the zips.
- **pluginval digests are verified with `shasum -a 256 -c`** on all four
  steps: the Windows steps already run under `shell: bash` (git-bash), so one
  command serves both platforms.
- **The AU plist's `0xFFFFFFFF` is left alone.** OS01 assigns it to OS17; it
  matters only if the AU ships.
