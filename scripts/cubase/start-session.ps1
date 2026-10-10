# M004: start one Cubase session for an in-DAW spec, end to end.
#
# The nightly's first session is assembled from the individual steps in
# .github/workflows/cubase-nightly.yml. The M004 specs each need a session of
# their own -- S01 saves and reopens a project, S05 opens a different fixture,
# S06 bounces offline, S04 and S07 change host state a later spec would inherit
# -- and repeating seven workflow steps per session would make the workflow the
# hardest thing in the repo to read. So the same steps, in the same order, live
# here, and the workflow calls this once per session.
#
# Each session writes under <ArtifactDir>/<Name>/: its probe capture, and, with
# -CopyFixture, a scratch copy of the fixture's folder. A session that may save
# MUST use -CopyFixture -- tests/cubase/fixtures/README.md requires that a run
# never write over a committed fixture -- and a copy opened from a new folder
# raises Cubase's "project file has been moved" prompt, which
# dismiss-moved-project.ps1 answers.
#
# Quit is NOT part of this script: the spec runs between start and quit, and the
# caller ends the session with quit-cubase.ps1 exactly as the first session does.

[CmdletBinding()]
param(
    [Parameter(Mandatory)] [string] $Name,
    [Parameter(Mandatory)] [string] $FixtureCpr,
    # Copy the fixture's whole folder (the .cpr alone is not a project: Cubase
    # also wants its Audio/ folder) into <ArtifactDir>/<Name>/project/ and open
    # the copy. Without it, FixtureCpr is opened in place.
    [switch] $CopyFixture,
    [string] $CubaseVersion = "14",
    [int] $CdpPort = 9222
)

. "$PSScriptRoot/_common.ps1"

$sessionDir = Join-Path (Get-PolyArtifactDir) $Name
New-Item -ItemType Directory -Force -Path $sessionDir | Out-Null

$cpr = $FixtureCpr
if ($CopyFixture) {
    $projectDir = Join-Path $sessionDir "project"
    if (Test-Path $projectDir) { Remove-Item -Recurse -Force $projectDir }
    Copy-Item -Recurse -Path (Split-Path -Parent $FixtureCpr) -Destination $projectDir
    $cpr = Join-Path $projectDir (Split-Path -Leaf $FixtureCpr)
}

# Publish where this session's files are, for the spec and the caller.
$probe = Join-Path $sessionDir "probe.jsonl"
if ($env:GITHUB_ENV) {
    Add-Content -Path $env:GITHUB_ENV -Value "POLY_SESSION_DIR=$sessionDir"
    Add-Content -Path $env:GITHUB_ENV -Value "POLY_SESSION_CPR=$cpr"
}
$env:POLY_SESSION_DIR = $sessionDir
$env:POLY_SESSION_CPR = $cpr

Write-PolyPhase -Phase "start-session" -State "start" `
    -Extra @{ name = $Name; cpr = $cpr; probe = $probe }

function Invoke-Step([string] $Script, [hashtable] $Arguments = @{}) {
    # _common.ps1 sets strict mode, under which reading an unset $LASTEXITCODE
    # throws; a script that never calls `exit` leaves it unset, so seed it.
    $global:LASTEXITCODE = 0
    & (Join-Path $PSScriptRoot $Script) @Arguments
    if ($LASTEXITCODE -ne 0) {
        Invoke-PolyPhaseFailure -Phase "start-session" `
            -Message "$Script failed (exit $LASTEXITCODE)" -Extra @{ name = $Name }
    }
}

Invoke-Step "kill-stale-cubase.ps1"
Invoke-Step "clear-safe-mode-flag.ps1" @{ CubaseVersion = $CubaseVersion }
Invoke-Step "launch-cubase.ps1" @{
    CubaseVersion = $CubaseVersion
    FixtureCpr    = $cpr
    ProbeOutput   = $probe
    EnableCdp     = $true
    CdpPort       = $CdpPort
}
Invoke-Step "dismiss-safe-mode.ps1" @{ TimeoutSeconds = 10 }
Invoke-Step "dismiss-moved-project.ps1" @{
    ProjectDir     = (Split-Path -Parent $cpr)
    TimeoutSeconds = $(if ($CopyFixture) { 30 } else { 5 })
}
# What the desktop looks like once the prompts are answered. Every M004
# session failed in the scheduled nightlies of 2026-10-09 and -10 with the
# editor's CDP port never opening, after a moved-project prompt the log says
# was answered; the same sessions had passed in daytime dispatches. Whether
# that click landed is visible here and nowhere else.
& (Join-Path $PSScriptRoot "capture-desktop.ps1") -OutputPath (Join-Path $sessionDir "desktop-after-prompts.png")

try {
    Invoke-Step "wait-for-ready.ps1" @{ TimeoutSeconds = 120 }
    Invoke-Step "focus-editor-cdp.ps1" @{ CdpPort = $CdpPort; TimeoutSeconds = 60 }
} catch {
    # The failure is re-raised; these only record what was on screen and what
    # windows existed when it happened, into the session's artifact folder.
    & (Join-Path $PSScriptRoot "capture-desktop.ps1") -OutputPath (Join-Path $sessionDir "desktop-at-failure.png")
    try {
        & (Join-Path $PSScriptRoot "diagnose-editor-window.ps1") -CdpPort $CdpPort `
            -OutputPath (Join-Path $sessionDir "editor-window-topology.txt") | Out-Null
    } catch {
        Write-PolyPhase -Phase "start-session" -State "ok" `
            -Detail "editor-window diagnostic failed: $($_.Exception.Message)"
    }
    throw
}

Write-PolyPhase -Phase "start-session" -State "ok" -Detail "session ready" `
    -Extra @{ name = $Name; cpr = $cpr }
