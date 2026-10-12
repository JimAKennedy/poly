# M011 — design: a release is installed the way a musician installs it

Agreed with the owner on 2026-10-12, section by section. M011 in
[the ledger](ledger.md) is the delivery plan; this document is the design it
implements and the reasoning behind each choice.

## Why the hosted runners are not enough

Three workflows already install Poly on a machine nobody has used:

- `package-macos` and `package-windows` in `ci.yml` build the installer from
  the PR's plug-in and install and remove it on a fresh hosted runner
  (M007/S01, M008/S01).
- `release-verify.yml` downloads a published Release's **zips**, checks the
  digest and the provenance attestation, records the Gatekeeper and
  SmartScreen-side verdicts, and runs pluginval on the installed bundle
  (M006/S01).

None of them shows four things the owner asked for:

1. **What a musician meets on first run**, from the installer they download,
   not the zip and not a command-line install that Gatekeeper never assesses.
2. **That a DAW finds and loads Poly** after the installer runs. pluginval
   loading the bundle proves the folder is right, not that a host scans it.
3. **Upgrade and uninstall** on a machine with history: one release, then the
   next over it, then removal.
4. **That this happens on every release** without anyone remembering.

A hosted runner cannot hold a DAW licence, and has no history to upgrade
over. So the checks run in virtual machines the owner controls, restored to a
clean state for every run.

**A defect found while designing this.** `release-verify.yml` declares
`on: release: published` and its header says it runs "on every published
Release, so no one has to remember". It never has. `release.yml` publishes
with `GITHUB_TOKEN`, and an event a workflow raises with `GITHUB_TOKEN` starts
no other workflow — except `workflow_dispatch` and `repository_dispatch`.
`v0.2.0-rc.1` was published at 00:06:29 on 2026-09-30; release-verify's only
run is a manual dispatch at 00:07:42. Row OS49 fixes it with the same
mechanism this milestone needs.

## What one run does

A run takes two published Releases: `TAG`, under test, and `PREV`, the newest
earlier Release that ships installers. On each platform, in a VM restored to
clean:

1. **Fetch as a user would.** Download the installer from the Release, apply
   the attribute a browser applies — the Mark-of-the-Web (`Zone.Identifier`,
   ZoneId 3) on Windows, `com.apple.quarantine` on macOS — and check it
   against `SHA256SUMS` and the provenance attestation, as `release-verify.yml`
   already does for the zips.
