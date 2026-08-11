---
class: gated
---

# Keep the `JIMW1` runner awake for the overnight nightly

The `cubase-nightly.yml` workflow runs on a `schedule:` cron (02:00 UTC) as well
as `workflow_dispatch`. Cubase's MIDI Remote surface, UI automation, and CDP all
need a **live, unlocked, interactive console session** — not just a powered-on
machine. If the box sleeps, the display powers off, or the session locks
overnight, Cubase launches but its MIDI Remote surface never activates, so the
driver never gets its ready ping and the nightly fails.

This file collects the exact config changes that keep the runner usable
unattended, so they're easy to find — and easy to tidy up — while `JIMW1` is a
poly-only test box. It **complements** `docs/windows-test-runner-setup.md` Part 7
(interactive-desktop hardening); apply Part 7 first, then this for the
overnight-specific bits.

## The failure this fixes

Symptom (scheduled runs `31235480922`, `31290961668`, `31350811504`,
`31453271210` — every 02:00 UTC run for four nights):

- Build, ctest, install, launch, and wait-ready all pass.
- **`Play scenario (mido driver)` fails**: the driver logs
  `[driver:port-open]` then polls the full 90 s with `any_rx=True` and
  `[driver:error] no ready ping (CC119=127) within 90.0s`.

`any_rx=True` means the loopMIDI loopback wire is alive (the driver's own poll
echoes come back) — so loopMIDI is running and the port is bound. What's missing
is the **Cubase-side** ready ping: the MIDI Remote script's `driver.mOnActivate`
never fires because Cubase's MIDI Remote surface never connected. The identical
commit passed 80 minutes earlier as a `workflow_dispatch` (owner at the console
via VNC). The only variable is session liveness at 02:00.

Root cause: the console session was present but **not held live** — the machine
slept, the display powered off, or the session locked, and Cubase's MIDI Remote
subsystem does not bind its surface without an active, unlocked desktop.

## Config changes (run once, from an elevated PowerShell as `polyci`)

### 1. Power: never sleep, never hibernate, never blank the display

Part 7 lists these; re-run them to be certain they stuck (Windows Update can
reset a power plan):

```powershell
powercfg /change standby-timeout-ac 0     # never sleep on AC
powercfg /change hibernate-timeout-ac 0   # never hibernate on AC
powercfg /change monitor-timeout-ac 0     # never power off the display
powercfg /hibernate off                   # remove hibernation entirely
powercfg /change disk-timeout-ac 0        # never spin down the disk
```

Confirm nothing is scheduled to sleep the box:

```powershell
powercfg /query SCHEME_CURRENT SUB_SLEEP   # STANDBYIDLE / HIBERNATEIDLE == 0x0
```

### 2. Do not let the session lock or blank

A locked session starves Cubase's MIDI Remote surface even though the machine is
awake. Disable the lock screen timeout and the screensaver for `polyci`:

```powershell
# No screensaver, no screensaver-triggered lock (HKCU = polyci's hive).
Set-ItemProperty 'HKCU:\Control Panel\Desktop' ScreenSaveActive '0'
Set-ItemProperty 'HKCU:\Control Panel\Desktop' ScreenSaveTimeOut '0'
Set-ItemProperty 'HKCU:\Control Panel\Desktop' ScreenSaverIsSecure '0'

# Do not require a password on wake (belt-and-suspenders with never-sleep).
powercfg /setacvalueindex SCHEME_CURRENT SUB_NONE CONSOLELOCK 0
powercfg /setactive SCHEME_CURRENT
```

Disable the interactive-logon lock timeout via policy (0 = never auto-lock):

```powershell
# Machine inactivity limit: 0 disables the automatic lock.
New-ItemProperty -Path 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System' `
  -Name 'InactivityTimeoutSecs' -Value 0 -PropertyType DWord -Force | Out-Null
```

> If the box is domain-joined, a GPO may re-lock it — but `JIMW1` is a
> standalone poly test box, so the local policy above holds.

### 3. Keep a display context alive

Part 7 already calls for the **dummy HDMI plug** (or an attached monitor).
Without a display context Windows can drop to a headless GPU state and Cubase /
WebView2 rendering breaks even with power settings correct. Verify the plug is
in.

### 4. Remote access via VNC, never RDP

RDP disconnect tears down the console session (which is what Cubase needs). Use
VNC (it attaches to the console without destroying it). If you must RDP, apply
the `tscon`-redirect-to-console trick on disconnect (Part 7). **Log out of any
RDP session before the nightly window** — a disconnected-but-not-logged-out RDP
session is the classic cause of a locked overnight desktop.

### 5. Auto-logon holds the session across reboots

Part 7's Sysinternals `Autologon` (or `netplwiz`) makes `polyci` land on the
console at boot with no password prompt, so a Windows-Update reboot recovers to a
live desktop and the runner logon task refires. Confirm it's still configured.

## Verify (before trusting the next overnight run)

1. **Simulate the overnight condition.** From VNC, do **not** touch the machine:
   disconnect VNC (do not log out) and leave it for longer than the sleep/lock
   timeouts *used to be* (e.g. 30–60 min). Reconnect via VNC.
2. Confirm the desktop is **still unlocked and interactive** — no lock screen,
   no password prompt, Cubase-able.
3. **Best signal — dispatch the nightly while disconnected.** From another
   machine, `gh workflow run cubase-nightly.yml --repo JimAKennedy/poly --ref main`
   while you are *not* VNC'd in, and confirm the `Play scenario` step passes with
   `[driver:ready-received] MIDI Remote script is live`. That reproduces the 2 AM
   condition (no human at the console) on demand.
4. **Then let the cron prove it.** The next 02:00 UTC scheduled run should go
   green end-to-end (`Compare probe output to golden` = success). If it still
   fails at `Play scenario` with `any_rx=True` and no ready ping, the session is
   still locking/blanking — re-check steps 2 and 4 above.

## Quick diagnosis if a future overnight run fails again

Pull the driver lines from the failed run's log:

- `any_rx=True` + no ready ping → loopback alive, **Cubase surface never
  activated** → session/desktop liveness (this doc). Check lock/sleep/RDP.
- `any_rx=False` → the loopback itself is dead → loopMIDI not running or the
  `poly-test` port missing (re-run `4-install-driver-deps.ps1` / recreate the
  port).
- `[driver:error] port 'poly-test' not found` → loopMIDI port absent entirely.

## Tidy-up (when `JIMW1` gets repurposed for other runner workloads)

These changes make the box permanently awake and unlocked — appropriate for a
dedicated poly test runner, **not** for a shared or higher-value machine. When
you add other GH runner workloads, revisit:

- Re-enable the lock/screensaver timeouts (steps 2) if the box holds anything
  sensitive.
- Reconsider "no password on wake" (`CONSOLELOCK`) and `InactivityTimeoutSecs`.
- The de-elevation / drop-admin follow-ups in
  `docs/windows-runner-rehome-and-deelevate.md` still apply regardless.

## Cross-references

- `docs/windows-test-runner-setup.md` Part 7 — interactive-desktop hardening (the
  authoritative provisioning runbook; this file is the overnight-specific
  supplement).
- `docs/windows-test-runner-setup.md` Part 11 — why Cubase needs a live desktop.
- `.github/workflows/cubase-nightly.yml` — the nightly this keeps green
  overnight (`schedule:` + `workflow_dispatch`).
- `tests/cubase/driver/README.md` — the driver whose ready-ping timeout is the
  symptom surface.
