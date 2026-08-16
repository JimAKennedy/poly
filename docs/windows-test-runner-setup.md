---
class: gated
---

# Windows Test-Runner Setup — Playwright + Poly Build + Cubase-in-the-Loop

> **Status (2026-08-01):** Setup runbook (human-run). Follow top to bottom on a
> **fresh Windows install**. This is the acceptance procedure for M042 S07
> ("Self-Hosted Runner and Nightly Workflow") — the machine it produces is the
> `cubase`-labelled self-hosted GitHub Actions runner that unblocks S07→S08→S09.
> **Lifecycle:** Provisioning artifact. Companion to `docs/testing-strategy.md`
> §3.2 + Appendix A (the hardware/hardening spec this operationalises).

**The one rule that dominates everything:** Steinberg Licensing binds Cubase
activations to a machine fingerprint. Set this box up **once**, image it when
it's golden (Part 8), and never re-provision. Every clean re-install burns a
Cubase activation.

**What this machine does — three jobs, in dependency order:**
1. **Build** the plugin from source with MSVC (so the runner tests *its own*
   fresh binary, not a downloaded artifact).
2. **Run the test pyramid** L1–L3: ctest, VST3 validator, pluginval, and the
   Playwright webui suite — the same gates CI runs, so a green nightly here
   means the whole below-Cubase pyramid passes on Windows too.
3. **Run L4 Cubase-in-the-loop**: launch Cubase with a fixture project, drive
   transport over virtual MIDI, capture with `poly_midi_probe`, and (Windows
   flagship only) attach Playwright over CDP to the WebView2 editor **inside**
   Cubase.

Work through the numbered parts. Each `☐` is a checkable step; each part ends
with a **Verify** you must see pass before moving on.

---

## Part 0 — Before you touch the keyboard (decisions & inventory)

- ☐ **Confirm the hardware is x64 physical (or Intel-Mac/Boot Camp), not a VM on
  Apple Silicon.** Cubase is x64-only and loopMIDI is an x64 kernel driver that
  cannot be emulated on ARM (testing-strategy Appendix A.1). If this box is an
  ARM machine running Windows-on-ARM, **stop** — it cannot run this suite.
- ☐ **Windows 11 (x64), fully activated.** Windows 10 works but 11 is what CI's
  `windows-2022` image is closest to; prefer 11.
- ☐ You have: a Steinberg account with a Cubase 12+ licence seat available, the
  GitHub repo admin rights (to register a self-hosted runner), and physical/VNC
  access to this box.
- ☐ A **dummy HDMI plug** (headless display emulator) on hand, OR a monitor that
  stays connected. The interactive desktop must never go away (Part 7).

**Verify:** `winver` shows Windows 11 x64; `systeminfo | findstr /C:"System Type"`
reports `x64-based PC`.

---

## Part 1 — Base OS setup

Do these first; several later steps assume an admin PowerShell and a package
manager.

### 1a — Create the dedicated local admin user `polyci`

You need a **local** account (not a Microsoft account) so auto-logon (Part 7) is
deterministic and no cloud password reset can lock the box out mid-nightly.
Windows 11 hides this path hard — it assumes you want a Microsoft account. There
are two reliable ways in; the PowerShell method is fastest and least fiddly.

**Method A — PowerShell (recommended, avoids the Settings-app runaround).**
From an **elevated PowerShell** (Win → type `powershell` → *Run as
administrator*) on whatever account did the initial Windows install:

```powershell
# Create the local user. You'll be prompted for a password (store it in your
# password manager). Leave the prompt blank ONLY if you accept a passwordless
# account — not recommended even on LAN.
$pw = Read-Host -AsSecureString "Password for polyci"
New-LocalUser -Name "polyci" -Password $pw -FullName "Poly CI Runner" `
  -Description "Self-hosted Cubase test runner" -PasswordNeverExpires

