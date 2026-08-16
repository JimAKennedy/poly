# M042 S09: assert the runner-machine invariants the Cubase tiers silently
# depend on. Read-only — changes nothing, exits 1 on any FAIL.
#
# Why this exists: every check below corresponds to a real failure that cost
# multiple debugging rounds because it presented as something else. The runner
# is provisioned by hand from docs/windows-test-runner-setup.md, so nothing but
# this script stops a re-provision (or a reboot into a re-registered task) from
# reintroducing them. The nightly runs it BEFORE the build so a broken posture
# fails in ten seconds with remediation text, instead of thirty minutes later as
# an opaque timeout.
#
# Run it by hand any time you touch the runner's configuration:
#   pwsh -File scripts/cubase/check-runner-posture.ps1 -RequireCdp
#
# IMPORTANT: run it from a NORMAL, non-elevated PowerShell. An elevated shell
# reports its own elevation and check 1 will (correctly) fail.

[CmdletBinding()]
param(
    # Assert the extra prerequisites of the L4-web CDP e2e (WebView2 Runtime,
    # non-elevated host). The nightly passes this when POLY_CDP_PORT is set.
    [switch] $RequireCdp,
    # VST3 dir the plugin is installed to. Defaults to the per-user location the
    # nightly installs to. The OTHER standard location is then treated as a
    # shadow and must be empty of Poly bundles.
    [string] $Vst3InstallDir = "",
    # Name of the runner's logon scheduled task, checked for elevation.
    [string] $TaskName = "GitHubActionsRunner"
)

. "$PSScriptRoot/_common.ps1"

$script:Failures = 0
$script:Warnings = 0

function Write-Check {
    param(
        [Parameter(Mandatory)] [ValidateSet("ok", "warn", "fail", "info")] [string] $Level,
        [Parameter(Mandatory)] [string] $Message,
        # Shown only on fail/warn: what to actually do about it.
        [string] $Remedy = ""
    )
    $prefix = switch ($Level) {
        "ok"   { "[ OK ] " }
        "warn" { "[WARN] " }
        "fail" { "[FAIL] " }
        default { "[ .. ] " }
    }
    $color = switch ($Level) {
        "ok"   { "Green" }
        "warn" { "Yellow" }
        "fail" { "Red" }
        default { "Cyan" }
    }
    Write-Host "$prefix$Message" -ForegroundColor $color
    if ($Remedy -and $Level -ne "ok") {
        foreach ($line in ($Remedy -split "`n")) {
            Write-Host "       $line" -ForegroundColor $color
        }
    }
    if ($Level -eq "fail") { $script:Failures++ }
    if ($Level -eq "warn") { $script:Warnings++ }
}

Write-PolyPhase -Phase "check-posture" -State "start" `
    -Detail "asserting runner-machine invariants" -Extra @{ requireCdp = [bool]$RequireCdp }

if (-not $IsWindows) {
    Write-Check -Level info -Message "Not Windows — the Cubase tiers are Windows-only; nothing to check."
    exit 0
}

# --- 1. This process must NOT be elevated -------------------------------------
# The S09 dead end. WebView2 discards browser flags delivered "via the local
# device environment" (WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS and the
# Edge/WebView2 registry policy keys) whenever the host app runs elevated:
#   https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/webview-features-flags
# Cubase is launched as a child of the job shell, so an elevated runner means an
# elevated Cubase means no CDP endpoint, ever. Runs #40-#47 chased sessions,
# desktop lock, editor focus and WebView2 process reuse before this was found.
$isElevated = ([Security.Principal.WindowsPrincipal]`
    [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator)
