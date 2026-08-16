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
#   The automated Actions-agent launch used to never bring the port up (M042
#   S09, runs #40-#47): the editor + choc WebView2 host DO materialize (topology
#   diagnostic: editor frame PRESENT, choc host PRESENT), yet the CDP port stayed
#   ABSENT. Root cause: the runner's logon task ran ELEVATED, and WebView2
#   ignores browser flags delivered via the environment or the registry whenever
#   the host app is elevated. Fixed by registering the task `-RunLevel Limited`
#   (docs/windows-runner-rehome-and-deelevate.md Part C3), which is also why
#   launch-cubase.ps1 now refuses to launch elevated with -EnableCdp.
#
#   This script is the gate: it polls the OS TCP listen table for
#   127.0.0.1:<port> and fails loud (nonzero exit) if the port never appears
#   within the timeout, so a still-absent endpoint is diagnosed HERE with a clear
#   phase line rather than surfacing as Playwright's opaque 30s connect-retry
#   timeout. Read editor-window-topology.txt (the diagnostic step just before
#   this) to see which layer is present — and whether the process is elevated
#   again — when the port is absent.
#
# Runs between Wait-for-ready and the L4-web e2e step, only when POLY_CDP_PORT is
# set. It never runs for a plain S07/S08 smoke.

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
        # topology diagnostic (run just before this) reports which layer is
        # missing. If it shows the editor frame + choc host PRESENT but no
        # listener, the --remote-debugging-port arg is not reaching the browser
        # process -- and the first thing to check is that line's "this process
        # elevated" value, because an elevated host makes WebView2 discard the
        # flag by design (the root cause of runs #40-#47).
        Invoke-PolyPhaseFailure -Phase "focus-editor-cdp" `
            -Message "CDP listener never appeared on 127.0.0.1:$CdpPort within ${TimeoutSeconds}s. Read editor-window-topology.txt: if the editor frame + choc host are PRESENT, the --remote-debugging-port arg is not reaching the browser process -- check whether that report says the process is elevated, which makes WebView2 discard the flag by design (re-register the runner logon task with -RunLevel Limited)." `
            -Extra @{ cdpPort = $CdpPort }
    }
} catch {
    Invoke-PolyPhaseFailure -Phase "focus-editor-cdp" -Message $_.Exception.Message
}
