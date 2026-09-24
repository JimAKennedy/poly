# M001 — The release describes itself

**Review-gate report.** Generated from the ledger, `git log` and
`M001-decisions.md`. `/jk:ship` is the next step and it is the owner's to take.

**Vision:** The version a DAW reports, the version the tag names and the notes
the Release publishes are the same thing, and the release tests what it ships.

**Branch:** `milestone/M001-release-honesty` · **Ledger:** `docs/plans/open-source-launch/ledger.md`

## Slices

| Slice | Title | Rows | Status |
|---|---|---|---|
| M001/S01 | One version, from one place | OS01, OS02 | done |
| M001/S02 | The release proves what it ships | OS03, OS04, OS05 | done |

Every row is `done`.

## Definition of done

- [x] `project(poly VERSION …)` is the only hand-edited version in the tree that
      reaches a shipped artifact
- [x] The plugin's factory class info reports that version, proved by changing
      it and reading the new value back from a built bundle
- [x] A check fails if a hand-typed version string returns to `plugids.h`, seen
      red before being trusted
- [x] The first version number is decided and recorded in this milestone's
      decisions file, and the CHANGELOG section `gen-release-notes.mjs` will
      extract for it describes the current tree
- [x] The release build runs the unit and golden tests on the exact
      configuration it packages, before packaging
- [x] Every Release carries a `SHA256SUMS` asset and a build-provenance
      attestation for each zip
- [x] Every pluginval download in every workflow is verified against a pinned
      digest before it runs
- [x] `scripts/check-release-workflow.mjs` asserts all three, and each new
      assertion is seen red by removing what it guards

## What changed

**The version is typed once** (OS01). `project(poly VERSION 0.2.0)`; the SDK's
`smtg_target_configure_version_file` generates `projectversion.h`; `plugids.h`
reads `FULL_VERSION_STR`. The built bundle's `moduleinfo.json`, which had shown
module `0.1.0` beside class `1.0.0`, reads `0.2.0` in all three places, and
followed a mutation to `0.2.1` and back. Both private npm manifests lost their
version fields. `check-version-source.mjs`, wired into `guards`, was red on the
old tree before being trusted.

**The changelog has the section the first tag will publish** (OS02).
`[Unreleased]` is `## [0.2.0] - unreleased`, unchanged inside; M006 dates it
when it tags. The contract test that once proved the generator on
`Unreleased` now reads the CMake version and asks for that, so a version bump
without a section fails `guards` rather than the tag.

**The release tests what it ships** (OS03). `Run tests` runs `ctest` between
Build and pluginval on both legs, unconditional. The macOS universal binary's
tests run for the first time anywhere.

**A downloader can verify what they got** (OS04). `SHA256SUMS` over every zip
and an `actions/attest-build-provenance` attestation per zip, with the job's
grants asserted to be exactly `contents`, `id-token`, `attestations: write`.

**The one unpinned binary is pinned** (OS05). All four pluginval downloads,
in both workflows, pipe the v1.0.4 asset's SHA-256 through
`shasum -a 256 -c` before unzip; the contract test requires exactly four
sites and the pinned constants.

## Validation

Re-run on `02b8c4c`, the head this report describes.

| Token | Command | Result | On |
|---|---|---|---|
| `format` | `pre-commit run --all-files` | pass | `02b8c4c` |
| `unit` | `cmake --build build … && ctest …` | pass — **698/698** | `4e35029` (last commit touching C++/CMake) |
| `guards` | `bash scripts/check-guards.sh` | pass — 67 cases | `02b8c4c` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` | pass | `02b8c4c` |
| `ledger` | `jk-standards ledger` | pass — 6 conform | `02b8c4c` |

**27 release-workflow contract cases before the milestone, 32 after**, plus
the four-case version-source guard. Every addition was seen red by removing or
tampering with what it guards.

## Traceability

Every commit carries `Slice:`. **No untraced commits.**

| Commit | Slice | Rows | Subject |
|---|---|---|---|
| `9182a7c` | M001/S01 | | docs(plans): front-load M001's decisions and plan both slices |
| `9e65385` | M001/S01 | OS01 | build: the plugin reports the version CMake declares, and nothing else types one |
| `4e35029` | M001/S01 | OS02 | docs(changelog): the section the first tag will publish is 0.2.0, tied to the build |
| `d53456b` | M001/S02 | OS03 | ci(release): run the tests on the configuration that ships |
| `d04fdc3` | M001/S02 | OS04 | ci(release): publish checksums and a build-provenance attestation per zip |
| `02b8c4c` | M001/S02 | OS05 | ci: verify every pluginval download against its pinned digest before running it |

## What a reviewer should look at twice

### The plugin's reported version drops from 1.0.0 to 0.2.0

Every DAW that has scanned a local build will see the number go down. Nothing
was ever released, so no user has a project that recorded 1.0.0, and state
compatibility rides on `kStateVersion`, not the display version. Recorded as
the owner's choice in the decisions.

### Two plan steps were wrong and were corrected before they ran

The SDK helper must be called from the top-level `CMakeLists.txt`: it writes
into the current binary directory but adds `PROJECT_BINARY_DIR` to the
includes, and the first build failed. And three test targets compile plugin
sources directly, so they needed the generated header's directory too. Both
are documented in the plan, the evidence and the decisions.

### Doc-drift trailers

Task 1's commit touched files the drift map pairs with `testing-strategy.md`
and `cubase-workflow.md`. Neither document describes what changed — an include
directory, an unpublished version, where a string comes from — and task 2's
commit carries a `Docs-Not-Affected` trailer for each with the reason.
`doc-discipline` reports both mappings satisfied.

### No `[Unreleased]` section exists until M006

Every upcoming change is in `## [0.2.0] - unreleased`. `/jk:ship`'s docs sync
appends changelog entries under the top section, so this milestone's entry
lands there too, which is right: it is part of 0.2.0. M006 opens a fresh
`[Unreleased]` when it dates the section.

### The provenance action is new to the tree

`actions/attest-build-provenance` v4.2.2 at `4d101475…`, resolved from the tag
on 2026-09-23. Dependabot's actions cadence will offer bumps; the contract test
pins the SHA, so a bump is a deliberate two-file change.

## Decisions

Copied from `M001-decisions.md` so the report stands alone.

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

## 2026-09-23 — judgment calls during M001/S01 task 1

- **The SDK helper is called from the top-level `CMakeLists.txt`, not the
  plugin's.** The plan said the plugin's; the first build failed with
  `projectversion.h` not found, because the helper writes the file into the
  current binary directory and adds `PROJECT_BINARY_DIR` to the includes —
  the two match only at the top level. Obviously right: the alternative was a
  second include line duplicating what the helper already adds.
- **The three test targets that compile plugin sources gain
  `${PROJECT_BINARY_DIR}`.** They include `plugin/source` rather than linking
  `poly_plugin`, so the plugin's include directories never reach them. A
  one-line addition beside the existing include, with the reason in a comment,
  rather than restructuring the tests to link the plugin.
