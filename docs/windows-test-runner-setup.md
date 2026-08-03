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

**Verify — all five must succeed:**
```powershell
git --version; cmake --version; ninja --version
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
  shared VST3 folder:
  ```powershell
  Copy-Item -Recurse -Force build\VST3\Release\poly_plugin.vst3 `
    "C:\Program Files\Common Files\VST3\poly_plugin.vst3"
  ```
  In Cubase, rescan plugins; confirm **Poly** appears and instantiates on an
  instrument track, opening the **WebUI editor** (a web view, not the old native
  layout).

**Verify:** ctest all-green; pluginval exits 0; Playwright all-green; Poly loads
in Cubase and its WebUI editor renders.

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

Now wire the machine into CI. The nightly workflow (authored in S07) targets the
`cubase` label.

- ☐ In GitHub: **repo → Settings → Actions → Runners → New self-hosted runner →
  Windows x64.** GitHub shows a per-runner token and download commands.
- ☐ On the box, run the shown `config.cmd`. When prompted:
  - **Labels:** add `cubase` (the nightly workflow keys off this).
  - **Work folder:** default `_work` is fine.
- ☐ **Install the runner as a LOGON SCHEDULED TASK, not a Windows service.** The
  default `svc.cmd install` registers a *service* — which has **no interactive
  desktop** and will break Cubase/CDP. Instead:
  - Do **not** run `svc.cmd install`.
  - Create a **Task Scheduler** task that runs `run.cmd` from the runner
    directory, triggered **At log on** of `polyci`, "Run only when user is
    logged on", highest privileges, and **not** "Stop if runs longer than…".
    (Auto-logon from Part 7 makes this fire on every boot.)
- ☐ Reboot. Confirm the runner comes up **idle/listening** in the GitHub Runners
  page after auto-logon.

**Verify:** GitHub → Settings → Actions → Runners shows this runner **online**
with the `cubase` label, and it's running as a logon task (visible in Task
Scheduler), not as a service.

---

## Part 10 — Security posture (non-negotiable — the runner executes repo code)

A self-hosted runner runs whatever a workflow tells it to. Lock it down.

- ☐ **The nightly workflow (S07) must be gated to `schedule` +
  `workflow_dispatch` on `main` only — NEVER `pull_request`.** A `pull_request`
  trigger would let any fork's PR run arbitrary code on your licensed box. (This
  is encoded in the workflow YAML in S07; the machine's job is to not be
  reachable in the first place.)
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

- ☐ From the repo Actions tab, once the S07 workflow exists, use
  **`workflow_dispatch`** to trigger it manually against `main`.
- ☐ Watch: the `cubase`-labelled runner picks up the job; the job builds/installs
  Poly, launches Cubase with the fixture `.cpr`, and quits Cubase cleanly (via
  key command) at the end.
- ☐ Confirm artifacts (probe JSONL, logs, and on failure the screen recording)
  upload.

**Verify:** A manual `workflow_dispatch` run completes green on the `cubase`
runner, with Cubase having launched and quit without a hung process left behind.

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
