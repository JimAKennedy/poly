# M042 S07: block until Cubase is ready, or fail loud on a bounded timeout.
#
# The readiness signal evolves across slices:
#   - S07 (now): "the launched Cubase process is alive AND has held a main
#     window across a minimum settle window" is sufficient — we are only
#     proving a clean launch/quit, no transport yet. The settle requirement
#     matters: Cubase's splash/loading window gets a non-zero MainWindowHandle
#     within a few hundred ms, so a single-poll "has a window" check passes on
#     the splash and would mask a Cubase that opens then hangs on a modal
#     dialog. Requiring the window to persist across StableSeconds rejects the
#     transient splash and only accepts a settled main window.
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
    [int] $PollSeconds = 2,
    # The main window must remain present for at least this long before we call
    # it ready — this is what rejects the transient splash window. The pid seen
    # with a window must be the same across the settle span.
    [int] $StableSeconds = 6,
    # Match the specific process launched by launch-cubase.ps1 rather than any
    # Cubase-named process. launch-cubase writes POLY_CUBASE_PID; if absent
    # (local dry-run), fall back to name matching.
    [int] $ExpectedPid = 0
)

. "$PSScriptRoot/_common.ps1"

if ($ExpectedPid -le 0 -and $env:POLY_CUBASE_PID) {
    $ExpectedPid = [int]$env:POLY_CUBASE_PID
}

Write-PolyPhase -Phase "wait-ready" -State "start" `
    -Extra @{ timeoutSeconds = $TimeoutSeconds; stableSeconds = $StableSeconds; expectedPid = $ExpectedPid }

try {
    $start = Get-Date
    $deadline = $start.AddSeconds($TimeoutSeconds)
    $names = Get-CubaseProcessNames

    # The candidate must be the SAME pid holding a window continuously across
    # the settle span. Track when a windowed pid was first seen; reset if it
    # changes or disappears.
    $settledPid = 0
    $settledSince = $null

    while ((Get-Date) -lt $deadline) {
        # Prefer the launched pid; otherwise any Cubase-named process. Require a
        # real main window (splash and main window both have non-zero handles,
        # but the settle requirement below filters the transient splash).
        $proc = $null
        if ($ExpectedPid -gt 0) {
            $proc = Get-Process -Id $ExpectedPid -ErrorAction SilentlyContinue |
                Where-Object { $_.MainWindowHandle -ne 0 } |
                Select-Object -First 1
        } else {
            $proc = Get-Process -Name $names -ErrorAction SilentlyContinue |
                Where-Object { $_.MainWindowHandle -ne 0 } |
                Select-Object -First 1
        }

        if ($proc) {
            if ($proc.Id -ne $settledPid) {
                # First sighting of this windowed pid — start the settle timer.
                $settledPid = $proc.Id
                $settledSince = Get-Date
            } elseif (((Get-Date) - $settledSince).TotalSeconds -ge $StableSeconds) {
                $elapsed = [int]((Get-Date) - $start).TotalSeconds
                Write-PolyPhase -Phase "wait-ready" -State "ok" `
                    -Detail "Cubase main window stable for ${StableSeconds}s" `
                    -Extra @{ pid = $proc.Id; elapsedSeconds = $elapsed; stableSeconds = $StableSeconds }
                return
            }
        } else {
            # Lost the window (splash closed before main window, or process
            # died) — reset the settle timer.
            $settledPid = 0
            $settledSince = $null
        }
        Start-Sleep -Seconds $PollSeconds
    }

    Invoke-PolyPhaseFailure -Phase "wait-ready" `
        -Message "Cubase did not hold a stable main window within ${TimeoutSeconds}s" `
        -Extra @{ timeoutSeconds = $TimeoutSeconds; stableSeconds = $StableSeconds; expectedPid = $ExpectedPid }
} catch {
    # Invoke-PolyPhaseFailure rethrows; re-wrap any other unexpected error the
    # same way so nothing is swallowed.
    if ($_.Exception.Message -notmatch "wait-ready failed") {
        Invoke-PolyPhaseFailure -Phase "wait-ready" -Message $_.Exception.Message
    }
    throw
}
