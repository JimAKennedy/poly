# M007/S01 — A package is built and tested unsigned

**Slice:** M007/S01 in `docs/plans/open-source-launch/ledger.md`
**Rows:** OS35 (the macOS artifact is a zip the user must unpack and place by
hand)
**Classification:** bounded. Two shell scripts and a distribution template
under a new `scripts/packaging/`, one CI job beside `pluginval-macos`, and
three steps in `release.yml` locked by new assertions in the existing
`check-release-workflow.mjs`. Decisions are in `M007-decisions.md`.

## Task status

- [x] 1. The package is built, and its payload proves where it installs
- [ ] 2. A CI job installs and removes it on a fresh Mac
- [ ] 3. The release builds, tests and publishes it beside the zip

## Definition of Done

Copied verbatim from the slice:

- [ ] The release workflow builds a `.pkg` that installs the VST3 to
      `/Library/Audio/Plug-Ins/VST3/`, or per-user on request, and the AU to
      `Components/` only if OS17 decided it ships
- [ ] Installing on a clean Mac, then removing, leaves the plug-in folders as
      they were, and the evidence records both
- [ ] The contract check asserts the package step's position and inputs

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `guards` | `bash scripts/check-guards.sh` |

`guards` runs `check-release-workflow.mjs`, `check-workflow-hygiene.mjs` and
`check-scripts-readme.sh` over the new files, and task 1's package test once
it is wired in. DoD 2's clean-Mac evidence is the `package-macos` job's run on
this milestone's PR, recorded at `/jk:ship`; locally, `pkgbuild` builds the
package and its payload is inspected without installing.

## Names used throughout

- `scripts/packaging/build-macos-pkg.sh <vst3> <version> <out.pkg>` — task 1.
- `scripts/packaging/distribution.xml` — task 1, the `productbuild`
  distribution, with `@VERSION@` substituted.
- `scripts/packaging/build-macos-pkg.test.mjs` — task 1, `node --test`;
  skips with a stated reason on any OS but macOS.
- `scripts/packaging/test-macos-pkg.sh <pkg>` — task 2, the install and
  removal test; needs `sudo` for the system-wide half, so it runs on hosted
  runners, never on a developer's machine.
- Package identifier `digital.jk.poly.vst3`; asset
  `poly-<tag>-macos-universal.pkg`.

## Task 1 — The package is built, and its payload proves where it installs

**Files:** create `scripts/packaging/build-macos-pkg.sh`,
`scripts/packaging/distribution.xml`,
`scripts/packaging/build-macos-pkg.test.mjs`; modify `scripts/README.md`,
`scripts/check-guards.sh`.

1. Write the test first. On macOS it builds a fixture bundle (a directory
   `poly_plugin.vst3/Contents/Info.plist` with a minimal plist, in a temp
   dir), runs `build-macos-pkg.sh` on it with version `9.9.9`, and asserts,
   by `pkgutil --expand` into a temp dir and reading the result:
   - the distribution names title "Poly", `enable_localSystem="true"`,
     `enable_currentUserHome="true"`, and `enable_anywhere="false"`;
   - one component package, identifier `digital.jk.poly.vst3`, version
     `9.9.9`, install location `/Library/Audio/Plug-Ins/VST3`;
   - its `PackageInfo` marks the bundle not relocatable;
   - `pkgutil --payload-files` on the component lists
     `./poly_plugin.vst3/Contents/Info.plist` and nothing outside
     `./poly_plugin.vst3`;
   - no `.component` (AU) is anywhere in the payload (OS17 declined it);
   - the script exits non-zero with a clear message when the bundle path does
     not exist or the version is empty.
   On any other OS the test calls `t.skip('pkgbuild is macOS-only')`.
   Run it: red — the script does not exist.
2. Implement `build-macos-pkg.sh` (`set -euo pipefail`): stage the bundle in a
   temp root; `pkgbuild --analyze` to a component plist, set
   `BundleIsRelocatable` false with `plutil`; `pkgbuild --root --component-plist
   --identifier digital.jk.poly.vst3 --version <v> --install-location
   /Library/Audio/Plug-Ins/VST3`; render `distribution.xml` with the version;
   `productbuild --distribution … --package-path …`. Write `distribution.xml`
   with the title, the three domain attributes, one choice referencing the
   component, and `<options customize="never" require-scripts="false"
   hostArchitectures="arm64,x86_64"/>`.
3. Run the test: green. Build the package from the real local bundle
   (`build/VST3/Release/poly_plugin.vst3`) and record in the evidence the
   payload listing's first lines, the identifier and the install location —
   inspected, not installed.
4. Add `scripts/packaging/` to `scripts/README.md` (the directory and its
   four files) and run the test from `check-guards.sh` beside the release
   contract.
