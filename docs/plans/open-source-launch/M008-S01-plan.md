# M008/S01 — An installer is built and tested unsigned

**Slice:** M008/S01 in `docs/plans/open-source-launch/ledger.md`
**Rows:** OS37 (the Windows artifact is a zip; the installer format is a
genuine choice)
**Classification:** bounded, but run against CI rather than locally: WiX
builds MSIs only on Windows. A WiX source and two PowerShell scripts under
`scripts/packaging/`, one CI job, and release steps locked by new contract
assertions — the shape M007/S01 established on macOS. Decisions, including
the format comparison OS37 requires, are in `M008-decisions.md`.

## Task status

- [ ] 1. The WiX source says what the installer does, and a test holds it there
- [ ] 2. A CI job builds, signs, installs and removes the MSI on a fresh runner
- [ ] 3. The release builds, tests and publishes it beside the zip

## Definition of Done

Copied verbatim from the slice:

- [ ] The installer format is chosen on which one signs and uninstalls cleanly
      in CI, with the alternatives and the reason in this milestone's decisions
      file
- [ ] The release workflow builds it; installing on a clean machine places the
      bundle where the DAW looks, and uninstalling removes it
- [ ] The contract check asserts the package step's position and inputs

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `guards` | `bash scripts/check-guards.sh` |

Locally, `guards` runs the WiX source test and the release contract. The real
build, sign, install and uninstall run only in the `package-windows` CI job,
on the draft PR opened in task 2; each task that changes what that job does
names the run in its evidence.

## Names used throughout

- `scripts/packaging/poly.wxs` — the WiX v5 package source.
- `scripts/packaging/poly-wxs.test.mjs` — task 1, a static test of it.
- `scripts/packaging/build-windows-msi.ps1 -Bundle -Version -Out` — task 2.
- `scripts/packaging/test-windows-msi.ps1 -Msi [-SelfSign]` — task 2.
- WiX `5.0.2`, installed with `dotnet tool install --tool-path`.
- UpgradeCode `CAC811BE-A55D-452F-A377-9442360217CD`; asset
  `poly-<tag>-windows-x64.msi`.

## Task 1 — The WiX source says what the installer does, and a test holds it there

**Files:** create `scripts/packaging/poly.wxs`,
`scripts/packaging/poly-wxs.test.mjs`; modify `scripts/check-guards.sh`,
`scripts/README.md`.

1. Write the test first: read `poly.wxs` as text and assert the WiX v4
   namespace; `Package` with `Name="Poly"`, `Manufacturer="jk.digital"`,
   `Scope="perMachine"`, `Version="$(Version)"` and the UpgradeCode above;
   a `MajorUpgrade` with a `DowngradeErrorMessage`; `MediaTemplate
   EmbedCab="yes"` (one artifact); a `StandardDirectory
   Id="CommonFiles64Folder"` containing `Directory Name="VST3"` containing
   `Directory Name="poly_plugin.vst3"`; a `Files` element harvesting
   `$(BundleDir)` into that directory; and no `CustomAction` (nothing runs
   that Windows Installer would not undo). Run: red.
2. Write `poly.wxs` to satisfy it.
3. Run the test (green); wire it into `check-guards.sh` beside the macOS
   package test; add `poly.wxs` and the test to the `## Packaging` section of
   `scripts/README.md` with their `packaging/` prefix.
4. `guards`, `format`. Commit with `Rows:` empty.

## Task 2 — A CI job builds, signs, installs and removes the MSI on a fresh runner

**Files:** create `scripts/packaging/build-windows-msi.ps1`,
`scripts/packaging/test-windows-msi.ps1`; modify `.github/workflows/ci.yml`,
`scripts/README.md`; push and open the draft PR.

1. `build-windows-msi.ps1` (`$ErrorActionPreference = 'Stop'`): fail with a
   reason if the bundle or version is missing; install WiX 5.0.2 into a
   tool directory if `wix` is absent; `wix build poly.wxs -arch x64
   -d Version=<v> -d BundleDir=<bundle> -o <out>`.
