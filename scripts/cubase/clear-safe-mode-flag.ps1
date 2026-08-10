# M042 S08: clear Cubase's "unexpected termination" flag BEFORE launch so Safe
# Mode never appears — the root-cause fix for the recovery-dialog chain.
#
# Root cause (confirmed on the runner 2026-08-10): Cubase writes a zero-byte
# sentinel `ApplicationStarted.txt` into its prefs dir at startup and DELETES it
# on a clean exit. The next launch reads it: if the file is still present, Cubase
# concludes the previous session terminated unexpectedly and pops the modal
# "Safe Mode" recovery dialog BEFORE loading the project.
#
# On this runner every quit hard-kills Cubase (the Steinberg Hub blocks a clean
# exit on project-close — the documented expected path in quit-cubase.ps1). A
# hard-kill never deletes the sentinel, so it is ALWAYS present on the next
# launch, so Safe Mode fires on EVERY run. That modal blocks the project from
# loading, so the MIDI Remote surface never connects and the driver times out.
#
# Deleting the sentinel before launch makes Cubase see a clean prior shutdown and
# skip Safe Mode entirely. This is far more robust than dismissing the dialog
# with SendKeys (confirmed on the runner NOT to close it): there is no window to
# fight and no keystroke to land — we remove the trigger. dismiss-safe-mode.ps1
# and wait-for-ready.ps1's Safe-Mode rejection stay as backstops in case a future
# Cubase version uses a different sentinel.
#
# Runs between Kill stale Cubase and Launch. It is a no-op (success) when the
# sentinel is absent — a genuinely clean prior run leaves nothing to clear — so
# it is safe on every run. It never fails the run: a missing prefs dir or an
# already-clean flag is the happy path, not an error.

[CmdletBinding()]
param(
    [Parameter(Mandatory)] [string] $CubaseVersion
)

. "$PSScriptRoot/_common.ps1"

Write-PolyPhase -Phase "clear-safe-mode-flag" -State "start" `
    -Extra @{ cubaseVersion = $CubaseVersion }

try {
    # Prefs dir layout matches archive-logs.ps1: %APPDATA%\Steinberg\Cubase <ver>_64.
    $prefsDir = Join-Path $env:APPDATA "Steinberg\Cubase $CubaseVersion`_64"
    $flag = Join-Path $prefsDir "ApplicationStarted.txt"

    if (Test-Path $flag) {
        Remove-Item -Force $flag
        Write-PolyPhase -Phase "clear-safe-mode-flag" -State "ok" `
            -Detail "deleted stale ApplicationStarted.txt (Cubase will skip Safe Mode)" `
            -Extra @{ flag = $flag }
    } else {
        Write-PolyPhase -Phase "clear-safe-mode-flag" -State "ok" `
            -Detail "no ApplicationStarted.txt present (clean prior shutdown, nothing to clear)" `
            -Extra @{ flag = $flag }
    }
} catch {
    # Clearing the flag is best-effort. If it fails, dismiss-safe-mode.ps1 and
    # wait-for-ready's rejection are still in the chain — log loud and continue.
    Write-PolyPhase -Phase "clear-safe-mode-flag" -State "fail" `
        -Detail "error while clearing Safe Mode flag: $($_.Exception.Message)"
}
