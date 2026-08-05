# M042 S08 runner one-shot: run install steps 1-4, then preflight.
#
# Convenience wrapper so you run ONE command in the VNC session instead of four.
# Assumes the loopMIDI 'poly-test' port already exists and Poly + poly_midi_probe
# are already BUILT under build/ (this installs them but does not build). If a
# step fails, fix it and re-run this or the individual numbered script.

[CmdletBinding()]
param()

. "$PSScriptRoot/_shared.ps1"

$steps = @(
    "1-sync-main.ps1",
    "2-install-plugins.ps1",
    "3-install-midi-remote.ps1",
    "4-install-driver-deps.ps1",
    "5-preflight.ps1"
)

foreach ($step in $steps) {
    Write-Host ""
    Write-S08 -Level info -Message "=== $step ==="
    & (Join-Path $PSScriptRoot $step)
    if ($LASTEXITCODE -ne 0 -and $step -ne "4-install-driver-deps.ps1") {
        # step 4 exits non-zero only on a missing loopMIDI port (a warn), which
        # preflight re-reports; every other non-zero is a real stop.
        Write-S08 -Level fail -Message "$step failed (exit $LASTEXITCODE). Stopping."
        exit 1
    }
}

Write-Host ""
Write-S08 -Level ok -Message "Install steps done. Review preflight above; if green, author the .cpr in Cubase next."