2. **First-run verdict.** macOS: `spctl --assess --type install -vv`. Windows:
   `Get-AuthenticodeSignature`, and the zone. Both: a screenshot of the
   installer opening. The verdict is compared with what the Release says of
   itself (M006/S02's Signed line): an unsigned Release that is rejected
   passes; a Release that says it is signed and is rejected fails. The
   screenshot is evidence, never an assertion — SmartScreen's wording changes
   as reputation accrues.
3. **Install, and the DAW finds it.** Install for all users, start the host,
   let it rescan, and prove Poly is listed and loads on a track.
4. **Upgrade.** Restore, install `PREV`, install `TAG` over it: one bundle,
   carrying `TAG`'s version, and the host still loads it.
5. **Uninstall.** Remove it, and prove the plug-in folder is as it was and the
   host's rescan no longer lists Poly.

Until two published Releases ship installers, step 4 reports
`SKIP: no earlier release ships an installer` — never a pass.

The install and remove assertions already exist in
`scripts/packaging/test-windows-msi.ps1` and `test-macos-pkg.sh`. Each gains a
phase switch (`-Phase install|uninstall`, `--phase install|remove`) so the
halves run separately; with no switch each runs both, so CI's packaging jobs
are unchanged. Their refusal to run where Poly is already installed is
exactly right inside a guest.

Each platform uploads one artifact: the log in the existing scripts'
`PASS`/`FAIL` style, the screenshots, and a `summary.json` naming the tag, the
OS version, the host version and each phase's result.

## Hosts

**Windows: Cubase.** The host Poly supports and the nightly drives.
**macOS: REAPER.** The owner's Mac keeps its Cubase seat; REAPER's evaluation
is the full product and needs no activation. A USD 60 discounted licence
covers standing use — one key, any number of computers, one at a time.

Steinberg Licensing allows one licence on up to three computers at once, and
the owner holds two licences, so the Windows guest takes an existing seat.
Steinberg says nothing public about virtual machines; whether an activation
survives a checkpoint restore is the first thing M011/S01 measures. If it does
not, the choice between a Cubase Elements seat and no Windows DAW check comes
back to the owner.

## Windows: a Hyper-V VM on JIMW1

JIMW1 is the `cubase`-labelled runner. Installing the MSI on it directly is
ruled out: the MSI is per-machine and needs administrator rights the runner
account no longer has, and it would put a second Poly bundle on a box whose
third invariant is exactly one (`docs/windows-test-runner-setup.md`).

**The guest, `poly-install-vm`**, built once from a runbook,
`docs/windows-install-vm-setup.md`: Windows 11 Pro x64, Generation 2, 8 GB of
memory, a 100 GB dynamic disk. The same Cubase version as the host, so the
nightly's `tests/cubase/fixtures/poly-4bar.cpr` opens in it; activated,
preferences seeded, Safe Mode cleared. PowerShell 7 and the WebView2 Runtime;
no build tools, no Node, no Poly. A local administrator `polytest` that logs
on automatically, with the lock screen and sleep off — invariant 4 applied to
the guest. One checkpoint, `clean`, taken shut down.

**The host side.** `polyci` joins *Hyper-V Administrators* — enough to restore
and start VMs without returning to `Administrators` — and
`check-runner-posture.ps1` asserts the membership alongside the
non-elevation it already asserts. Jobs carry `[self-hosted, cubase, hyperv]`;
JIMW1 has one runner, so a verification run and the nightly queue and never
overlap. `scripts/install-verify/windows/run-vm.ps1` restores `clean`, starts
the guest, waits for PowerShell Direct, copies in the installer, the scripts
and the fixture, runs each phase, copies the results out, and turns the guest
off without saving.

**Invariant 2 inside the guest.** PowerShell Direct is a non-interactive
session and cannot start a Cubase that presents a UI. A logon scheduled task
in `polytest`'s session starts Cubase; the host waits on a status file it
writes.

**Phases.** The fixture opens through the existing
`scripts/cubase/start-session.ps1`, which waits for the editor's CDP port: the
port appearing means Cubase found Poly, loaded it on its track and
materialised its editor. `vst3plugins.xml` must list the bundle at its
`Common Files` path. The first-run screenshot is taken by the host through
Hyper-V's `Msvm_VideoHead` thumbnail call while the guest's logon task has the
MSI open as a double-click would.

**Cost.** About 15 to 20 minutes a run. The guest needs a Windows 11 Pro
licence; an evaluation image would expire.

## macOS: disposable Tart VMs on the owner's Mac

The owner has one Mac, the development machine. A second user account does
not give a clean machine — an all-users install writes the shared
`/Library` — and registering the Mac itself as a runner would run workflow
code beside the owner's keys and the development copy `build.sh` deploys.

**The golden image, `poly-install-golden`**, built once from a runbook,
`docs/macos-install-vm-setup.md`: Cirrus Labs' `macos-tahoe-vanilla`, macOS
26 to match the host (a guest cannot be newer than its host), which logs on
automatically. REAPER 7.82, Universal build, first-run dialogs dismissed,
licence entered once bought. Screen Recording granted by hand to the runner's
shell, without which `screencapture` returns the wallpaper and which a script
cannot grant. No Poly, no build tools, no keys, no shared folders.

**The launcher**, `scripts/install-verify/macos/launcher.sh`, is the only thing
that runs on the host, as a LaunchAgent while the owner is logged in. It
requests a single-job runner configuration with the owner's existing `gh`
login (the generate-jitconfig API, so no new secret), clones the golden image
(`tart clone`, copy-on-write, instant), runs it headless, starts the runner
inside it over SSH, waits for the one job to finish, then stops and deletes
the clone and loops. One idle clone waits; never more than two run, Apple's
licence limit. Labels: `[self-hosted, macOS, ARM64, tart, install-verify]`.
Workflow code only ever runs in a clone that is deleted after one job.

**Phases.** `curl` the `.pkg`, then set `com.apple.quarantine` with `xattr -w`,
since `curl` does not. `open` the package for Installer's Gatekeeper dialog and
`screencapture` it. After the install half of `test-macos-pkg.sh`, delete
`reaper-vstplugins_arm64.ini` so REAPER genuinely rescans, and start REAPER
with a ReaScript on its command line that adds Poly to a new track with
`TrackFX_AddByName`, writes the result, and quits. A valid effect index means
found and loaded. Upgrade checks the bundle's `Info.plist` version.

**Cost.** About 10 minutes a run; about 50 GB of disk for the golden image.

## The workflow

`.github/workflows/installer-verify.yml`, triggered by `workflow_dispatch`
alone, with `tag` (required) and `prev` (optional). No `push`, no
`pull_request`, no `release` trigger: the repository is public, and a
self-hosted runner must never run a fork's code.

`release.yml` ends with a step that dispatches both `installer-verify.yml` and
`release-verify.yml` for the new tag, with `actions: write` granted to that
step's job alone. The release does not wait on either.

Jobs:

1. `resolve`, hosted: the tag is a published Release with the expected
   installer assets; `prev` is chosen when not given.
2. `windows` and `macos`, self-hosted, independent of each other.
3. `report`, hosted, the only job with `issues: write`: opens or updates an
   issue labelled `install-verify-failure`.

A platform whose runner never picked the job up — the Mac was asleep, and
GitHub cancels a job queued for 24 hours — is reported as *not run*, never as
a pass and never as an installer defect.

`tag` and `prev` are validated against the tag pattern in `resolve` and read
from `env:`, never interpolated into a `run:` body. Default permissions are
`contents: read`.

**Guards.**

- `scripts/check-workflow-hygiene.mjs`: every job on a `self-hosted` runner is
  in a workflow with no `pull_request`, `pull_request_target` or `push`
  trigger. This covers `cubase-nightly.yml`, which a comment and the runbook
  protect today.
- `scripts/check-release-workflow.mjs`: the dispatch step follows publishing,
  and only its job holds `actions: write`.
- `.jk/validations.yml` declares an `install-verify` token, exempt from the
  pre-push gate with its reason, so `doc-conformance-wiring` accepts it.

**Evidence.** Artifacts expire after 90 days. A slice that relies on a run
names the run and quotes its key lines, as `evidence/M008-S01.md` does.

## What waits on what

S01 measures four unknowns, and nothing is built on them before it answers:

1. Cubase stays activated across checkpoint restores without using another
   seat.
2. A Hyper-V Administrator who is not an administrator can use PowerShell
   Direct and capture the guest's screen.
3. REAPER runs a ReaScript given on its command line, unattended, in a guest
   with no audio device.
4. Screen Recording survives a Tart clone.

S02 (Windows) and S03 (macOS) follow S01 and are independent of each other.
S04 (the workflow) follows both, and is done when one dispatched run against a
published Release that ships installers is green on both platforms. No such
Release exists yet: cutting one — `v0.2.0-rc.2`, unsigned, which M006/S02
already lets it declare — is the owner's decision, and the slice stops and
asks rather than tagging.

M007/S02 and M008/S02 depend on M011/S04: their definitions of done ask for a
clean machine and no warning, and this milestone is how that is shown
repeatably rather than once.

## Out of scope

- Asserting SmartScreen's wording. It is captured, not checked.
- Hosts beyond Cubase on Windows and REAPER on macOS — M010 measures those.
- Intel Macs.
- A hosted fallback for the DAW checks: no hosted runner can hold either
  licence.
