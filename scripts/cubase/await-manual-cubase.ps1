# M042 S09: pause the CI job for the owner to launch Cubase by hand, then
# proceed once the Poly editor's CDP endpoint is live.
#
# Why this exists (the seven-round S09 finding):
#   Under the unattended Actions-agent launch, choc's WebView2 never exposes the
#   --remote-debugging-port CDP endpoint even though the editor + WebView2 host
#   materialize (runs #40/#42/#44/#45/#46/#47: 0 of 18 msedgewebview2.exe
#   children ever carried the flag, port ABSENT). Yet a Cubase launched BY HAND
#   in the owner's interactive VNC session DID expose CDP on 127.0.0.1:9222
#   (runner-confirmed, #215). The differentiator is the session/instantiation
#   path, not anything reachable from the launch script.
#
#   So in the manual-CDP flow the CI job does NOT launch Cubase. It builds and
#   installs the FRESH plugin + probe, then blocks HERE until the owner:
#     1. opens Cubase on the checked-out fixture (loading the just-installed
#        plugin), and
#     2. drops a sentinel file to signal "Cubase is up, go".
#   This gate waits for BOTH the sentinel AND a live CDP port, so a forgotten
#   sentinel or a not-yet-materialized editor can't let the e2e attach against a
#   half-ready Cubase. It fails loud on timeout so a forgotten dispatch never
#   pins the single runner.
#
# Runs only in the manual-CDP flow (POLY_MANUAL_CUBASE == 'true'); the auto
# launch/wait-ready/kill-stale steps are skipped in that flow.

[CmdletBinding()]
param(
    # The CDP remote-debugging port the owner's manually-launched Cubase must
    # expose (matches POLY_CDP_PORT / the e2e's CDP_ENDPOINT).
    [Parameter(Mandatory)] [int] $CdpPort,
    # Absolute path of the fixture the owner should open, echoed in the prompt so
    # they always open the freshly-checked-out project (not a stale hand clone).
    [string] $FixtureCpr = "",
    # The sentinel file the owner creates once Cubase is up. Its presence is the
    # explicit human "go" signal.
    [string] $GoFile = "",
    # Hard ceiling. A forgotten dispatch must never pin the single runner; on
    # timeout we fail loud so the quit phase still runs (workflow `if: always()`).
    [int] $TimeoutSeconds = 900,
    [int] $PollSeconds = 3,
    # Emit a heartbeat line every this-many seconds so a watching owner sees the
    # gate is alive and what it is still waiting for.
    [int] $HeartbeatSeconds = 20
)

. "$PSScriptRoot/_common.ps1"

if (-not $GoFile) {
    # Default sentinel in the runner's temp dir. The owner creates it with e.g.
    #   New-Item -ItemType File $env:TEMP\poly-cdp-go.txt
    $GoFile = Join-Path $env:TEMP "poly-cdp-go.txt"
}

