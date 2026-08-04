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

    # Resolve the artifact dir to a canonical full path so the "already staged"
    # check below is robust to relative paths / trailing slashes.
    $artifactFull = (Resolve-Path -Path $ArtifactDir).ProviderPath

    # Copy a source tree/file into the artifact dir if it exists; warn if not.
    function Copy-IfPresent {
        param([string] $Source, [string] $Label)
        if (-not ($Source -and (Test-Path $Source))) {
            Write-PolyPhase -Phase "archive" -State "ok" `
                -Detail "no $Label to collect (skipped)" -Extra @{ source = $Source }
            return
        }

        # Skip anything that already lives inside the artifact dir — the probe
        # JSONL (POLY_PROBE_OUTPUT) and the _common.ps1 status/last-error files
        # are written straight into $ArtifactDir, so they are already staged.
        # Copying them onto themselves throws "Cannot overwrite the item ...
        # with itself" and, historically, failed the whole archive step.
        $sourceFull = (Resolve-Path -Path $Source).ProviderPath
        if ($sourceFull -eq $artifactFull -or
            $sourceFull.StartsWith($artifactFull + [System.IO.Path]::DirectorySeparatorChar,
                [System.StringComparison]::OrdinalIgnoreCase)) {
            Write-PolyPhase -Phase "archive" -State "ok" `
                -Detail "$Label already in artifact dir (already staged)" `
                -Extra @{ source = $Source }
            return
        }

        $destName = Split-Path -Leaf $Source
        Copy-Item -Recurse -Force -Path $Source -Destination (Join-Path $ArtifactDir $destName)
        Write-PolyPhase -Phase "archive" -State "ok" `
            -Detail "collected $Label" -Extra @{ source = $Source }
    }

    # Cubase per-user log/crash directory (stable Windows layout).
    $cubaseAppData = Join-Path $env:APPDATA "Steinberg\Cubase $CubaseVersion`_64"
    Copy-IfPresent -Source $cubaseAppData -Label "Cubase prefs/logs"

    $crashDir = Join-Path $env:LOCALAPPDATA "Steinberg\CrashDumps"
    Copy-IfPresent -Source $crashDir -Label "Steinberg crash dumps"

    # Probe output (S08 populates this; in S07 it is typically absent).
    Copy-IfPresent -Source $env:POLY_PROBE_OUTPUT -Label "probe JSONL"

    Write-PolyPhase -Phase "archive" -State "ok" -Detail "archive complete"
} catch {
    # Archive is a best-effort diagnostics collector run under the workflow's
    # `if: always()`. It must never fail the job — a failure here would mask the
    # real result of the launch/quit smoke it exists to diagnose. Record the
    # error (Invoke-PolyPhaseFailure persists cubase-last-error.json) but do NOT
    # rethrow; swallow it so the step exits 0.
    try {
        Invoke-PolyPhaseFailure -Phase "archive" -Message $_.Exception.Message
    } catch {
        # Invoke-PolyPhaseFailure ends with `throw`; absorb it here.
        Write-Warning "[cubase:archive] non-fatal: $($_.Exception.Message)"
    }
}