# Make it a local administrator (needed for VS Build Tools, driver installs,
# Task Scheduler, and auto-logon config).
Add-LocalGroupMember -Group "Administrators" -Member "polyci"
```

Verify the account exists and is an admin:
```powershell
Get-LocalUser polyci
Get-LocalGroupMember -Group "Administrators" | Select-String polyci
```

> If `PasswordNeverExpires` errors on your edition, drop that flag and instead
> run afterwards: `Set-LocalUser -Name polyci -PasswordNeverExpires $true`.
> An expiring password will silently break auto-logon weeks later — make sure
> it's set to never expire.

**Method B — Settings app (if you prefer GUI, note the trap).** Settings →
*Accounts* → *Other users* → *Add account*. When it demands an email:
1. Click **"I don't have this person's sign-in information."**
2. On the next screen click **"Add a user without a Microsoft account."**
3. Enter username `polyci` and a password, and answer the security questions.
4. The account is created as a **Standard** user. You must then promote it:
   *Other users* → click `polyci` → *Change account type* → **Administrator**.

Both methods land you the same place: a local admin named `polyci`.

- ☐ **`polyci` exists as a LOCAL account** (not linked to a Microsoft account).
- ☐ **`polyci` is in the Administrators group.**
- ☐ **Its password is set to never expire** and stored in your password manager.

### 1b — Switch to `polyci` and set up its shell

- ☐ **Sign out** and **log in as `polyci`.** From here on, everything runs as
  this user — the toolchain, the repo checkout, and the runner all live in its
  profile, and auto-logon (Part 7) targets it.
- ☐ **Open an elevated PowerShell** (Win → type `powershell` → *Run as
  administrator*). Keep one open throughout.
- ☐ Set execution policy so scripts run:
  ```powershell
  Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy RemoteSigned -Force
  ```
- ☐ **Install winget** if missing (it ships with modern Win11; if `winget
  --version` fails, install "App Installer" from the Microsoft Store).

**Verify:** `winget --version` prints a version.

---

## Part 2 — Build toolchain (matches CI's `windows-2022` leg exactly)

The runner builds the plugin itself, so it needs the same toolchain CI uses:
**Visual Studio 2022 (MSVC v143), CMake, Ninja, Git.** Versions are pinned to
what `.github/workflows/ci.yml` runs.

- ☐ **Git:**
  ```powershell
  winget install --id Git.Git -e --source winget
  ```
- ☐ **Visual Studio 2022 Build Tools** with the C++ workload (this is the MSVC
  compiler CI uses via `-G "Visual Studio 17 2022"`):
  ```powershell
  winget install --id Microsoft.VisualStudio.2022.BuildTools -e --source winget `
    --override "--quiet --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
  ```
  (If you prefer the full IDE for debugging fixture `.cpr` issues later, install
  `Microsoft.VisualStudio.2022.Community` with the same `--add` instead.)
- ☐ **CMake** (3.14+ required; get current):
  ```powershell
  winget install --id Kitware.CMake -e --source winget
  ```
- ☐ **Ninja** (the engine/WASM legs use `-G Ninja`; harmless to have both):
  ```powershell
  winget install --id Ninja-build.Ninja -e --source winget
  ```
- ☐ **PowerShell 7 (`pwsh`)** — **required by the Cubase nightly workflow.**
  Every `run:` step in `.github/workflows/cubase-nightly.yml` (and every
  `scripts/cubase/*.ps1`) uses `shell: pwsh`, i.e. PowerShell **Core** 7+, which
  a fresh Windows box does **not** ship (it only has Windows PowerShell 5.1,
  `powershell.exe`). GitHub-hosted `windows-2022` images pre-install `pwsh`; a
  self-hosted runner does not — so the nightly fails on the first script step
  with `pwsh: command not found` until this is installed (ISSUE-003 in
  `docs/windows-test-runner-setup-issues.md`):
  ```powershell
  winget install --id Microsoft.PowerShell -e --source winget
  ```
  After installing, **restart the runner's logon task (or reboot)** so the
  runner process picks up the new PATH — an already-running runner won't see
  `pwsh` until it restarts.
- ☐ **Microsoft Edge WebView2 Runtime** (Poly's UI is a WebView2 web view;
  without the runtime the plugin opens to a blank window with no error):
  ```powershell
  winget install --id Microsoft.EdgeWebView2Runtime -e --source winget
  ```
  Modern Windows 11 usually has this via Windows Update, but fresh installs
  and stripped images may not. Verify with:
  ```powershell
  reg query "HKLM\SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}" /v pv
  ```
  If the key exists and `pv` has a version string, the runtime is installed.
- ☐ **Close and reopen PowerShell** so PATH updates land.

**Verify — all six must succeed:**
```powershell
git --version; cmake --version; ninja --version
# PowerShell 7 (the workflow's shell — must print 7.x, NOT 5.1):
pwsh --version
# MSVC: from a "Developer PowerShell for VS 2022" (Start menu), run:
cl
# WebView2 Runtime:
reg query "HKLM\SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}" /v pv
```

---

## Part 3 — Node + Playwright (the L2 webui suite)

