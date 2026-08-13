# M042 S09: wait for the Poly editor's WebView2 CDP endpoint to come up before
# the Playwright-over-CDP e2e attaches.
#
# Why this exists (the S09 attach failure, runner-confirmed):
#   choc's WebView2 DOES honor WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS=
#   --remote-debugging-port and opens a real CDP listener on 127.0.0.1:<port>
#   (owned by a Cubase child process). The runner diagnostic
#   (diagnose-editor-window.ps1) proved on JIMW1 that this port is up even with
#   Cubase in the BACKGROUND and the choc WebView2 host window Visible=False --
#   so CDP does NOT require the editor to be visible or foregrounded. An earlier
#   theory that focus was required was WRONG (see MEM115); this script no longer
#   forces foreground.
#
#   The real failure mode is that in an unattended launch the WebView2/choc host
#   may never INSTANTIATE at all (process_calls=0 -- the editor never
#   materialized once), so the port never opens. This gate polls the OS TCP
#   listen table for 127.0.0.1:<port> and fails loud (nonzero exit) if the port
#   never appears within the timeout, so a still-absent endpoint is diagnosed
#   HERE with a clear phase line rather than surfacing as Playwright's opaque
#   30s connect-retry timeout.
#
# Runs between Wait-for-ready and the L4-web e2e step, only when POLY_CDP_PORT
# is set (the S09 flow). It never runs for S07/S08.

[CmdletBinding()]
param(
    # The CDP remote-debugging port WebView2 was launched with (POLY_CDP_PORT).
    [Parameter(Mandatory)] [int] $CdpPort,
    # How long to wait for the CDP listener to appear.
    [int] $TimeoutSeconds = 60,
    [int] $PollSeconds = 2
)

. "$PSScriptRoot/_common.ps1"

Write-PolyPhase -Phase "focus-editor-cdp" -State "start" `
    -Extra @{ cdpPort = $CdpPort; timeoutSeconds = $TimeoutSeconds }

try {
    # True when something is LISTENING on 127.0.0.1:<port>. Reads the OS TCP
    # table rather than connecting -- focus-independent, and cheap to poll.
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

    if ($listening) {
        Write-PolyPhase -Phase "focus-editor-cdp" -State "ok" `
            -Detail "CDP listener up on 127.0.0.1:$CdpPort (Poly editor WebView2 materialized)" `
            -Extra @{ cdpPort = $CdpPort }
    } else {
        # The port never came up within the window. This is a hard failure for the
        # S09 flow: without the CDP endpoint the e2e cannot attach. Fail loud so
        # the quit phase still runs (workflow `if: always()`) and the cause is
        # captured here, not left to Playwright's opaque connect timeout. The
        # near-certain cause is that the WebView2/choc host never instantiated in
        # this unattended session (process_calls=0) -- NOT a focus problem.
        Invoke-PolyPhaseFailure -Phase "focus-editor-cdp" `
            -Message "CDP listener never appeared on 127.0.0.1:$CdpPort within ${TimeoutSeconds}s. Likely the Poly editor's WebView2/choc host never instantiated in this unattended session (process_calls=0), so no CDP endpoint was exposed." `
            -Extra @{ cdpPort = $CdpPort }
    }
} catch {
    Invoke-PolyPhaseFailure -Phase "focus-editor-cdp" -Message $_.Exception.Message
}
