# M042 S09: operator-run script for the manual-CDP flow.
#
# Run this by hand in the runner's VNC session AFTER a dispatch with
# `-f cdp_port=9222 -f manual_cubase=true` reaches the "Await manual Cubase
# (CDP)" gate. It does, in one step, what the gate's printed recipe asks the
# operator to do by hand:
#   1. set WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS=--remote-debugging-port=<port>
#      in THIS shell (so the Cubase child inherits it -- the #215 recipe, the one
#      path that actually exposes CDP; double-clicking the .cpr does NOT),
#   2. launch Cubase on the freshly-checked-out fixture,
#   3. wait for the Poly editor to materialise AND the CDP port to listen, then
#   4. drop the sentinel file the gate is polling for.
#
# Why a script (not hand-typed): the env var MUST be set in the same shell that
# launches Cubase, before launch. Typing it by hand is error-prone (a missed
# env-var-before-launch silently yields no CDP port and a confusing timeout).
# This keeps the recipe under version control so it always matches the workflow
# (same port default, same fixture path, same sentinel path as
# await-manual-cubase.ps1).
#
# It does NOT wait for the CI job -- it just gets Cubase into the CDP-ready state
# and signals go. The CI gate (await-manual-cubase.ps1) then detects the sentinel
# + live port and proceeds with the e2e.

[CmdletBinding()]
param(
    # CDP remote-debugging port -- must match the dispatch's `-f cdp_port` and the
    # gate's -CdpPort (default 9222).
    [int] $CdpPort = 9222,
    # Cubase major version installed on the runner (matches POLY_CUBASE_VERSION).
    [string] $CubaseVersion = "14",
    # Fixture to open. Defaults to the runner's checked-out fixture so the manual
    # launch always uses the freshly-installed plugin's test project, not a stale
    # hand clone. Override for a local dry-run.
    [string] $FixtureCpr = "C:\actions-runner\_work\poly\poly\tests\cubase\fixtures\poly-4bar.cpr",
    # Sentinel the CI gate polls for. MUST match await-manual-cubase.ps1's default
    # ($env:TEMP\poly-cdp-go.txt) so the gate sees it.
    [string] $GoFile = "",
    # How long to wait for the editor + CDP port before giving up (and NOT
    # dropping the sentinel, so the gate never proceeds against a half-ready
    # Cubase).
    [int] $TimeoutSeconds = 120,
    [int] $PollSeconds = 2
)

. "$PSScriptRoot/_common.ps1"

if (-not $GoFile) {
    $GoFile = Join-Path $env:TEMP "poly-cdp-go.txt"
}

$exe = Get-CubaseExePath -CubaseVersion $CubaseVersion
if (-not $exe) {
    throw "Cubase $CubaseVersion executable not found under C:\Program Files\Steinberg."
}
if (-not (Test-Path $FixtureCpr)) {
    throw "Fixture not found: $FixtureCpr"
}

# Remove any stale sentinel first so the gate never sees a leftover from a prior
# run before this Cubase is actually CDP-ready.
if (Test-Path $GoFile) {
    Remove-Item -Path $GoFile -Force -ErrorAction SilentlyContinue
    Write-Host "[launch-manual-cdp] removed stale go-file $GoFile"
}

# STEP 1: set the CDP arg in THIS shell BEFORE launch. Start-Process inherits the
# current process environment, so the launched Cubase (and the WebView2 child it
# spawns) sees it. This is the load-bearing line -- without it the port never
# binds.
$env:WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS = "--remote-debugging-port=$CdpPort"
Write-Host "[launch-manual-cdp] WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS = $env:WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS"

# STEP 2: launch Cubase on the fixture.
Write-Host "[launch-manual-cdp] launching Cubase $CubaseVersion on $FixtureCpr"
$proc = Start-Process -FilePath $exe -ArgumentList "`"$FixtureCpr`"" -PassThru
Write-Host "[launch-manual-cdp] launched pid $($proc.Id) -- open the Poly editor if it is not already open"

# STEP 3: wait for the CDP port to listen. The editor's WebView2 exposes the port
# only once it has materialised; this poll is the real readiness signal.
function Test-CdpListening {
    param([int] $Port)
    $rows = Get-NetTCPConnection -State Listen -ErrorAction SilentlyContinue |
        Where-Object { $_.LocalPort -eq $Port }
    return [bool]$rows
}

$deadline = (Get-Date).AddSeconds($TimeoutSeconds)
$listening = $false
while ((Get-Date) -lt $deadline) {
    if (Test-CdpListening -Port $CdpPort) {
        $listening = $true
        break
    }
    Start-Sleep -Seconds $PollSeconds
}

if (-not $listening) {
    Write-Host "::error::[launch-manual-cdp] CDP port $CdpPort never came up within ${TimeoutSeconds}s."
    Write-Host "  Confirm the Poly editor window is open. If it is and the port is still"
    Write-Host "  absent, quit Cubase and re-run this script (the env var must be set"
    Write-Host "  before Cubase launches). NOT dropping the sentinel -- the gate will not"
    Write-Host "  proceed against a Cubase with no CDP port."
    exit 1
}

# STEP 4: CDP is live -- drop the sentinel so the CI gate proceeds.
New-Item -ItemType File -Force -Path $GoFile | Out-Null
Write-Host "[launch-manual-cdp] CDP listening on 127.0.0.1:$CdpPort -- dropped sentinel $GoFile"
Write-Host "[launch-manual-cdp] the CI 'Await manual Cubase (CDP)' gate should now proceed to the e2e."
