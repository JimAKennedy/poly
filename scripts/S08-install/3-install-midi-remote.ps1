# M042 S08 runner step 3: install the Cubase MIDI Remote transport script.
#
# Copies tests/cubase/midi-remote/poly-transport.js into Cubase's driver-scripts
# tree. The <vendor>/<device> path segments must match
# makeDeviceDriver('Jk Digital', 'Poly Test', ...) in the script, which
# Get-PolyMidiRemoteDir encodes. After running, open Cubase's MIDI Remote tab —
# with the loopMIDI 'poly-test' port present, the script auto-loads.

[CmdletBinding()]
param()

. "$PSScriptRoot/_shared.ps1"

$root = Get-PolyRepoRoot
$src = Join-Path $root "tests\cubase\midi-remote\poly-transport.js"
$dest = Get-PolyMidiRemoteDir

if (-not (Test-Path $src)) {
    Write-S08 -Level fail -Message "Source script not found at $src. Run 1-sync-main.ps1 first."
    exit 1
}

New-Item -ItemType Directory -Force -Path $dest | Out-Null
Copy-Item -Force $src (Join-Path $dest "poly-transport.js")
Write-S08 -Level ok -Message "Installed poly-transport.js -> $dest"
Write-S08 -Level info -Message "Next: open Cubase's MIDI Remote tab; with the loopMIDI 'poly-test' port present the script should appear in the surface list."
