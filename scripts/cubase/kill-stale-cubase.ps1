# M042 S07: terminate any lingering Cubase process before a run.
#
# A previous run that failed to quit cleanly leaves Cubase holding the audio
# device / MIDI ports / project lock. Kill it first so the launch phase starts
# from a known-clean state. Idempotent — a no-op when nothing is running.

[CmdletBinding()]
param()

. "$PSScriptRoot/_common.ps1"

Write-PolyPhase -Phase "kill-stale" -State "start"

try {
    $names = Get-CubaseProcessNames
    $procs = Get-Process -Name $names -ErrorAction SilentlyContinue
    if (-not $procs) {
        Write-PolyPhase -Phase "kill-stale" -State "ok" -Detail "no stale Cubase process"
        return
    }

    foreach ($p in $procs) {
        try {
            Stop-Process -Id $p.Id -Force -ErrorAction Stop
            Write-PolyPhase -Phase "kill-stale" -State "ok" `
                -Detail "terminated $($p.ProcessName)" -Extra @{ pid = $p.Id }
        } catch {
            # Non-fatal: one process may already be gone by the time we reach it.
            Write-PolyPhase -Phase "kill-stale" -State "ok" `
                -Detail "process $($p.Id) already exited or unkillable" -Extra @{ pid = $p.Id }
        }
    }
} catch {
    Invoke-PolyPhaseFailure -Phase "kill-stale" -Message $_.Exception.Message
}