CI runs the webui Playwright suite on **Node 24** with `@playwright/test`
**pinned to 1.62.0** (see `webui/package.json`). Match both — Playwright is
strict about browser/package version skew.

- ☐ **Node 24 LTS:**
  ```powershell
  winget install --id OpenJS.NodeJS.LTS -e --source winget
  ```
  If winget's LTS is not on the 24 line yet, install `OpenJS.NodeJS` and pin
  with `nvm-windows` to be safe. Confirm `node --version` starts with `v24`.
- ☐ Playwright browsers are installed later (Part 6) from inside the checked-out
  repo, so the version matches `package-lock.json` exactly. Don't `npm install
  -g playwright` — it'll drift from the pinned 1.62.0.

**Verify:** `node --version` → `v24.x`; `npm --version` prints.

---

## Part 4 — Cubase + audio/MIDI plumbing (the L4 substrate)

This is the licensing-sensitive part. Do it deliberately.

- ☐ **Install Steinberg Download Assistant**, sign in, install **Cubase 12+**
  (match the version you'll commit fixtures for — Cubase rewrites `.cpr` per
  major version, so pick one and pin it).
- ☐ **Activate Cubase** via Steinberg Licensing. Launch it once; confirm it
  opens to an empty project. **This activation is precious — see Part 8.**
- ☐ **Audio endpoint for ASIO:** Cubase's *Generic Low Latency ASIO* driver
  needs a real output device. Onboard audio is fine; if the box is truly
  headless with no audio codec, install **VB-CABLE** (virtual audio device) so
  ASIO has an endpoint.
- ☐ **Virtual MIDI loopback — loopMIDI (x64):** download from Tobias Erichsen's
  site, install, and create **one port named `poly-test`**. The S08 Python +
  `mido` driver sends transport/parameter MIDI into this port; Cubase's MIDI
  Remote script listens on it.

  > **Windows MIDI port naming:** The Windows MIDI API (via rtmidi) appends a
  > device index to every port name — so loopMIDI's `poly-test` appears as
  > `poly-test 0` (input) and `poly-test 1` (output) in `mido.get_output_names()`.
  > This is standard Windows behaviour, not a misconfiguration. Scripts that
  > match the port should use a **prefix match** (e.g.
  > `[n for n in mido.get_output_names() if n.startswith('poly-test')]`) rather
  > than an exact string comparison. The `MIDI_PORT_NAME` env var (default
  > `poly-test`) can be set to the full suffixed name if needed — e.g.
  > `MIDI_PORT_NAME=poly-test 1` for output.
- ☐ **Python 3 + mido (the transport driver, S08):**
  ```powershell
  winget install --id Python.Python.3.12 -e --source winget
  # reopen PowerShell, then:
  python -m pip install --upgrade pip
  python -m pip install mido python-rtmidi
  ```
  > **`python3` on Windows:** Windows ships a `python3.exe` stub in
  > `WindowsApps` that redirects to the Microsoft Store instead of running
  > Python. The real interpreter is `python.exe`. The repo's test files
  > handle this with platform detection; if you write new scripts that spawn
  > Python, use `python` on Windows, not `python3`.

**Verify:** Cubase launches to an empty project without a licensing prompt;
loopMIDI shows the `poly-test` port; `python -c "import mido; print(mido.get_output_names())"`
lists a port starting with `poly-test` (e.g. `poly-test 1` — see the note
above about Windows MIDI port naming).

---

## Part 5 — Cubase preferences seeding (headless-friendly)

Cubase's defaults fight unattended automation. Fix them once.

- ☐ **Disable the Hub / Steinberg Hub** (Preferences → General → *Show Hub when
  Cubase starts* off) so launch goes straight to a project.
- ☐ **Disable all first-run / "what's new" dialogs.**
- ☐ Set the audio driver to *Generic Low Latency ASIO* (or your VB-CABLE
  device) and confirm it engages without a dialog.
- ☐ Leave room for the **MIDI Remote script** — it's authored and installed in
  S08, not now. Just note where Cubase's MIDI Remote scripts directory is
  (`Documents\Steinberg\Cubase\MIDI Remote\Driver Scripts\Local`).
- ☐ **Back up the whole prefs directory** (`%APPDATA%\Steinberg\Cubase <ver>_64`)
  to a zip on disk — the golden image (Part 8) captures it, but a separate zip
  makes prefs re-seeding after a Cubase update a 1-minute restore.

**Verify:** Kill Cubase, relaunch — it opens to an empty project with no Hub, no
dialogs, ASIO engaged.

---

## Part 6 — Clone the repo & prove the full pyramid builds locally

Before wiring the runner, prove this machine can do everything by hand. If any
step here fails, the nightly will fail — fix it now, not in CI.

- ☐ Clone (SSH key must be on this box and added to GitHub, or use HTTPS + a PAT):
  ```powershell
  cd C:\
  git clone git@github.com:JimAKennedy/poly.git
  cd C:\poly
  ```
- ☐ **Configure + build the plugin (MSVC, validator ON — same as CI):**
  ```powershell
  cmake -S . -B build -G "Visual Studio 17 2022" -DSMTG_RUN_VST_VALIDATOR=ON
  cmake --build build --config Release --parallel
  ```
  The VST3 SDK and GoogleTest are fetched automatically via FetchContent; the
  first configure downloads them (slow once, cached after).
- ☐ **Run the C++ / host test suite (L1 + L3 golden MIDI + bridge contract):**
  ```powershell
  ctest --test-dir build --build-config Release --output-on-failure
  ```
- ☐ **Install pluginval and run it at CI strictness (L3 conformance):** download
  `pluginval_Windows.zip` from Tracktion's latest release, unzip to
  `C:\tools\pluginval\`, then:
  ```powershell
  C:\tools\pluginval\pluginval.exe --strictness-level 8 --skip-gui-tests `
    build\VST3\Release\poly_plugin.vst3
  ```
