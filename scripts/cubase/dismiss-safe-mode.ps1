# M042 S08: dismiss Cubase's "Safe Mode" recovery dialog if it appears on launch.
#
# Why this exists: the quit phase hard-kills Cubase (the Steinberg Hub blocks a
# clean exit on project-close, so quit-cubase.ps1 falls back to Stop-Process
# -Force — the documented expected path). A hard-kill makes Cubase record an
# "unexpected termination" flag, so the NEXT launch opens a modal Safe Mode
# dialog ("The application was terminated unexpectedly") BEFORE it loads the
# project. That modal:
#   - blocks the project from ever loading (so the MIDI Remote surface never
#     connects and the driver's ready poll times out — the flake we chased), and
#   - has a real window handle, so wait-for-ready.ps1 would mistake it for the
#     settled main window and report "ready" against a Cubase that is stuck.
#
# The dialog's defaults are exactly what an unattended run wants: "Use current
# program preferences" selected and "Deactivate all third party plug-ins"
# unchecked. So dismissing it is just pressing its focused OK button — we send
# Enter to the window. This is deterministic and matches what a human would do.
#
# Runs between Launch and Wait-for-ready. It is a no-op (success) when no Safe
# Mode dialog is present — a clean launch simply has nothing to dismiss — so it
# is safe to run on every launch. It never fails the run: a launch with no
# recovery prompt is the happy path, not an error.

[CmdletBinding()]
param(
    # How long to watch for the dialog before giving up. The dialog appears
    # within the first few seconds of launch if it appears at all; a short window
    # keeps a clean launch from stalling.
    [int] $TimeoutSeconds = 20,
    [int] $PollSeconds = 1,
    # The launched Cubase pid (from launch-cubase.ps1). Falls back to
    # POLY_CUBASE_PID, then to any Cubase-named process.
    [int] $ExpectedPid = 0
)

. "$PSScriptRoot/_common.ps1"

if ($ExpectedPid -le 0 -and $env:POLY_CUBASE_PID) {
    $ExpectedPid = [int]$env:POLY_CUBASE_PID
}

# The dialog's window title. Matched case-insensitively as a substring so a
# minor version-to-version wording change ("Safe Mode" caption) still matches.
$SafeModeTitle = "Safe Mode"

Write-PolyPhase -Phase "dismiss-safe-mode" -State "start" `
    -Extra @{ timeoutSeconds = $TimeoutSeconds; expectedPid = $ExpectedPid }

try {
    Add-Type -AssemblyName System.Windows.Forms

    $names = Get-CubaseProcessNames
    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    $dismissed = $false

    while ((Get-Date) -lt $deadline) {
        # Find the Cubase process whose MAIN WINDOW is the Safe Mode dialog.
        # MainWindowTitle reflects the top-level window Cubase currently owns; on
        # the recovery path that is the Safe Mode modal.
        $proc = $null
        if ($ExpectedPid -gt 0) {
            $proc = Get-Process -Id $ExpectedPid -ErrorAction SilentlyContinue |
                Where-Object { $_.MainWindowTitle -like "*$SafeModeTitle*" } |
                Select-Object -First 1
        } else {
            $proc = Get-Process -Name $names -ErrorAction SilentlyContinue |
                Where-Object { $_.MainWindowTitle -like "*$SafeModeTitle*" } |
                Select-Object -First 1
        }

        if ($proc) {
            # Bring the dialog to the foreground, then press Enter to activate the
            # focused OK button (defaults are the safe choice — see header).
            $wshell = New-Object -ComObject WScript.Shell
            if ($wshell.AppActivate($proc.Id)) {
                Start-Sleep -Milliseconds 300
                [System.Windows.Forms.SendKeys]::SendWait("{ENTER}")
                Write-PolyPhase -Phase "dismiss-safe-mode" -State "ok" `
                    -Detail "dismissed Safe Mode dialog (sent Enter -> OK, kept current preferences)" `
                    -Extra @{ pid = $proc.Id; windowTitle = $proc.MainWindowTitle }
                $dismissed = $true
                break
            }
        }
        Start-Sleep -Seconds $PollSeconds
    }

    if (-not $dismissed) {
        # No dialog within the window — the happy path (clean launch) OR the
        # dialog closed on its own. Either way there is nothing to dismiss; this
        # is not a failure.
        Write-PolyPhase -Phase "dismiss-safe-mode" -State "ok" `
            -Detail "no Safe Mode dialog seen within ${TimeoutSeconds}s (clean launch)"
    }
} catch {
    # A failure to drive the dialog must NOT fail the run here — wait-for-ready
    # is the load-bearing gate. Log loud and continue; if the dialog really was
    # blocking, wait-for-ready's Safe-Mode rejection will time out and fail loud
    # with full context.
    Write-PolyPhase -Phase "dismiss-safe-mode" -State "fail" `
        -Detail "error while dismissing Safe Mode dialog: $($_.Exception.Message)"
}