Write-PolyPhase -Phase "await-manual" -State "start" `
    -Extra @{ cdpPort = $CdpPort; goFile = $GoFile; timeoutSeconds = $TimeoutSeconds }

# Clear a stale sentinel from a prior run so we require a FRESH one for this run
# (else a leftover file would let the gate pass before the owner even acts).
if (Test-Path $GoFile) {
    Remove-Item -Path $GoFile -Force -ErrorAction SilentlyContinue
    Write-Host "[cubase:await-manual] removed stale go-file $GoFile"
}

# The operator prompt — printed once, prominently, in the live workflow log.
# CRITICAL: the CDP port only binds when the env var is set in the SAME shell
# that launches Cubase, BEFORE launch (the child inherits it) — this is the
# #215 recipe, the one path that actually exposes CDP. Just double-clicking the
# .cpr will NOT bring the port up.
$fixtureLine = if ($FixtureCpr) { $FixtureCpr } else { "(no fixture set — open your Poly project)" }
$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$launchScript = Join-Path $repoRoot "scripts\cubase\launch-manual-cdp.ps1"
Write-Host ""
Write-Host "================ MANUAL CUBASE STEP (S09 CDP) ================"
Write-Host " The CI build + install is done. In your VNC session, run the"
Write-Host " version-controlled launch script from a PowerShell window:"
Write-Host ""
Write-Host "     & '$launchScript' -CdpPort $CdpPort"
Write-Host ""
Write-Host " It sets WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS, launches Cubase on the"
Write-Host " fixture ($fixtureLine),"
Write-Host " waits for the CDP port, then drops the sentinel this gate polls for."
Write-Host " (Open the Poly editor window if it does not open on its own.)"
Write-Host ""
Write-Host " Prefer that script over hand-typing — the env var MUST be set in the"
Write-Host " SAME shell before Cubase launches, or the CDP port never binds."
Write-Host ""
Write-Host " This gate waits for BOTH the sentinel AND a live CDP listener on"
Write-Host " 127.0.0.1:$CdpPort, then the e2e proceeds. Times out in ${TimeoutSeconds}s."
Write-Host "============================================================="
Write-Host ""

# True when something is LISTENING on 127.0.0.1:<port>. Reads the OS TCP table
# (focus-independent) rather than connecting.
function Test-CdpListening {
    param([int] $Port)
    $rows = Get-NetTCPConnection -State Listen -ErrorAction SilentlyContinue |
        Where-Object { $_.LocalPort -eq $Port }
    return [bool]$rows
}

# Resolve the owner's Cubase pid so downstream steps (focus-editor-cdp /
# diagnose-editor-window) can scope to THIS Cubase rather than any Cubase-named
# process. Publish it the same way launch-cubase.ps1 does.
function Publish-CubasePid {
    $names = Get-CubaseProcessNames
    $proc = Get-Process -Name $names -ErrorAction SilentlyContinue |
        Where-Object { $_.MainWindowHandle -ne 0 } |
        Select-Object -First 1
    if (-not $proc) { return }
    if ($env:GITHUB_ENV) {
        Add-Content -Path $env:GITHUB_ENV -Value "POLY_CUBASE_PID=$($proc.Id)"
    }
    $env:POLY_CUBASE_PID = "$($proc.Id)"
    $pidPath = Join-Path (Get-PolyArtifactDir) "cubase-pid.txt"
    Set-Content -Path $pidPath -Value "$($proc.Id)"
    Write-PolyPhase -Phase "await-manual" -State "ok" `
        -Detail "resolved owner-launched Cubase" -Extra @{ pid = $proc.Id }
}

try {
    $start = Get-Date
    $deadline = $start.AddSeconds($TimeoutSeconds)
    $lastHeartbeat = $start

    while ((Get-Date) -lt $deadline) {
        $haveGo = Test-Path $GoFile
        $haveCdp = Test-CdpListening -Port $CdpPort

        if ($haveGo -and $haveCdp) {
            Publish-CubasePid
            $elapsed = [int]((Get-Date) - $start).TotalSeconds
            Write-PolyPhase -Phase "await-manual" -State "ok" `
                -Detail "owner signalled go and CDP is live on 127.0.0.1:$CdpPort" `
                -Extra @{ cdpPort = $CdpPort; elapsedSeconds = $elapsed }
            return
        }

        if (((Get-Date) - $lastHeartbeat).TotalSeconds -ge $HeartbeatSeconds) {
            $waiting = @()
            if (-not $haveGo)  { $waiting += "sentinel($GoFile)" }
            if (-not $haveCdp) { $waiting += "CDP(127.0.0.1:$CdpPort)" }
            $remain = [int](($deadline - (Get-Date)).TotalSeconds)
            Write-Host "[cubase:await-manual] still waiting for: $($waiting -join ', ')  (${remain}s left)"
            $lastHeartbeat = Get-Date
        }
        Start-Sleep -Seconds $PollSeconds
    }

    # Timeout — fail loud so the runner is never pinned by a forgotten dispatch.
    $haveGo = Test-Path $GoFile
    $haveCdp = Test-CdpListening -Port $CdpPort
    Invoke-PolyPhaseFailure -Phase "await-manual" `
        -Message "manual Cubase not ready within ${TimeoutSeconds}s (sentinel=$haveGo, cdp=$haveCdp). Open Cubase on the fixture, confirm the Poly editor, and create $GoFile, then re-dispatch." `
        -Extra @{ cdpPort = $CdpPort; sentinelPresent = $haveGo; cdpListening = $haveCdp }
} catch {
    if ($_.Exception.Message -notmatch "await-manual failed") {
        Invoke-PolyPhaseFailure -Phase "await-manual" -Message $_.Exception.Message
    }
    throw
}