- ☐ **Run the Playwright webui suite (L2) — from `webui\`, matching ci.yml:**
  ```powershell
  cd C:\poly\webui
  npm ci
  npx playwright install --with-deps chromium
  npx playwright test
  cd C:\poly
  ```
  (Run Playwright from `webui\`, never repo root — the root-relative form
  installs a mismatched Playwright and fails with "test.describe() not expected
  here".)
- ☐ **Install the plugin so Cubase can load it.** Copy the built bundle to the
  **per-user** VST3 folder — the same `POLY_VST3_INSTALL_DIR` the nightly
  installs to, and the one a non-elevated runner can write (Part 12):
  ```powershell
  Copy-Item -Recurse -Force build\VST3\Release\poly_plugin.vst3 `
    "$env:LOCALAPPDATA\Programs\Common\VST3\poly_plugin.vst3"
  ```
  > **Only ONE copy of `poly_plugin.vst3` may exist on the box.** A leftover
  > stale bundle in a *different* VST3 location (e.g. the machine-wide
  > `C:\Program Files\Common Files\VST3` or an old manual copy) makes Cubase
  > load the wrong binary — the exact trap behind the blank-window incident
  > (ISSUE-001). Before rescanning, confirm there is no second copy:
  > ```powershell
  > Get-ChildItem -Recurse -Filter poly_plugin.vst3 `
  >   "C:\Program Files\Common Files\VST3", "$env:LOCALAPPDATA\Programs\Common\VST3" `
  >   -ErrorAction SilentlyContinue | Select-Object FullName
  > ```
  > If more than one path prints, delete the stale one, then clear Cubase's VST3
  > cache so it re-scans from scratch: delete
  > `%APPDATA%\Steinberg\Cubase <ver>_64\Vst3Cache.xml` (or the
  > `VST3 Cache.xml` variant) before relaunching.
- ☐ In Cubase, rescan plugins; confirm **Poly** appears and instantiates on an
  instrument track, opening the **WebUI editor** — a live web view showing the
  lane strips and preset picker, **not** a blank white window.
  > **If the editor is blank/white:** that was ISSUE-001 (a WebView2 async-init
  > race + a missing `WS_POPUP → WS_CHILD` reparenting style), **fixed in the
  > plugin** as of the `fix/windows-test-portability` work (merged via PR #168).
  > A blank editor now means one of: (a) the **WebView2 Runtime is missing**
  > (re-check Part 2's `reg query`), (b) you loaded a **stale bundle** (see the
  > single-copy note above), or (c) the build predates the fix — rebuild from a
  > current `main`.

**Verify:** ctest all-green; pluginval exits 0; Playwright all-green; Poly loads
in Cubase and its WebUI editor renders the lane strips (not a blank window),
with exactly one `poly_plugin.vst3` on the machine.

> **Presets note (ISSUE-002, by design):** Cubase's *own* preset browser (the
> DAW dropdown at the top of the plugin header) will show **no** presets for
> Poly — Poly does not implement the VST3 `ProgramList` API. Poly's factory
> presets are applied from **Poly's own web-UI preset picker**, not the DAW's.
> This is expected, not a fault; see `docs/cubase-workflow.md` for the user-facing
> note.

