---
class: gated
---

# Windows runner — rehome and de-elevate

Remediation runbook for the `JIMW1` Cubase box. Three independent changes, in
the order they must happen:

| Part | Change | Why | Downtime |
|---|---|---|---|
| A | Turn Smart App Control off | Unblocks the build **today** | Reboot |
| B | Move the runner out of `C:\Windows\System32` | Repo code is building inside a protected OS directory | ~10 min, runner offline |
| C | De-elevate the runner (and drop `polyci` from Administrators) | Runner executes repo code; it should not be admin — **and an elevated runner makes the S09 CDP e2e impossible** | None if prerequisites are met |

> **Order matters.** Part A is the only one that fixes the currently-failing
> nightly — B and C are hygiene and change nothing about the build outcome. Do A,
> confirm green, then B, then C. Doing C first will make B need a second admin
> login you may not have tested yet.

> **Part C3 is no longer optional.** It started life as security hygiene, but it
> is now a functional requirement of the L4-web tier. WebView2 discards browser
> flags delivered via the environment or the registry whenever the host app is
> elevated, so an elevated Cubase can never expose the `--remote-debugging-port`
> CDP endpoint the Playwright e2e attaches to — that was the whole of the M042
> S09 seven-round dead end (runs #40–#47). See
> `docs/windows-test-runner-setup.md` Part 12. If you ever re-elevate the logon
> task, the nightly's CDP steps fail loud by design.

This runbook is a companion to `docs/windows-test-runner-setup.md` (the
from-scratch build of this machine). Part 9 of that doc is the step that, left
under-specified, produced the `System32` install this runbook cleans up.

---

## Background — what went wrong

**1. Smart App Control turned itself on.** On **Sat 1 Aug 2026, 12:57 local**,
SAC promoted itself from evaluation mode to enforcement with no human action:

```
HKLM\SYSTEM\CurrentControlSet\Control\CI\Policy
  VerifiedAndReputablePolicyState : 1   # enforcing
  SAC_PreviousState               : 2   # was evaluation
  SAC_EnforcementReason           : 1
```

Evaluation mode silently profiles a machine and flips to enforcement on its own
once it decides the box looks clean. Nothing in the repo, the runner, or the
workflow changed.

**2. It blocked the VST3 SDK's post-build tool.** The first nightly afterwards
that actually re-linked the plugin died in the build step:

```
Event 3077 (Error) — Microsoft-Windows-CodeIntegrity/Operational
  Code Integrity determined that a process (cmd.exe) attempted to load
    ...\build\bin\Release\moduleinfotool.exe
  that did not meet the Enterprise signing level requirements
  Policy ID: {0283ac0f-fff1-49ae-ada1-8a933130cad6}   # "VerifiedAndReputableDesktop" = SAC
```

`moduleinfotool.exe` generates `moduleinfo.json` for the VST3 bundle. It is
compiled locally, `NotSigned`, and gets a **new hash on every build** — so it has
zero reputation every single time and can never earn its way past SAC. Timeline
on the failing run: linked at `11:30:06`, blocked at `11:30:09`.

An earlier run that day passed only because it did not re-link the tool; the
post-build step never executed. The apparent "config change window" between the
last green run and the first red one is a red herring.

**3. The runner is installed inside `System32`.** Created **2 Aug 2026, 21:03**
at `C:\Windows\System32\actions-runner`, with `_work` — the checkout and build
tree — underneath it. Unrelated to the SAC failure, but wrong on its own terms.

---

## Part A — Turn Smart App Control off

SAC is fundamentally incompatible with a machine whose job is to compile and
immediately execute unsigned binaries.

> **This is a one-way door.** Re-enabling SAC requires a clean Windows reinstall.
> That is an acceptable trade on a dedicated build box; do not do it on a
> workstation.

**Why the alternatives don't work:**

- **Defender exclusions do not apply.** SAC is a Code Integrity policy, not
  Defender. `Add-MpPreference -ExclusionPath` has no effect on it.
- **There is no per-file allowlist.** SAC is on/off; no exemption UI exists.
- **Signing is impractical** — you would be re-signing an SDK tool on every
  build.
- **Suppressing the tool** (`SMTG_CREATE_MODULE_INFO=OFF`, a VST3 SDK CMake
  option — the repo does not set it) works, but costs you `moduleinfo.json`,
  which Cubase 12+ uses for faster plugin scanning. Keep this in reserve.

### Steps

- ☐ **Via the UI:** Windows Security → *App & browser control* → *Smart App
  Control settings* → **Off**.
- ☐ **Or via registry**, from an elevated PowerShell:
  ```powershell
  Set-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\CI\Policy" `
                   -Name VerifiedAndReputablePolicyState -Value 0 -Type DWord -Force
  ```
- ☐ **Reboot.** The registry write does not take effect in the current boot
  session — the old policy stays loaded and keeps blocking until restart.

**Verify:** after reboot,

```powershell
(Get-ItemProperty "HKLM:\SYSTEM\CurrentControlSet\Control\CI\Policy").VerifiedAndReputablePolicyState
# expect 0

Get-WinEvent -FilterHashtable @{
  LogName = "Microsoft-Windows-CodeIntegrity/Operational"; Id = 3077
} -MaxEvents 5 -ErrorAction SilentlyContinue |
  Select-Object TimeCreated, Id
# expect no new 3077 events after the reboot timestamp
```

Then dispatch **Cubase Nightly (L4)** manually and confirm the build step passes.

---

## Part B — Rehome the runner to `C:\actions-runner`

### Why it matters

- `_work` — a checkout of the repo, built by whatever the workflow says — lives
  under a **protected OS directory**.
- The tree inherits `System32`'s ACL: `Users` get `ReadAndExecute` only, so the
  runner **must** run elevated just to write its own work folder. That forces
  the elevation Part C wants to remove.
- Windows Update, SFC, and DISM all treat `System32` as theirs. A servicing
  operation is entitled to be surprised by it.

> **This does not fix the build.** SAC blocks on signature and reputation, not
> path — `moduleinfotool.exe` is blocked identically at `C:\actions-runner`.
> Part A is the fix; Part B is hygiene.

### Prerequisites

- ☐ An elevated PowerShell on `polyci` (still an admin at this point).
- ☐ Two tokens from **repo → Settings → Actions → Runners**, both valid ~1 hour,
  so fetch each as you reach its step:
  - a **removal** token — click the `JIMW1` runner → *Remove*
  - a **registration** token — *New self-hosted runner*
- ☐ Note the current registration so you can reproduce it:
  ```
  name       JIMW1
  url        https://github.com/JimAKennedy/poly
  labels     cubase          # cubase-nightly.yml targets [self-hosted, cubase]
  work       _work
  ```

### B1 — Stop and deregister the old runner

```powershell
Stop-ScheduledTask   -TaskName "GitHubActionsRunner"
Disable-ScheduledTask -TaskName "GitHubActionsRunner"

Set-Location C:\Windows\System32\actions-runner
.\config.cmd remove --token <REMOVAL_TOKEN>
```

If `config.cmd remove` fails (expired token, runner already gone), delete the
runner in the GitHub UI instead and continue — the local files are discarded in
B4 either way.

**Verify:** the runner no longer appears under Settings → Actions → Runners.

### B2 — Install clean at the correct path

```powershell
New-Item -ItemType Directory -Force C:\actions-runner
Set-Location C:\actions-runner

$ver = "2.336.0"   # or current: https://github.com/actions/runner/releases
Invoke-WebRequest -Uri "https://github.com/actions/runner/releases/download/v$ver/actions-runner-win-x64-$ver.zip" `
                  -OutFile runner.zip
Expand-Archive .\runner.zip -DestinationPath . -Force
Remove-Item .\runner.zip

.\config.cmd --url https://github.com/JimAKennedy/poly `
             --token <REGISTRATION_TOKEN> `
             --name JIMW1 --labels cubase --work _work --unattended
```

- `--labels cubase` is **not optional** — without it the nightly never gets
  scheduled and simply hangs as queued.
- Do **not** run `svc.cmd install`. A Windows service has no interactive
  desktop; Cubase and the S09 CDP work require one. See
  `windows-test-runner-setup.md` Part 7.

**Verify:** GitHub shows `JIMW1` online with the `cubase` label.

### B3 — Repoint the logon task

Simplest is to delete and recreate, so the working directory moves too:

```powershell
Unregister-ScheduledTask -TaskName "GitHubActionsRunner" -Confirm:$false

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

Keep `-RunLevel Highest` for now; Part C removes it.

`-ExecutionTimeLimit ([TimeSpan]::Zero)` is the "not *Stop if runs longer
than…*" setting. Without it Task Scheduler kills the listener after three days
and the runner goes quietly offline.

**Verify:** reboot, confirm auto-logon lands on the desktop and the runner
returns to idle/listening without anyone touching it.

### B4 — Delete the old tree

Only after GitHub shows the new runner online:

```powershell
Remove-Item -Recurse -Force C:\Windows\System32\actions-runner
```

**Verify:**

```powershell
Test-Path C:\Windows\System32\actions-runner   # expect False
(Get-ScheduledTask GitHubActionsRunner).Actions.Execute   # expect C:\actions-runner\run.cmd
```

Then dispatch the nightly once and confirm it runs green from the new path.

---

## Part C — De-elevate `polyci`

The runner executes whatever a workflow tells it to. It should not be running as
a local administrator. This is not a one-liner — two real dependencies on
elevation have to be removed first.

### C1 — Rehome the plugin install (the actual blocker) — **done in the repo**

`cubase-nightly.yml` used to install the built bundle to a machine-wide
location, `C:\Program Files\Common Files\VST3`. That folder is writable only by
`Administrators`, `SYSTEM`, and `TrustedInstaller`, so the *Install plugin for
Cubase* step — which does a `Remove-Item -Recurse -Force` then
`Copy-Item -Recurse` — needed admin.

**Resolved: the workflow now installs to the per-user VST3 path**,
`%LOCALAPPDATA%\Programs\Common\VST3`, which Cubase scans alongside the
machine-wide one (confirmed against the runner's own
`Cubase Pro VST3 Cache\vst3plugins.xml`) and which a non-admin can write. The
step derives it from `$env:LOCALAPPDATA` inside the `pwsh` block — note that a
job-level `env:` entry **cannot** work here, because the `env` expression
context sees only workflow-defined variables and `${{ env.LOCALAPPDATA }}`
expands to the empty string. `scripts/S08-install/_shared.ps1` defaults to the
same path.

The remaining task is a one-time cleanup that only an admin can do:

- ☐ **Delete the Poly bundles from the machine-wide folder.** Cubase scans both
  locations, and two bundles with the same VST3 class ID resolve to whichever
  copy it scanned first — leave them and the nightly may silently test a stale
  build. From an elevated PowerShell:
  ```powershell
  Remove-Item -Recurse -Force "C:\Program Files\Common Files\VST3\poly_plugin.vst3"
  Remove-Item -Recurse -Force "C:\Program Files\Common Files\VST3\poly_midi_probe.vst3"
  ```
  The install step fails the job if it finds either one, so this cannot be
  forgotten silently.
- ☐ Dispatched the nightly and confirmed the install step passes.

### C2 — Confirm a second working admin

- ☐ **Log in as the fallback admin and verify it actually works** before
  touching `polyci`.

Current state of this box:

| Account | Enabled | Admin | Notes |
|---|---|---|---|
| `Administrator` | **No** | yes | built-in, disabled |
| `polyci` | yes | yes | the runner account — being de-elevated |
| `theji` | yes | yes | Microsoft account — **the fallback** |

`theji` is the only other usable admin. If you cannot log into it, either fix
that or enable the built-in `Administrator` (`net user Administrator /active:yes`
plus a strong password) **before** proceeding. Otherwise you lose the ability to
install VS Build Tools updates, Cubase updates, and drivers — the exact reasons
Part 1a of the setup doc made `polyci` an admin.

### C3 — Drop the task's elevation — **required for the S09 CDP e2e**

Re-register the task principal with `-RunLevel Limited`, from an elevated
PowerShell:

```powershell
$p = New-ScheduledTaskPrincipal -UserId "$env:COMPUTERNAME\polyci" `
                                -LogonType Interactive -RunLevel Limited
Set-ScheduledTask -TaskName "GitHubActionsRunner" -Principal $p

# The change only takes effect for a NEW listener process.
Stop-ScheduledTask  -TaskName "GitHubActionsRunner"
Start-ScheduledTask -TaskName "GitHubActionsRunner"
```

This only works once Part B has moved the runner off `System32` — a
non-elevated `polyci` cannot write to the old tree at all. `C:\actions-runner`
grants `Authenticated Users:(M)`, so the work tree stays writable after the
de-elevation.

C3 is sufficient on its own for the CDP fix: `-RunLevel Limited` gives the task
`polyci`'s **filtered** token (medium integrity), which is exactly the context
an ordinary VNC shell runs in. C4 below removes the admin membership as well,
which is worth doing but is not what CDP depends on.

**Verify** — from a normal, non-elevated PowerShell:

```powershell
(Get-ScheduledTask GitHubActionsRunner).Principal.RunLevel   # expect Limited
```

Then dispatch **Cubase Nightly (L4)** and confirm the *Wait for Poly editor CDP
endpoint* step passes. `editor-window-topology.txt` in the run artifacts should
report `this process elevated: NO` and a WebView2 child carrying
`--remote-debugging-port`.

### C4 — Remove admin membership

```powershell
Remove-LocalGroupMember -Group "Administrators" -Member "$env:COMPUTERNAME\polyci"
```

- ☐ **Log `polyci` fully out and back in.** Group membership is baked into the
  logon token; the running session keeps its admin rights until it ends.

**Verify:** after the re-logon,

```powershell
whoami /groups | Select-String "S-1-5-32-544"   # expect no match (not in Administrators)
Get-LocalGroupMember Administrators | Select-Object Name
```

Then dispatch the nightly and confirm a full green run — configure, build,
ctest, install, Cubase launch/quit — as a non-admin.

---

## Rollback

| Part | Reversible? | How |
|---|---|---|
| A — SAC off | **No** | Clean Windows reinstall only. |
| B — rehome | Yes | Re-register the runner at the old path; nothing is destroyed until B4. |
| C — de-elevate | Yes | `Add-LocalGroupMember -Group Administrators -Member polyci`, re-register the task with `-RunLevel Highest`, re-logon. |

## Gotchas

- **A registry-only SAC change needs a reboot.** Setting the value to `0` and
  re-running the nightly without restarting still fails, identically.
- **Runner tokens expire in about an hour.** Fetch each one at the step that
  uses it, not all up front.
- **Losing the `cubase` label** turns the failure mode from "job fails" into
  "job silently queues forever". Check the label first when the nightly stops
  reporting at all.
- **Group membership changes need a re-logon**, not just a task restart.
- **Auto-logon must survive the reboots.** Every part of this runbook reboots at
  least once; if Sysinternals `Autologon` is not still configured (Part 7 of the
  setup doc), the box comes back to a locked screen and the runner never starts.

## Evidence

Captured on `JIMW1`, 4 Aug 2026, while diagnosing the nightly failure:

- `Microsoft-Windows-CodeIntegrity/Operational` — events `3077`/`3033`/`3118` at
  `11:30:09` local, policy `{0283ac0f-fff1-49ae-ada1-8a933130cad6}`.
- `HKLM\SYSTEM\CurrentControlSet\Control\CI\Policy` — key last written
  **1 Aug 2026 12:57:47** local; `SAC_PreviousState = 2`.
- `build\bin\Release\moduleinfotool.exe` — `NotSigned`, created `11:30:06`,
  blocked `11:30:09`.
- `_diag\Worker_20260804-135419-utc.log` → `Succeeded` (no re-link);
  `_diag\Worker_20260804-152810-utc.log` → `Failed`, build step exit code 1.