2. `test-windows-msi.ps1 -Msi <path> [-SelfSign]`, printing each check:
   - refuse (exit 2) if `C:\Program Files\Common Files\VST3\poly_plugin.vst3`
     already exists;
   - with `-SelfSign`: create a code-signing certificate with
     `New-SelfSignedCertificate -Type CodeSigningCert` in `Cert:\CurrentUser\My`,
     export it and import it into `LocalMachine\Root` and
     `LocalMachine\TrustedPublisher`, then `signtool sign /fd SHA256 /sha1
     <thumbprint> <msi>` once and `signtool verify /pa <msi>` — the one-step
     sign OS37 asks for;
   - snapshot whether the VST3 folder exists and its listing;
   - `msiexec /i <msi> /qn /l*v install.log` (wait, check exit 0); assert
     `…\VST3\poly_plugin.vst3\Contents\x86_64-win\poly_plugin.vst3` exists and
     an Uninstall registry entry with `DisplayName` "Poly" and `Publisher`
     "jk.digital" exists (what Apps & features lists);
   - `msiexec /x <msi> /qn /l*v uninstall.log`; assert the bundle and the
     registry entry are gone and the VST3 folder is as it was (absent if it
     was absent, the same listing if not);
   - exit non-zero naming any mismatch.
3. Add job `package-windows` to `ci.yml`: `needs: build`,
   `runs-on: windows-2022`, PowerShell; download `plugin-windows-2022`; read
   the version from `CMakeLists.txt`; build; test with `-SelfSign`; upload
   the MSI and both logs as artifact `package-windows` (7 days). Add it to
   `ci-complete`'s `needs` and result check.
4. Add both scripts to `scripts/README.md`. Run `guards`, `format`,
   `check-workflow-hygiene`.
5. Commit, push the branch, open a **draft** PR titled
   "M008/S01 — An installer is built and tested unsigned", and watch
   `package-windows`. If it fails on WiX authoring or the scripts, fix it in
   this task with a further commit, re-push, and repeat until green — each
   failure and fix noted in the evidence. A failure outside this slice's
   files is a halt.
6. Evidence names the green run and quotes its checks. Commit with `Rows:`
   empty (the run URL lands in the evidence of this or the next commit).

## Task 3 — The release builds, tests and publishes it beside the zip

**Files:** modify `.github/workflows/release.yml`,
`scripts/check-release-workflow.mjs`, `RELEASING.md`; the ledger; the
evidence file.

1. Contract assertions first, mirroring M007/S01's: a step `Build installer
   package (Windows)`, Windows-gated, running `build-windows-msi.ps1` with
   the located bundle, the tag's version and
   `poly-${env:GITHUB_REF_NAME}-${{ matrix.asset }}.msi`; it follows the
   Windows signing gate and the Windows zip and precedes upload; a step `Test
   installer package (Windows)` runs `test-windows-msi.ps1` on it (without
   `-SelfSign`: the release ships unsigned until S02) before upload; upload,
   `SHA256SUMS`, the attestation and the Release's files include `*.msi`.
   Run: red.
2. Edit `release.yml` to satisfy them; update its header and `RELEASING.md`.
3. Run the contract (green), `guards`, `format`; push, and confirm the PR's
   CI is still green.
4. Evidence; tick the DoD; set OS37 and the slice `done`; run
   `jk-standards ledger`. Commit with `Rows: OS37`; push.

## Self-review

- **DoD 1** (format chosen on signing and clean uninstall in CI; alternatives
  and reason recorded) — `M008-decisions.md`; task 2's job signs in one step
  and uninstalls cleanly on a real runner.
- **DoD 2** (the release builds it; install on a clean machine places the
  bundle where the DAW looks; uninstall removes it) — task 3 (release), task
  2 (the CI proof), and the release leg's own test.
- **DoD 3** (contract asserts the step's position and inputs) — task 3 step 1.
- **OS37** — "The decision is recorded with what was tried; the installer
  installs and uninstalls cleanly on a clean machine (evidence); the release
  publishes it beside the zip": decisions file, task 2's run, task 3.
- **Names** — as listed, used consistently.
- **Placeholders** — none; the UpgradeCode and the WiX version are fixed.