---

## Part 7 — Interactive-desktop hardening (where self-hosted Cubase runners die)

Cubase, UI automation, and CDP **all require a live interactive desktop**. A
Windows service has none; a disconnected RDP session tears one down. Configure
for a permanent console session.

- ☐ **Auto-logon** `polyci` into a console session. Use Sysinternals `Autologon`
  (simplest, stores the credential in LSA secrets) or `netplwiz`. Reboot and
  confirm it lands on the desktop with no password prompt.
- ☐ **BIOS/UEFI: power-on after power loss** (so a power blip auto-recovers the
  box). Set in firmware, not Windows.
- ☐ **Keep a display alive:** plug in the **dummy HDMI** plug (or leave a monitor
  attached). Without it, Windows may drop to a headless GPU state and Cubase /
  WebView2 rendering breaks.
- ☐ **Remote access via VNC, not RDP.** Install a VNC server (e.g. TightVNC/
  UltraVNC) — VNC attaches to the console session without destroying it. If you
  *must* use RDP, apply the `tscon`-redirect-to-console trick on disconnect so
  the interactive session survives.
- ☐ **Disable sleep, hibernate, and display-off:**
  ```powershell
  powercfg /change standby-timeout-ac 0
  powercfg /change hibernate-timeout-ac 0
  powercfg /change monitor-timeout-ac 0
  powercfg /hibernate off
  ```
- ☐ **Windows Update → controlled window.** Set active hours / a weekly
  maintenance window with a controlled reboot; don't let it reboot mid-nightly.
  **Pin the Cubase version** — treat any Cubase update as a deliberate
  fixture-upgrade event (testing-strategy §6), never automatic.

**Verify:** Reboot the machine with only power connected (no keyboard/monitor if
using the dummy plug + VNC). It comes back, auto-logs-in, and you can VNC to a
live desktop where Cubase launches.

---

## Part 8 — Golden disk image (disaster recovery, BEFORE registering the runner)

Do this the moment Parts 1–7 verify. The image lets you restore **this same
machine** without re-activating Cubase.

- ☐ Take a **full disk image** (Macrium Reflect, Clonezilla, or a Windows system
  image) to external storage.
- ☐ Label it with the date, Cubase version, and the note: *"Restore only onto the
  SAME hardware — restoring onto different hardware costs a Steinberg
  activation."*

**Verify:** The image file exists on external storage and its size looks sane
(tens of GB).

---

## Part 9 — Register the self-hosted GitHub Actions runner

Now wire the machine into CI. The nightly workflow
(`.github/workflows/cubase-nightly.yml`) targets the `cubase` label and calls
the launch/quit machinery in `scripts/cubase/` (see that directory's
`README.md`).

- ☐ **Install the runner at `C:\actions-runner`.** Create the directory *first*
  and `cd` into it by absolute path before running anything GitHub gives you:
  ```powershell
  New-Item -ItemType Directory -Force C:\actions-runner
  Set-Location C:\actions-runner
  ```
  > **Do not paste GitHub's download snippet into an elevated shell as-is.** It
  > opens with `mkdir actions-runner; cd actions-runner` — *relative* paths. An
  > elevated PowerShell starts in `C:\Windows\System32`, so the snippet silently
  > installs the runner to `C:\Windows\System32\actions-runner`, builds repo code
  > inside a protected OS directory, and makes the whole tree admin-only. This
  > has happened on this box once already; if you find a runner there, follow
  > `docs/windows-runner-rehome-and-deelevate.md` to relocate it.
- ☐ In GitHub: **repo → Settings → Actions → Runners → New self-hosted runner →
  Windows x64.** GitHub shows a per-runner token and download commands.
- ☐ From `C:\actions-runner`, run the shown `config.cmd`. When prompted:
  - **Labels:** add `cubase` (the nightly workflow keys off this).
  - **Work folder:** default `_work` is fine.
- ☐ **Confirm the install path before going further** — a wrong path here is
  cheap to fix now and painful later:
  ```powershell
  (Get-Item C:\actions-runner\run.cmd).FullName   # expect C:\actions-runner\run.cmd
  Test-Path C:\Windows\System32\actions-runner    # expect False
  ```