5. Run `guards`, `format`. Commit with `Rows:` empty.

## Task 2 — A CI job installs and removes it on a fresh Mac

**Files:** create `scripts/packaging/test-macos-pkg.sh`; modify
`.github/workflows/ci.yml`, `scripts/README.md`,
`scripts/check-workflow-hygiene.mjs` only if a rule needs the new job named
(it should not).

1. Write `test-macos-pkg.sh <pkg>` (`set -euo pipefail`), printing each fact
   it checks:
   - **System-wide:** snapshot `ls -la /Library/Audio/Plug-Ins/VST3` (create
     the directory first if absent, and note that it was absent);
     `sudo installer -pkg <pkg> -target /`; assert
     `/Library/Audio/Plug-Ins/VST3/poly_plugin.vst3/Contents/Info.plist`
     exists and `pkgutil --pkg-info digital.jk.poly.vst3` names that
     location; remove every path `pkgutil --only-files --files
     digital.jk.poly.vst3` lists (under the install location), then the
     bundle's directories, then `sudo pkgutil --forget digital.jk.poly.vst3`;
     assert the folder listing equals the snapshot.
   - **Per-user:** the same against `~/Library/Audio/Plug-Ins/VST3` with
     `installer -pkg <pkg> -target CurrentUserHomeDirectory` (no `sudo`) and
     `pkgutil --volume "$HOME"` for its receipt.
   - Exit non-zero on any mismatch, naming it.
2. Add job `package-macos` to `ci.yml`: `needs: build`, `runs-on: macos-14`,
   download `plugin-macos-14`, read the version from `CMakeLists.txt`'s
   `project(poly VERSION …)`, run `build-macos-pkg.test.mjs`, build the
   package from the downloaded bundle, run `test-macos-pkg.sh`, upload the
   `.pkg` as artifact `package-macos` (7 days). Add `package-macos` to
   `ci-complete`'s `needs` and its result check, as every other job is.
3. Run `bash -n` and `shellcheck` (if installed) on the script;
   `check-workflow-hygiene`, `guards`, `format`. The job itself first runs
   on this milestone's PR.
4. Add `test-macos-pkg.sh` to `scripts/README.md`. Commit with `Rows:` empty.

## Task 3 — The release builds, tests and publishes it beside the zip

**Files:** modify `.github/workflows/release.yml`,
`scripts/check-release-workflow.mjs`, `RELEASING.md`; the ledger; the
evidence file.

1. Add assertions to `check-release-workflow.mjs` first:
   - a step named `Build installer package (macOS)` exists, macOS-gated, runs
     `scripts/packaging/build-macos-pkg.sh` with the located bundle, the
     tag's version, and `poly-${GITHUB_REF_NAME}-${{ matrix.asset }}.pkg`;
   - it comes **after** `Require signing or an explicit allowance` (macOS)
     and `Locate and package VST3 (macOS)`, and **before** `Upload release
     asset`;
   - a step named `Test installer package (macOS)` runs
     `scripts/packaging/test-macos-pkg.sh` on that file, after the build step
     and before upload;
   - the upload step's `path` includes `poly-*.pkg`; `Write checksums` covers
     `*.pkg`; `Attest build provenance`'s `subject-path` covers the `.pkg`;
     `Publish GitHub Release`'s `files` include `dist/*.pkg`.
   Run: red.
2. Edit `release.yml` to satisfy them, deriving the version as the release
   job does (`${GITHUB_REF_NAME#v}`), and update its header comment.
3. Add one paragraph to `RELEASING.md`: the macOS leg also publishes an
   unsigned `.pkg` (signed from M007/S02), tested by install and removal on
   the leg's own runner.
4. Run the contract (green), `guards`, `format`.
5. Evidence; tick the DoD (box 2 with the note that its CI evidence is the PR
   run, recorded at ship); set OS35 and the slice `done`; run `jk-standards
   ledger`. Commit with `Rows: OS35`.

## Self-review

- **DoD 1** (release builds a `.pkg` installing the VST3 to
  `/Library/Audio/Plug-Ins/VST3/` or per-user; AU only if OS17 ships it) —
  task 3 (release), task 1 (install location, both domains, no AU, asserted
  on the built package).
- **DoD 2** (install then remove on a clean Mac leaves the folders as they
  were; evidence records both) — task 2's script and CI job, both domains,
  with the PR run recorded at ship; and the release leg runs it per release.
- **DoD 3** (contract asserts the package step's position and inputs) — task
  3 step 1.
- **OS35** — "installs and uninstalls cleanly on a clean machine (evidence);
  the release publishes it beside the zip; if the AU ships, … universal":
  tasks 2 and 3; the AU clause does not apply (OS17 declined).
- **Names** — as listed above, used consistently.
- **Placeholders** — none.
