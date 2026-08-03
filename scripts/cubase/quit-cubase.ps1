# M042 S07: quit Cubase cleanly, with a hard-kill fallback.
#
# Preferred path: ask each Cubase window to close (CloseMainWindow), which lets
# Cubase release the audio device / MIDI ports / project lock gracefully. If a
# process refuses to exit within a grace period, hard-kill it — a leftover
# Cubase would block the next nightly (kill-stale would catch it, but leaving a
# hung process is a failure signal worth surfacing here).
#
# Runs under the workflow's `if: always()` so it executes even when
# wait-for-ready failed, ensuring the runner is left clean.

[CmdletBinding()]
param(
    [int] $GraceSeconds = 15
)

. "$PSScriptRoot/_common.ps1"

Write-PolyPhase -Phase "quit" -State "start"

try {
    $names = Get-CubaseProcessNames
    $procs = Get-Process -Name $names -ErrorAction SilentlyContinue
    if (-not $procs) {
        Write-PolyPhase -Phase "quit" -State "ok" -Detail "no Cubase process to quit"
        return
    }

    # Ask nicely first.
    foreach ($p in $procs) {
        try { $p.CloseMainWindow() | Out-Null } catch { }
    }

    # Wait for graceful exit.
    $deadline = (Get-Date).AddSeconds($GraceSeconds)
    while ((Get-Date) -lt $deadline) {
        $still = Get-Process -Name $names -ErrorAction SilentlyContinue
        if (-not $still) {
            Write-PolyPhase -Phase "quit" -State "ok" -Detail "clean graceful exit"
            return
        }
        Start-Sleep -Seconds 1
    }

    # Hard-kill fallback. This is a degraded outcome — record it as such but do
    # not fail the quit phase (leaving the runner clean is the goal; the hung
    # process is captured in the status log for diagnosis).
    $remaining = Get-Process -Name $names -ErrorAction SilentlyContinue
    foreach ($p in $remaining) {
        try {
            Stop-Process -Id $p.Id -Force -ErrorAction Stop
            Write-PolyPhase -Phase "quit" -State "ok" `
                -Detail "hard-killed unresponsive Cubase (graceful quit timed out)" `
                -Extra @{ pid = $p.Id; graceSeconds = $GraceSeconds }
        } catch {
            Write-PolyPhase -Phase "quit" -State "ok" `
                -Detail "process $($p.Id) exited during hard-kill" -Extra @{ pid = $p.Id }
        }
    }
} catch {
    Invoke-PolyPhaseFailure -Phase "quit" -Message $_.Exception.Message
}
