# M042 S07: launch Cubase with the fixture project (or empty, in S07).
#
# Cubase has no CLI transport/automation API (testing-strategy.md §3.1) — the
# only supported invocation is file-association open: `Cubase <path>.cpr`. This
# script resolves the Cubase.exe for the target version, exports POLY_PROBE_OUTPUT
# (so poly_midi_probe, S06, flushes JSONL on deactivate), and starts the process
# detached. It does NOT wait for readiness — that is wait-for-ready.ps1 — so the
# launch and readiness concerns stay separable and independently diagnosable.

[CmdletBinding()]
param(
    [Parameter(Mandatory)] [string] $CubaseVersion,
    # S07: empty string => launch Cubase with no project. S08 passes the
    # committed fixture .cpr here.
    [string] $FixtureCpr = "",
    [string] $ProbeOutput = ""
)

. "$PSScriptRoot/_common.ps1"

Write-PolyPhase -Phase "launch" -State "start" `
    -Detail "Cubase $CubaseVersion" -Extra @{ fixture = $FixtureCpr }

try {
    $exe = Get-CubaseExePath -CubaseVersion $CubaseVersion
    if (-not $exe) {
        Invoke-PolyPhaseFailure -Phase "launch" `
            -Message "Cubase $CubaseVersion executable not found under C:\Program Files\Steinberg" `
            -Extra @{ cubaseVersion = $CubaseVersion }
    }

    # Export the probe output path for the child process to inherit.
    if ($ProbeOutput) {
        $probeDir = Split-Path -Parent $ProbeOutput
        if ($probeDir) { New-Item -ItemType Directory -Force -Path $probeDir | Out-Null }
        $env:POLY_PROBE_OUTPUT = $ProbeOutput
        Write-PolyPhase -Phase "launch" -State "ok" `
            -Detail "POLY_PROBE_OUTPUT set" -Extra @{ probeOutput = $ProbeOutput }
    }

    if ($FixtureCpr) {
        if (-not (Test-Path $FixtureCpr)) {
            Invoke-PolyPhaseFailure -Phase "launch" `
                -Message "fixture .cpr not found" -Extra @{ fixture = $FixtureCpr }
        }
        $proc = Start-Process -FilePath $exe -ArgumentList "`"$FixtureCpr`"" -PassThru
    } else {
        # No project: Cubase opens to its default state (Hub disabled per
        # runbook Part 5, so this lands on an empty project).
        $proc = Start-Process -FilePath $exe -PassThru
    }

    Write-PolyPhase -Phase "launch" -State "ok" `
        -Detail "launched" -Extra @{ pid = $proc.Id; exe = $exe }
} catch {
    Invoke-PolyPhaseFailure -Phase "launch" -Message $_.Exception.Message
}
