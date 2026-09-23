# M001/S01 — One version, from one place

**Slice:** M001/S01 in `docs/plans/open-source-launch/ledger.md`
**Rows:** OS01 (five version strings disagree), OS02 (the changelog has no
section for the version that would be tagged)
**Depends:** nothing.
**Classification:** bounded. A CMake call the SDK provides, one header line,
two manifest edits, a heading rename, a guard, and a contract test. Every
decision was taken in `M001-decisions.md` before this plan was written.

## Task status

- [x] 1. The plugin reports the CMake version, and a guard keeps it that way
- [x] 2. The changelog has the section the first tag will publish, and the slice closes

## Definition of Done

Copied verbatim from the slice:

- [x] `project(poly VERSION …)` is the only hand-edited version in the tree that
      reaches a shipped artifact
- [x] The plugin's factory class info reports that version, proved by changing
      it and reading the new value back from a built bundle
- [x] A check fails if a hand-typed version string returns to `plugids.h`, seen
      red before being trusted
- [x] The first version number is decided and recorded in this milestone's
      decisions file, and the CHANGELOG section `gen-release-notes.mjs` will
      extract for it describes the current tree

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `unit` | `cmake --build build --config Release --parallel && ctest --test-dir build --build-config Release --output-on-failure` |
| `guards` | `bash scripts/check-guards.sh` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |

## What was measured

| | |
|---|---|
| `CMakeLists.txt:20` | `VERSION 0.1.0` |
| `plugin/source/plugids.h:15` | `static constexpr auto kPolyVersionString = "1.0.0";`, consumed twice in `factory.cpp` |
| `build/VST3/Release/poly_plugin.vst3/Contents/Resources/moduleinfo.json` | module `"Version": "0.1.0"`, both classes `"Version": "1.0.0"` |
| `webui/package.json` | `"private": true`, `"version": "0.1.0"` |
| `site/package.json` | no `private`, `"version": "0.0.1"` |
| SDK helper | `smtg_target_configure_version_file(target)` in `SMTG_Bundle.cmake` generates `projectversion.h` defining `FULL_VERSION_STR "@PROJECT_VERSION@"` and adds its directory to the target's includes |
| `CHANGELOG.md` headings | `## [Unreleased]` at line 7, `## [0.1.0] - 2026-06-27` at line 102 |
| `scripts/gen-release-notes.mjs:56` | matches `^##\s+\[token\]`, token compared case-insensitively and exactly |
| `scripts/check-guards.sh` | node guards are `run_guard "<name>" node --test scripts/<file>.mjs` lines |

---

## Task 1 — The plugin reports the CMake version, and a guard keeps it that way

**Consumes:** nothing. **Produces:** OS01 closed.

Files: modify `CMakeLists.txt`, `plugin/CMakeLists.txt`,
`plugin/source/plugids.h`, `webui/package.json`, `site/package.json`,
`scripts/check-guards.sh`, `scripts/README.md`; create
`scripts/check-version-source.mjs`.

1. Write `scripts/check-version-source.mjs` as a `node --test` file with three
   cases: `plugin/source/plugids.h` contains no quoted `\d+\.\d+\.\d+` literal
   and does include `projectversion.h`; `CMakeLists.txt` declares
   `project(poly VERSION x.y.z)`; neither `webui/package.json` nor
   `site/package.json` has a `version` key and both are `"private": true`.
   Wire it into `scripts/check-guards.sh` as
   `run_guard "version-source" node --test scripts/check-version-source.mjs`
   beside the other node guards, and list it in `scripts/README.md`, which
   `check-scripts-readme` requires. Run it: all three cases must fail on the
   current tree — that is the red step.
2. `CMakeLists.txt`: `VERSION 0.1.0` → `VERSION 0.2.0`, and call
   `smtg_target_configure_version_file(poly_plugin)` **in the top-level list**
   beside `add_subdirectory(plugin)` — the helper writes the header into the
   current binary directory and adds `PROJECT_BINARY_DIR` to the includes, so
   called from `plugin/CMakeLists.txt` the file lands where nothing looks
   (found when the first build failed; corrected before the task ran on).
   In `tests/CMakeLists.txt`, add `${PROJECT_BINARY_DIR}` to the three test
   targets that compile plugin sources directly, for the same reason. `plugids.h`: `#include "projectversion.h"` and
   `static constexpr auto kPolyVersionString = FULL_VERSION_STR;`. Remove
   `"version"` from both npm manifests and add `"private": true` to
   `site/package.json`.
3. Build (`cmake --build build --config Release --parallel`) and read
   `build/VST3/Release/poly_plugin.vst3/Contents/Resources/moduleinfo.json`:
   the module and both classes must report `0.2.0`. Then prove the wiring
   rather than the number: set `VERSION 0.2.1` in `CMakeLists.txt`,
   reconfigure and rebuild, read `0.2.1` back from all three places, and
   restore `0.2.0`, rebuilding once more. Record all three readings.
4. Run the guard again: green. Run `npm --prefix site test` and
   `npm --prefix webui test` once to confirm the manifests without a version
   still load (npm accepts a versionless private package).
5. Run `format`, `unit`, `guards`. Append the evidence to
   `evidence/M001-S01.md` with the `moduleinfo.json` readings. Commit with
   `Slice: M001/S01`, `Rows: OS01`, closing OS01 in the same edit.

## Task 2 — The changelog has the section the first tag will publish, and the slice closes

**Consumes:** task 1's version. **Produces:** OS02 closed, and the slice.

Files: modify `CHANGELOG.md`, `scripts/check-release-workflow.mjs`,
`docs/plans/open-source-launch/ledger.md`,
`docs/plans/open-source-launch/evidence/M001-S01.md`.

1. In `scripts/check-release-workflow.mjs`, replace the test
   `gen-release-notes emits a non-empty body for Unreleased` with one that
   reads the version from `CMakeLists.txt`'s `project(poly VERSION …)` line
   and asserts `gen-release-notes.mjs <that version>` prints a non-empty body.
   Keep the `0.1.0` test: June's section remains and must stay extractable.
   Run `node --test scripts/check-release-workflow.mjs`: the new case fails,
   because no `0.2.0` section exists yet — the red step.
2. In `CHANGELOG.md`, rename `## [Unreleased]` to `## [0.2.0] - unreleased`.
   Nothing else in the section changes. Do not add an empty `[Unreleased]`;
   M006 opens one when it dates the section (recorded in the decisions).
3. Run the contract check: green. Run
   `node scripts/gen-release-notes.mjs 0.2.0 | head -5` and confirm it prints
   the section's first entries.
4. Run `format`, `unit`, `guards`, `doc-discipline`. Append the evidence,
   tick all four definition-of-done boxes, close OS02, set the slice `done`,
   run `jk-standards ledger`, and commit with `Slice: M001/S01`,
   `Rows: OS02`.
