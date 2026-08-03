# M042 S07: block until Cubase is ready, or fail loud on a bounded timeout.
#
# The readiness signal evolves across slices:
#   - S07 (now): "process is alive with a main window" is sufficient — we are
#     only proving a clean launch/quit, no transport yet.
#   - S08: the MIDI Remote script sends a 'ready' ping over loopMIDI; this
#     script will be extended to wait on that ping (a stronger signal that the
#     project loaded and the remote surface is live).
#
# The timeout is the load-bearing safety property: an unattended nightly must
# NEVER hang the single runner. On timeout we fail loud (persisted error +
# nonzero exit) so the quit phase still runs (workflow `if: always()`) and the
# next run isn't blocked.

[CmdletBinding()]
param(
    [int] $TimeoutSeconds = 120,
    # Poll interval; small enough to be responsive, large enough to be cheap.
    [int] $PollSeconds = 2
)

. "$PSScriptRoot/_common.ps1"

Write-PolyPhase -Phase "wait-ready" -State "start" `
    -Extra @{ timeoutSeconds = $TimeoutSeconds }

try {
    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    $names = Get-CubaseProcessNames

    while ((Get-Date) -lt $deadline) {
        $proc = Get-Process -Name $names -ErrorAction SilentlyContinue |
            Where-Object { $_.MainWindowHandle -ne 0 } |
            Select-Object -First 1

        if ($proc) {
            $elapsed = [int]((Get-Date) - $deadline.AddSeconds(-$TimeoutSeconds)).TotalSeconds
            Write-PolyPhase -Phase "wait-ready" -State "ok" `
                -Detail "Cubase main window present" `
                -Extra @{ pid = $proc.Id; elapsedSeconds = $elapsed }
            return
        }
        Start-Sleep -Seconds $PollSeconds
    }

    Invoke-PolyPhaseFailure -Phase "wait-ready" `
        -Message "Cubase did not present a main window within ${TimeoutSeconds}s" `
        -Extra @{ timeoutSeconds = $TimeoutSeconds }
} catch {
    # Invoke-PolyPhaseFailure rethrows; re-wrap any other unexpected error the
    # same way so nothing is swallowed.
    if ($_.Exception.Message -notmatch "wait-ready failed") {
        Invoke-PolyPhaseFailure -Phase "wait-ready" -Message $_.Exception.Message
    }
    throw
}
