# M042 S07: shared helpers for the Cubase launch/quit machinery.
#
# Dot-source this at the top of each phase script:
#   . "$PSScriptRoot/_common.ps1"
#
# Provides structured phase logging and durable status/error persistence so an
# unattended nightly failure is diagnosable from artifacts alone (observability
# discipline: log decisions, fail loud, persist the reason). Nothing here
# touches Cubase — it is pure plumbing reused by every phase script.

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# Where phase status is persisted. Defaults to a repo-local _artifacts dir so a
# local dry-run works without the workflow env; the workflow sets
# POLY_ARTIFACT_DIR to the run's staging dir.
function Get-PolyArtifactDir {
    if ($env:POLY_ARTIFACT_DIR) { return $env:POLY_ARTIFACT_DIR }
    return (Join-Path (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)) "_artifacts")
}

function Get-PolyStatusPath {
    return (Join-Path (Get-PolyArtifactDir) "cubase-run-status.jsonl")
}

# One structured JSONL line per phase transition. `phase` is the script name
# (kill-stale / launch / wait-ready / quit / archive); `state` is one of
# start / ok / fail; `detail` is a short human note; extra is a hashtable of
# domain fields (pid, path, elapsedSeconds, ...).
function Write-PolyPhase {
    param(
        [Parameter(Mandatory)] [string] $Phase,
        [Parameter(Mandatory)] [ValidateSet("start", "ok", "fail")] [string] $State,
        [string] $Detail = "",
        [hashtable] $Extra = @{}
    )
    $dir = Get-PolyArtifactDir
    New-Item -ItemType Directory -Force -Path $dir | Out-Null

    $record = [ordered]@{
        ts     = (Get-Date).ToUniversalTime().ToString("o")
        phase  = $Phase
        state  = $State
        detail = $Detail
    }
    foreach ($k in $Extra.Keys) { $record[$k] = $Extra[$k] }

    $json = ($record | ConvertTo-Json -Compress -Depth 5)
    Add-Content -Path (Get-PolyStatusPath) -Value $json

    # Also echo to the workflow log so it is visible in the live run, not only
    # in the archived artifact.
    $marker = switch ($State) { "fail" { "::error::" } default { "" } }
    Write-Host "$marker[cubase:$Phase] $State $Detail"
}

# Persist a terminal failure to a dedicated file a cold-start reader checks
# first, then rethrow so the workflow step fails loud (no swallowing).
function Invoke-PolyPhaseFailure {
    param(
        [Parameter(Mandatory)] [string] $Phase,
        [Parameter(Mandatory)] [string] $Message,
        [hashtable] $Extra = @{}
    )
    Write-PolyPhase -Phase $Phase -State "fail" -Detail $Message -Extra $Extra
    $errPath = Join-Path (Get-PolyArtifactDir) "cubase-last-error.json"
    $payload = [ordered]@{
        ts      = (Get-Date).ToUniversalTime().ToString("o")
        phase   = $Phase
        message = $Message
    }
    foreach ($k in $Extra.Keys) { $payload[$k] = $Extra[$k] }
    Set-Content -Path $errPath -Value ($payload | ConvertTo-Json -Depth 5)
    throw "$Phase failed: $Message"
}

# Resolve the Cubase executable path for a given major version. Windows install
# layout is stable: C:\Program Files\Steinberg\Cubase <ver>\Cubase<ver>.exe.
# Returns $null if not found (caller decides whether that is fatal).
function Get-CubaseExePath {
    param([Parameter(Mandatory)] [string] $CubaseVersion)
    $candidates = @(
        "C:\Program Files\Steinberg\Cubase $CubaseVersion\Cubase$CubaseVersion.exe",
        "C:\Program Files\Steinberg\Cubase $CubaseVersion\Cubase.exe"
    )
    foreach ($c in $candidates) {
        if (Test-Path $c) { return $c }
    }
    return $null
}

# All Cubase process names we might need to match, version-agnostic.
function Get-CubaseProcessNames {
    return @("Cubase", "Cubase14", "Cubase13", "Cubase12")
}