if ($isElevated) {
    Write-Check -Level fail -Message "This process is ELEVATED. WebView2 will discard the CDP debugging flag and the L4-web e2e cannot work." -Remedy @"
Re-register the runner's logon task with -RunLevel Limited, from an ELEVATED shell:
  `$p = New-ScheduledTaskPrincipal -UserId "`$env:COMPUTERNAME\polyci" ``
                                  -LogonType Interactive -RunLevel Limited
  Set-ScheduledTask -TaskName "$TaskName" -Principal `$p
  Stop-ScheduledTask -TaskName "$TaskName"; Start-ScheduledTask -TaskName "$TaskName"
See docs/windows-runner-rehome-and-deelevate.md Part C3.
"@
} else {
    Write-Check -Level ok -Message "Not elevated (medium integrity) — WebView2 will honor the CDP flag."
}

# --- 2. The logon task itself must be registered Limited -----------------------
# Check 1 catches the live process; this catches the CONFIG, so a fix that was
# only applied to the running listener (and would revert on the next logon)
# still fails here.
$task = Get-ScheduledTask -TaskName $TaskName -ErrorAction SilentlyContinue
if (-not $task) {
    Write-Check -Level info -Message "Scheduled task '$TaskName' not found — skipping the task-elevation check (not the runner box, or a differently-named task)."
} elseif ($task.Principal.RunLevel -eq "Highest") {
    Write-Check -Level fail -Message "Scheduled task '$TaskName' is registered -RunLevel Highest. Every job step, and the Cubase it launches, will be elevated." -Remedy @"
Re-register it Limited (see check 1's remedy). The running listener may already
be non-elevated, but the next logon would restore the elevated one.
"@
} else {
    Write-Check -Level ok -Message "Scheduled task '$TaskName' is -RunLevel $($task.Principal.RunLevel)."
}

# --- 3. The runner must be a logon task, not a Windows service -----------------
# A service runs in session 0 with no interactive desktop: Cubase cannot present
# a UI, the Poly editor never materializes, and neither the MIDI Remote surface
# nor CDP come up. Documented in the runbook Part 9, but nothing enforced it.
$svc = Get-Service -Name "actions.runner.*" -ErrorAction SilentlyContinue
if ($svc) {
    $names = ($svc | ForEach-Object { $_.Name }) -join ", "
    Write-Check -Level fail -Message "The runner is installed as a Windows SERVICE ($names). Session 0 has no interactive desktop, so Cubase cannot run there." -Remedy @"
Uninstall the service and use the logon scheduled task instead:
  C:\actions-runner\svc.cmd uninstall
Then follow docs/windows-test-runner-setup.md Part 9.
"@
} else {
    Write-Check -Level ok -Message "No actions.runner service installed (logon-task model, as required)."
}

# --- 4. Exactly one copy of each Poly bundle across the VST3 search paths ------
# Cubase scans BOTH the per-user and machine-wide VST3 folders. Two bundles with
# the same VST3 class ID resolve to whichever was scanned first, so a leftover
# copy makes the nightly test a stale build while reporting green — the
# ISSUE-001 blank-window trap, and what masked a stale e2e contract for weeks.
if (-not $Vst3InstallDir) {
    $Vst3InstallDir = Join-Path $env:LOCALAPPDATA "Programs\Common\VST3"
}
$machineWide = "C:\Program Files\Common Files\VST3"
$shadowDir = if ($Vst3InstallDir -eq $machineWide) {
    Join-Path $env:LOCALAPPDATA "Programs\Common\VST3"
} else {
    $machineWide
}
foreach ($bundle in @("poly_plugin.vst3", "poly_midi_probe.vst3")) {
    $shadow = Join-Path $shadowDir $bundle
    if (Test-Path $shadow) {
        Write-Check -Level fail -Message "$bundle also exists at $shadow — it shadows the install target ($Vst3InstallDir) and Cubase may load it instead." -Remedy @"
Delete the shadow copy (elevated shell if it is under Program Files):
  Remove-Item -Recurse -Force '$shadow'
Then clear Cubase's VST3 cache so it rescans from scratch.
"@
    } else {
        Write-Check -Level ok -Message "No shadow $bundle in $shadowDir."
    }
}

# --- 5. WebView2 Runtime present (CDP flows only) ------------------------------
# Poly's editor is hosted by WebView2 via choc; without the Evergreen Runtime
# there is no editor and no CDP endpoint to attach to.
if ($RequireCdp) {
    $wvKey = "HKLM:\SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}"
    $wvVersion = (Get-ItemProperty -Path $wvKey -Name pv -ErrorAction SilentlyContinue).pv
    if ($wvVersion) {
        Write-Check -Level ok -Message "WebView2 Evergreen Runtime $wvVersion installed."
        # >= 150 additionally refuses to serve the DevTools endpoint to an
        # elevated host however the flag is delivered
        # (MicrosoftEdge/WebView2Feedback#5640) — worth stating explicitly so a
        # future reader does not "fix" check 1 by delivering the flag in code.
        $major = ($wvVersion -split '\.')[0] -as [int]
        if ($major -ge 150) {
            Write-Check -Level info -Message "Runtime is >= 150: the DevTools endpoint is unreachable for ELEVATED hosts regardless of how the flag is delivered (WebView2Feedback#5640). Non-elevated is the only working configuration."
        }
    } else {
        Write-Check -Level fail -Message "WebView2 Evergreen Runtime not found — Poly's editor cannot host and CDP cannot be exposed." -Remedy @"
  winget install --id Microsoft.EdgeWebView2Runtime --accept-source-agreements --accept-package-agreements
"@
    }
}

# --- 6. Interactive console session (warn) -------------------------------------
# Cubase needs a real desktop. This is a warning, not a failure: the check may
# run before the session settles, and the launch/wait-ready phase fails loudly
# and specifically if the desktop really is unusable.
$sessionId = (Get-Process -Id $PID).SessionId
if ($sessionId -eq 0) {
    Write-Check -Level warn -Message "Running in session 0 (no interactive desktop). Cubase will not be able to present its UI." `
        -Remedy "Run the runner as a logon task on the console session — docs/windows-test-runner-setup.md Part 7 and Part 9."
} else {
    Write-Check -Level ok -Message "Running in interactive session $sessionId."
}

Write-Host ""
if ($script:Failures -gt 0) {
    Invoke-PolyPhaseFailure -Phase "check-posture" `
        -Message "$($script:Failures) runner-posture check(s) FAILED, $($script:Warnings) warning(s). Fix the items above before the Cubase phase — each one silently breaks a tier." `
        -Extra @{ failures = $script:Failures; warnings = $script:Warnings }
}

Write-PolyPhase -Phase "check-posture" -State "ok" `
    -Detail "all runner-posture checks passed" `
    -Extra @{ warnings = $script:Warnings; requireCdp = [bool]$RequireCdp }
exit 0
