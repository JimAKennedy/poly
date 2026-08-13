# M042 S09: bring the Poly editor to the foreground and wait for its WebView2
# CDP endpoint to come up, before the Playwright-over-CDP e2e attaches.
#
# Why this exists (the S09 attach failure, runner-confirmed):
#   choc's WebView2 DOES honor WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS=
#   --remote-debugging-port and opens a real CDP listener on 127.0.0.1:<port>
#   (owned by a Cubase child process) -- but ONLY while the Poly plugin editor
#   window is materialized AND Cubase has foreground focus. Cubase destroys the
#   WebView2 view on editor focus-loss and recreates it on focus-return, so the
#   CDP port comes and goes with the editor. On an unattended runner nothing
#   gives the editor sustained focus, so the port never stays up long enough for
#   connectOverCDP to attach (observed: process_calls=0, ECONNREFUSED 9222).
#
#   The fixture (#212) saves with the editor window open, so it exists on load;
#   this step forces Cubase foreground so the editor materializes and WebView2
#   binds the CDP port, then it POLLS the port until it is actually listening.
#   That poll is the real readiness signal for the e2e -- far stronger than a
#   window-title check -- and it fails loud (nonzero exit) if the port never
#   opens, so a still-absent endpoint is diagnosed HERE with a clear phase line
#   rather than surfacing as Playwright's opaque 30s connect-retry timeout.
#
# Runs between Wait-for-ready and the L4-web e2e step, only when POLY_CDP_PORT
# is set (the S09 flow). It never runs for S07/S08.

[CmdletBinding()]
param(
    # The CDP remote-debugging port WebView2 was launched with (POLY_CDP_PORT).
    [Parameter(Mandatory)] [int] $CdpPort,
    # How long to wait for the CDP listener to appear after foregrounding.
    [int] $TimeoutSeconds = 60,
    [int] $PollSeconds = 2,
    # After the port is confirmed up, keep re-asserting foreground for this many
    # seconds so the WebView2 view (and its CDP port) is NOT torn down while the
    # Playwright attach happens in a sibling process. 0 = verify-only (fail-loud
    # gate): confirm the port then exit. The e2e step runs a second invocation
    # of this script as a BACKGROUND job with -HoldSeconds so Cubase stays
    # foreground across `npx playwright test`.
    [int] $HoldSeconds = 0,
    # The launched Cubase pid (from launch-cubase.ps1). Falls back to
    # POLY_CUBASE_PID, then to any Cubase-named process.
    [int] $ExpectedPid = 0
)

. "$PSScriptRoot/_common.ps1"

if ($ExpectedPid -le 0 -and $env:POLY_CUBASE_PID) {
    $ExpectedPid = [int]$env:POLY_CUBASE_PID
}

Write-PolyPhase -Phase "focus-editor-cdp" -State "start" `
    -Extra @{ cdpPort = $CdpPort; timeoutSeconds = $TimeoutSeconds; expectedPid = $ExpectedPid }

try {
    Add-Type -AssemblyName System.Windows.Forms

    # SetForegroundWindow is the Win32 call that actually materializes/repaints a
    # window; AppActivate (WScript.Shell) is the established fallback used by
    # dismiss-safe-mode.ps1. We use both: SetForegroundWindow on the launched
    # pid's main window handle, then AppActivate as a belt-and-suspenders.
    Add-Type -Namespace Win32 -Name Fg -MemberDefinition @'
[System.Runtime.InteropServices.DllImport("user32.dll")]
public static extern bool SetForegroundWindow(System.IntPtr hWnd);
[System.Runtime.InteropServices.DllImport("user32.dll")]
public static extern bool ShowWindow(System.IntPtr hWnd, int nCmdShow);
'@

    $names = Get-CubaseProcessNames
    $wshell = New-Object -ComObject WScript.Shell

    # Resolve the Cubase process whose window we foreground.
    function Get-CubaseProc {
        if ($ExpectedPid -gt 0) {
            return Get-Process -Id $ExpectedPid -ErrorAction SilentlyContinue |
                Select-Object -First 1
        }
        return Get-Process -Name $names -ErrorAction SilentlyContinue |
            Where-Object { $_.MainWindowHandle -ne 0 } |
            Select-Object -First 1
    }

    # True when something is LISTENING on 127.0.0.1:<port>. Reads the OS TCP
    # table (focus-independent) rather than connecting, so the check itself does
    # not need to steal focus back from the editor.
    function Test-CdpListening {
        param([int] $Port)
        $rows = Get-NetTCPConnection -State Listen -ErrorAction SilentlyContinue |
            Where-Object { $_.LocalPort -eq $Port }
        return [bool]$rows
    }

    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    $listening = $false
    $foregrounded = $false

    while ((Get-Date) -lt $deadline) {
        # (Re-)assert foreground on every poll: the WebView2 view is torn down on
        # focus-loss, so we must keep Cubase foreground until the port is up.
        $proc = Get-CubaseProc
        if ($proc -and $proc.MainWindowHandle -ne 0) {
            # SW_RESTORE = 9 (un-minimize/repaint), then raise to foreground.
            [void][Win32.Fg]::ShowWindow($proc.MainWindowHandle, 9)
            [void][Win32.Fg]::SetForegroundWindow($proc.MainWindowHandle)
            [void]$wshell.AppActivate($proc.Id)
            $foregrounded = $true
        }

        if (Test-CdpListening -Port $CdpPort) {
            $listening = $true
            break
        }
        Start-Sleep -Seconds $PollSeconds
    }

    if ($listening) {
        Write-PolyPhase -Phase "focus-editor-cdp" -State "ok" `
            -Detail "CDP listener up on 127.0.0.1:$CdpPort (Poly editor materialized)" `
            -Extra @{ cdpPort = $CdpPort; foregrounded = $foregrounded; holdSeconds = $HoldSeconds }

        # Hold mode: keep re-asserting foreground so the WebView2 view (and its
        # CDP port) is not torn down while a sibling process attaches. Runs as a
        # background job from the e2e step. Loop quietly (no per-tick phase spam);
        # stop early if the port drops so we don't spin uselessly.
        if ($HoldSeconds -gt 0) {
            $holdUntil = (Get-Date).AddSeconds($HoldSeconds)
            while ((Get-Date) -lt $holdUntil) {
                $proc = Get-CubaseProc
                if ($proc -and $proc.MainWindowHandle -ne 0) {
                    [void][Win32.Fg]::ShowWindow($proc.MainWindowHandle, 9)
                    [void][Win32.Fg]::SetForegroundWindow($proc.MainWindowHandle)
                    [void]$wshell.AppActivate($proc.Id)
                }
                Start-Sleep -Milliseconds 500
            }
        }
    } else {
        # The port never came up within the window. This is a hard failure for the
        # S09 flow: without the CDP endpoint the e2e cannot attach. Fail loud so
        # the quit phase still runs (workflow `if: always()`) and the cause is
        # captured here, not left to Playwright's opaque connect timeout.
        Invoke-PolyPhaseFailure -Phase "focus-editor-cdp" `
            -Message "CDP listener never appeared on 127.0.0.1:$CdpPort within ${TimeoutSeconds}s despite foregrounding Cubase. Likely the Poly editor did not materialize, or WebView2 did not expose CDP for this session." `
            -Extra @{ cdpPort = $CdpPort; foregrounded = $foregrounded }
    }
} catch {
    Invoke-PolyPhaseFailure -Phase "focus-editor-cdp" -Message $_.Exception.Message
}
