# M042 S09: hand-launch Cubase with the CDP port open, for diagnosis.
#
# THE NIGHTLY NO LONGER NEEDS THIS. The workflow launches Cubase itself and the
# CDP port comes up unattended, because the runner's logon task now runs
# NON-elevated (docs/windows-runner-rehome-and-deelevate.md Part C3). WebView2
# discards browser flags set via the environment or registry whenever the host
# app is elevated, which is why the automated launch used to yield no CDP port
# while this hand-run script always worked -- the differentiator was integrity
# level, not the human. The manual-CDP gate (await-manual-cubase.ps1) and its
# `manual_cubase` dispatch input are gone.
#
# Keep this for reproducing the editor/CDP state by hand outside a CI run. In one
# step it:
#   1. sets WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS=--remote-debugging-port=<port>
#      in THIS shell (so the Cubase child inherits it -- an env var set after
#      launch has no effect),
#   2. sets POLY_PROBE_OUTPUT so poly_midi_probe has somewhere to flush its JSONL,
#   3. clears Cubase's Safe Mode flag, launches Cubase on the fixture, and
#   4. waits for the CDP port to listen, failing loud if it never does.
#
# RUN IT FROM A NORMAL, NON-ELEVATED POWERSHELL. From an elevated shell Cubase
# inherits the elevation and the port can never bind -- the script refuses to
# start in that case rather than leaving you with a confusing timeout.
#
# The sentinel file below is vestigial (nothing polls for it now that the CI gate
# is gone); it is kept because it is harmless and makes a hand run observable.

[CmdletBinding()]
param(
    # CDP remote-debugging port to open (matches the workflow's cdp_port input).
    [int] $CdpPort = 9222,
    # Cubase major version installed on the runner (matches POLY_CUBASE_VERSION).
    [string] $CubaseVersion = "14",
    # Fixture to open. Defaults to the runner's checked-out fixture so the manual
    # launch always uses the freshly-installed plugin's test project, not a stale
    # hand clone. Override for a local dry-run.
    [string] $FixtureCpr = "C:\actions-runner\_work\poly\poly\tests\cubase\fixtures\poly-4bar.cpr",
    # Where poly_midi_probe flushes its JSONL on Cubase deactivate. This MUST be
    # set in THIS shell before launch so the Cubase child inherits it -- the probe
    # reads getenv("POLY_PROBE_OUTPUT") and writes nowhere if it is absent, which
    # is exactly what happened in run #53 (e2e green, but no probe.jsonl -> the
    # compare-to-golden step failed with "No such file"). The default matches the
    # workflow's POLY_PROBE_OUTPUT (github.workspace\_artifacts\probe.jsonl) so the
    # CI compare step finds it. An existing $env:POLY_PROBE_OUTPUT in this shell
    # wins (non-standard runner layouts / local dry-runs).
    [string] $ProbeOutput = "",
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

# Refuse to run elevated. WebView2 ignores browser flags delivered via the
# environment or the registry when the host app is elevated, so an elevated
# launch cannot expose CDP no matter what this script does -- better to say so
# now than to time out 120s later. See docs/windows-test-runner-setup.md Part 12.
$isElevated = ([Security.Principal.WindowsPrincipal]`
    [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator)
if ($isElevated) {
    throw ("This shell is ELEVATED. WebView2 ignores WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS " +
        "for elevated host apps, so the CDP port would never bind. Re-run from a normal " +
        "(non-elevated) PowerShell window.")
}

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

# Resolve the probe output path. Precedence: explicit -ProbeOutput arg, then an
# existing $env:POLY_PROBE_OUTPUT already in this shell, then the CI default
# (derived from the fixture's workspace root: <workspace>\_artifacts\probe.jsonl,
# matching the workflow's POLY_PROBE_OUTPUT). The fixture lives at
# <workspace>\tests\cubase\fixtures\poly-4bar.cpr, so the workspace root is four
# levels up.
if (-not $ProbeOutput) {
    if ($env:POLY_PROBE_OUTPUT) {
        $ProbeOutput = $env:POLY_PROBE_OUTPUT
    } else {
        $workspace = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $FixtureCpr)))
        $ProbeOutput = Join-Path $workspace "_artifacts\probe.jsonl"
    }
}

# Remove any stale sentinel first so the gate never sees a leftover from a prior
# run before this Cubase is actually CDP-ready.
if (Test-Path $GoFile) {
    Remove-Item -Path $GoFile -Force -ErrorAction SilentlyContinue
    Write-Host "[launch-manual-cdp] removed stale go-file $GoFile"
}

# Clear Cubase's Safe Mode flag before launch (same root-cause fix as
# clear-safe-mode-flag.ps1, which the CI chain SKIPS in the manual-CDP flow).
# Cubase writes a zero-byte ApplicationStarted.txt on startup and deletes it on a
# clean exit; every quit here hard-kills Cubase (the Hub blocks a clean exit), so
# the sentinel is always left behind and the next launch pops the modal Safe Mode
# recovery dialog BEFORE loading the fixture. Deleting it makes Cubase see a clean
# prior shutdown and skip the dialog, so the operator no longer has to click
# through it. Best-effort: a missing prefs dir or absent flag is the happy path.
$prefsDir = Join-Path $env:APPDATA "Steinberg\Cubase $CubaseVersion`_64"
$safeModeFlag = Join-Path $prefsDir "ApplicationStarted.txt"
if (Test-Path $safeModeFlag) {
    Remove-Item -Path $safeModeFlag -Force -ErrorAction SilentlyContinue
    Write-Host "[launch-manual-cdp] cleared stale Safe Mode flag $safeModeFlag (Cubase will skip the recovery dialog)"
} else {
    Write-Host "[launch-manual-cdp] no Safe Mode flag present (clean prior shutdown, nothing to clear)"
}

# STEP 1: set the CDP arg AND the probe output path in THIS shell BEFORE launch.
# Start-Process inherits the current process environment, so the launched Cubase
# (and the WebView2 child it spawns) sees both. These are the load-bearing lines:
#   - WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS: without it the CDP port never binds.
#   - POLY_PROBE_OUTPUT: without it poly_midi_probe (running INSIDE this Cubase)
#     reads getenv and finds nothing, so it writes no JSONL on deactivate and the
#     CI compare-to-golden step fails with "No such file" (run #53). The automated
#     launch-cubase.ps1 sets this the same way; the manual flow skips that script,
#     so we must set it here.
$env:WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS = "--remote-debugging-port=$CdpPort"
Write-Host "[launch-manual-cdp] WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS = $env:WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS"

$probeDir = Split-Path -Parent $ProbeOutput
if ($probeDir) { New-Item -ItemType Directory -Force -Path $probeDir | Out-Null }
$env:POLY_PROBE_OUTPUT = $ProbeOutput
Write-Host "[launch-manual-cdp] POLY_PROBE_OUTPUT = $env:POLY_PROBE_OUTPUT"

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
