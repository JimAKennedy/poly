# M042 S07: collect Cubase + runner diagnostics into the artifact staging dir.
#
# Runs under the workflow's `if: always()` so artifacts are collected on both
# success and failure. Gathers, best-effort (a missing source is a warning, not
# a failure): Cubase's own logs/crash dumps, the probe JSONL (if S08 produced
# it), and any screen recording. The phase status JSONL and last-error file
# written by _common.ps1 already live in the artifact dir, so they upload too.

[CmdletBinding()]
param(
    [Parameter(Mandatory)] [string] $CubaseVersion,
    [Parameter(Mandatory)] [string] $ArtifactDir
)

. "$PSScriptRoot/_common.ps1"

Write-PolyPhase -Phase "archive" -State "start" -Extra @{ artifactDir = $ArtifactDir }

try {
    New-Item -ItemType Directory -Force -Path $ArtifactDir | Out-Null

    # Copy a source tree/file into the artifact dir if it exists; warn if not.
    function Copy-IfPresent {
        param([string] $Source, [string] $Label)
        if ($Source -and (Test-Path $Source)) {
            $destName = Split-Path -Leaf $Source
            $dest = Join-Path $ArtifactDir $destName
            $srcFull = (Resolve-Path $Source).Path
            $destFull = [System.IO.Path]::GetFullPath($dest)
            if ($srcFull -eq $destFull) {
                Write-PolyPhase -Phase "archive" -State "ok" `
                    -Detail "$Label already in artifact dir (skipped copy)" -Extra @{ source = $Source }
            } else {
                Copy-Item -Recurse -Force -Path $Source -Destination $dest
                Write-PolyPhase -Phase "archive" -State "ok" `
                    -Detail "collected $Label" -Extra @{ source = $Source }
            }
        } else {
            Write-PolyPhase -Phase "archive" -State "ok" `
                -Detail "no $Label to collect (skipped)" -Extra @{ source = $Source }
        }
    }

    # Cubase per-user log/crash directory (stable Windows layout).
    $cubaseAppData = Join-Path $env:APPDATA "Steinberg\Cubase $CubaseVersion`_64"
    Copy-IfPresent -Source $cubaseAppData -Label "Cubase prefs/logs"

    $crashDir = Join-Path $env:LOCALAPPDATA "Steinberg\CrashDumps"
    Copy-IfPresent -Source $crashDir -Label "Steinberg crash dumps"

    # Probe output (S08 populates this; in S07 it is typically absent).
    Copy-IfPresent -Source $env:POLY_PROBE_OUTPUT -Label "probe JSONL"

    # Probe diagnostic sidecar. The probe always writes probe-status.txt next to
    # POLY_PROBE_OUTPUT (probe.jsonl -> probe-status.txt) recording what it saw:
    # env var, process-call count, event count, whether the stop edge fired, last
    # flush result. It lands even when probe.jsonl does not, so a silent no-flush
    # is diagnosable from the artifact instead of another runner cycle. Emit a
    # dedicated archive line so its presence/absence is visible in the job log.
    if ($env:POLY_PROBE_OUTPUT) {
        $probeStatus = [System.IO.Path]::ChangeExtension($env:POLY_PROBE_OUTPUT, $null) + "-status.txt"
        Copy-IfPresent -Source $probeStatus -Label "probe status sidecar"
    }

    Write-PolyPhase -Phase "archive" -State "ok" -Detail "archive complete"
} catch {
    # Archive is a best-effort diagnostics collector run under the workflow's
    # `if: always()`. It must never fail the job — a failure here would mask the
    # real result of the launch/quit smoke it exists to diagnose. Record the
    # error (Invoke-PolyPhaseFailure persists cubase-last-error.json) but do NOT
    # let its trailing `throw` propagate; swallow it so the step exits 0.
    $archiveError = $_.Exception.Message
    try {
        Invoke-PolyPhaseFailure -Phase "archive" -Message $archiveError
    } catch {
        # Emit a GitHub ::warning:: annotation (not just Write-Warning, which is
        # invisible in the run summary) so a real archive failure is surfaced in
        # the run UI even though the step deliberately exits 0.
        Write-Host "::warning title=Cubase archive::[cubase:archive] non-fatal failure: $archiveError"
    }
}