- ☐ **Install the runner as a LOGON SCHEDULED TASK, not a Windows service.** The
  default `svc.cmd install` registers a *service* — which has **no interactive
  desktop** and will break Cubase/CDP. Instead:
  - Do **not** run `svc.cmd install`.
  - Create a **Task Scheduler** task that runs `run.cmd` **by absolute path**,
    triggered **At log on** of `polyci`, "Run only when user is logged on", and
    **not** "Stop if runs longer than…". (Auto-logon from Part 7 makes this fire
    on every boot.) Equivalent from an elevated PowerShell:
    ```powershell
    $a = New-ScheduledTaskAction -Execute "C:\actions-runner\run.cmd" `
                                 -WorkingDirectory "C:\actions-runner"
    $t = New-ScheduledTaskTrigger -AtLogOn -User "$env:COMPUTERNAME\polyci"
    $p = New-ScheduledTaskPrincipal -UserId "$env:COMPUTERNAME\polyci" `
                                    -LogonType Interactive -RunLevel Highest
    $s = New-ScheduledTaskSettingsSet -ExecutionTimeLimit ([TimeSpan]::Zero) `
                                      -AllowStartIfOnBatteries `
                                      -DontStopIfGoingOnBatteries
    Register-ScheduledTask -TaskName "GitHubActionsRunner" `
                           -Action $a -Trigger $t -Principal $p -Settings $s
    ```
    `-RunLevel Highest` is the *initial* posture only, because Part 1a made
    `polyci` a local admin. Once the box is stable, drop both the elevation and
    the admin membership — see
    `docs/windows-runner-rehome-and-deelevate.md` Part C.
- ☐ Reboot. Confirm the runner comes up **idle/listening** in the GitHub Runners
  page after auto-logon.

**Verify:** GitHub → Settings → Actions → Runners shows this runner **online**
with the `cubase` label; it is running as a logon task (visible in Task
Scheduler), not as a service; and its working directory is `C:\actions-runner`,
**not** anywhere under `C:\Windows`.

---

## Part 10 — Security posture (non-negotiable — the runner executes repo code)

A self-hosted runner runs whatever a workflow tells it to. Lock it down.

- ☐ **The nightly workflow (`.github/workflows/cubase-nightly.yml`) is gated to
  `schedule` + `workflow_dispatch` only — NEVER `pull_request`.** A
  `pull_request` trigger would let any fork's PR run arbitrary code on your
  licensed box. (This is encoded in the workflow YAML — a `pull_request` trigger
  is asserted-absent in the T01 verify; the machine's job is to not be reachable
  in the first place.)
- ☐ **LAN-only, no inbound exposure.** No port-forwarding, no public IP. The
  runner makes *outbound* long-poll connections to GitHub; it needs no inbound.
- ☐ **Repo-scoped runner** (registered against `JimAKennedy/poly`, done in
  Part 9), not org-wide — smaller blast radius.
- ☐ Confirm Windows Firewall has no inbound rule you added for VNC exposed to the
  internet; VNC should be LAN-only too.

**Verify:** From outside your LAN, the box is unreachable (no open inbound
ports). The runner still shows online in GitHub (outbound-only works).

---

## Part 11 — End-to-end smoke (S07 exit criterion)

The S07 "done" signal: *nightly triggers → runner picks up → Cubase launches and
quits cleanly.* Prove the machine can do that manually before the scheduled run.

- ☐ From the repo Actions tab, select **Cubase Nightly (L4)** and use
  **`workflow_dispatch` → Run workflow** against `main`.
- ☐ Watch: the `cubase`-labelled runner picks up the job; the job builds/installs
  Poly (MSVC), runs `ctest`, then the `scripts/cubase/` machinery kills stale
  Cubase, launches it (empty project in S07 — the fixture `.cpr` slots in at
  S08), waits for its main window, and quits it (graceful `CloseMainWindow` with
  a hard-kill fallback).
- ☐ Confirm the `cubase-nightly-artifacts` upload contains
  `cubase-run-status.jsonl` (one line per phase) and — on failure —
  `cubase-last-error.json`. Read the status JSONL to see which phase failed.

**Verify:** A manual `workflow_dispatch` run completes green on the `cubase`
runner, with Cubase having launched and quit without a hung process left behind.
This run is the **S07 exit-criterion evidence** — capture the run URL for the
slice's UAT record.

---

## Part 12 — WebView2 CDP enablement (S09 L4-web flagship)

The L4-web tier drives Poly's **actual plugin editor inside Cubase** with
Playwright, attaching over the Chrome DevTools Protocol (CDP). On Windows the
editor is hosted by WebView2 (via `choc`), which honors a CDP remote-debugging
port; macOS's WKWebView exposes no such port, so **this flow is Windows-only** —
the reason this box is the flagship.

### How CDP gets exposed

WebView2 reads `WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS` from its process
environment at startup. Set it to `--remote-debugging-port=9222` before Cubase
launches and every WebView2 the process spawns (including Poly's editor) listens
for CDP on that port.

The nightly does this for you: the e2e flow passes `-EnableCdp` to
`scripts/cubase/launch-cubase.ps1`, which sets the env var before
`Start-Process` so the launched Cubase inherits it. The S07/S08 flows leave
`-EnableCdp` off, so their launches are unaffected.

### The runner must NOT be elevated (the S09 dead end)

> **This is the load-bearing constraint of the whole L4-web tier.** If the
> `GitHubActionsRunner` logon task is ever re-registered with
> `-RunLevel Highest`, CDP silently stops working and the e2e fails with a bare
> "CDP port never came up".

WebView2 **deliberately discards** browser flags delivered "via the local device
environment" — both `WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS` and the
`Software\Policies\Microsoft\Edge\WebView2` registry keys — whenever the host
app is running elevated. From Microsoft's
[WebView2 browser flags](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/webview-features-flags)
reference:

> Elevated apps ignore flags that are set via the local device environment.
> Elevated apps honor flags that are set via code.

This is the entire explanation for the seven-round S09 dead end (runs #40–#47).
The runner's logon task ran with `-RunLevel Highest`, so every job step — and
therefore Cubase — ran at high integrity, and **0 of 18** `msedgewebview2.exe`
children ever carried `--remote-debugging-port`, in every round, no matter how
the flag was delivered. A Cubase launched by hand from an ordinary
(medium-integrity) VNC shell exposed the port every single time. Sessions,
window stations, desktop lock state, editor focus and WebView2 process reuse
were all investigated and all ruled out — the topology diagnostic reported
session 1, console matched, input desktop reachable, editor frame and choc host
both PRESENT on every failing run.

The documented escape hatch — passing `AdditionalBrowserArguments` through
`ICoreWebView2EnvironmentOptions` in code, which *is* honored when elevated —
is a dead end here too: WebView2 Runtime **150 and later** has an open
regression making the DevTools endpoint unreachable for elevated hosts however
the flag arrives
([WebView2Feedback#5640](https://github.com/MicrosoftEdge/WebView2Feedback/issues/5640)).
This box runs 151.x. **A non-elevated Cubase is the only supported
configuration.**

Consequences, all already in place:

- The logon task is registered `-RunLevel Limited`
  (`docs/windows-runner-rehome-and-deelevate.md` Part C3).
- The nightly installs the plugin to the **per-user** VST3 folder
  (`%LOCALAPPDATA%\Programs\Common\VST3`), which Cubase scans alongside the
  machine-wide one and which a non-admin can write. Nothing Poly-named may be
  left in `C:\Program Files\Common Files\VST3` — the duplicate VST3 class ID
  resolves to whichever copy Cubase scanned first, so a stale bundle there means
  the run silently tests an old build. The install step fails the job if it
  finds one.
- `launch-cubase.ps1 -EnableCdp` and `launch-manual-cdp.ps1` both refuse to run
  elevated, and `diagnose-editor-window.ps1` records the elevation state in
  `editor-window-topology.txt`.

**Verify** (from a normal, non-elevated PowerShell — an elevated one reports
only itself):

```powershell
(Get-ScheduledTask GitHubActionsRunner).Principal.RunLevel   # expect Limited
```

- ☐ Confirm the **WebView2 Runtime** is installed (Part 2 already covers this).
  CDP attach fails loud if WebView2 isn't present.
- ☐ No manual env setup is needed for the nightly — the launch script owns it.
  To exercise CDP by hand for diagnosis, in the shell that will start Cubase:
  ```powershell
  $env:WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS = "--remote-debugging-port=9222"
  # then launch Cubase from that same shell and open the Poly editor
  # verify the port is listening (use 127.0.0.1, NOT localhost — WebView2
  # binds IPv4 only, and `localhost` resolves to IPv6 ::1 first):
  Test-NetConnection -ComputerName 127.0.0.1 -Port 9222
  ```

### The editor must be materialized — but focus is NOT required

The Poly editor window has to exist for its WebView2 to be created at all, so
the fixture (#212) is saved with the editor already open and it materializes on
project load.

Focus, however, is irrelevant: the runner diagnostic confirmed the CDP port is
listening with Cubase in the **background** and the choc WebView2 host window
`Visible=False`. An earlier theory that the editor had to be foregrounded was
wrong, and `scripts/cubase/focus-editor-cdp.ps1` no longer forces foreground
despite its name — it only polls the OS TCP table until `127.0.0.1:<port>` is
listening and fails loud if it never comes up.

Checking by hand: use `127.0.0.1`, **not** `localhost` — WebView2 binds IPv4
only and `localhost` resolves to `::1` first, which reads as closed.

```powershell
Get-NetTCPConnection -State Listen | Where-Object LocalPort -eq 9222
Invoke-RestMethod http://127.0.0.1:9222/json/version
```

### Security

The remote-debugging port binds to **localhost only** — it is not reachable off
the box. The runner's network posture (Part 10: LAN-gated, no inbound exposure)
is what keeps a local CDP port from becoming a remote-code surface. Do not add a
port forward or bind the port to `0.0.0.0`; a CDP endpoint is effectively remote
code execution against the browser, and it must stay local.

### S09 exit-criterion dispatch run

The S09 "done" signal: *Playwright attaches over CDP to the editor inside Cubase,
toggles a kick step, transport plays, and the probe JSONL shows the changed
pattern.* Prove it manually once, the same way Part 11 proves S07.

Prerequisites (owner-provisioned, R10): the S08 fixture `.cpr`, the loopMIDI
`poly-test` port, and the MIDI Remote script are all in place (Part 4 + the S08
recipe).

- ☐ **Verify the WebView2 Runtime is installed.** Cubase hosts Poly's editor via
  WebView2 (choc), which is what exposes CDP for the e2e to attach to. Check the
  Evergreen Runtime version from an elevated PowerShell:
  ```powershell
  $key = "HKLM:\SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}"
  (Get-ItemProperty -Path $key -Name pv -ErrorAction SilentlyContinue).pv
  ```
  A non-empty version string (e.g. `151.0.4129.78`) means it is installed. If
  empty, install it and re-check:
  `winget install --id Microsoft.EdgeWebView2Runtime --accept-source-agreements --accept-package-agreements`.
- ☐ Confirm the runner's logon task is **not** elevated — see *The runner must
  NOT be elevated* above. This is the single most common cause of a missing CDP
  port:
  ```powershell
  (Get-ScheduledTask GitHubActionsRunner).Principal.RunLevel   # expect Limited
  ```
- ☐ `POLY_FIXTURE_CPR` is already set in the workflow env (the S08 fixture
  landed), so the transport flow is always on. The S09 e2e flow is on by default
  too: the `cdp_port` dispatch input defaults to `9222` and scheduled runs always
  use `9222`. Clear the field on a dispatch to skip the e2e and run only the
  S07/S08 launch/transport smoke.
- ☐ From the Actions tab, run **Cubase Nightly (L4)** via
  **`workflow_dispatch` → Run workflow** against `main`, leaving **cdp_port** at
  its `9222` default. Or from the CLI:
  `gh workflow run cubase-nightly.yml --repo JimAKennedy/poly --ref main`.
- ☐ Watch the e2e steps run in order: *Install e2e deps → Run L4-web e2e (attach,
  toggle, transport)* while Cubase is up, then after quit *Assert toggled step in
  probe (L4-web)*.
- ☐ On failure, the `cubase-e2e-traces` artifact carries Playwright traces +
  screenshots; the `cubase-nightly-artifacts` upload carries the probe JSONL and
  `e2e-expected-hit.json`.

**Verify:** A manual `workflow_dispatch` run completes green with the L4-web e2e
attaching over CDP, toggling the step, and the post-quit assertion confirming the
probe JSONL contains the toggled note-on. This run is the **S09 exit-criterion
evidence** — capture the run URL for the slice's UAT record.

---

## Appendix — What runs where (mental model)

| Layer | What | Where it runs on this box |
|---|---|---|
| L1 | Engine + unit tests | `ctest` (Part 6) |
| L2 | Bridge contract + webui | Playwright from `webui\` (Part 6) |
| L3 | VST3 validator + pluginval + in-process host + golden MIDI | build-time validator + `pluginval.exe` + `ctest` (Part 6) |
| L4 | Cubase-in-the-loop | Cubase + loopMIDI + mido driver + `poly_midi_probe` (S08) |
| L4-web | Playwright over CDP into WebView2 **inside Cubase** | S09 — Windows-only, the reason this box is the flagship |

**S07 is the slice this runbook satisfies.** S08 (fixtures + MIDI driver +
MIDI Remote script) and S09 (Playwright-over-CDP) build on the machine this
produces — you author those from your dev box; they *execute* here.
