# M042 S08 runner step 3: install the Cubase MIDI Remote transport script.
#
# Copies tests/cubase/midi-remote/JkDigital_PolyTest.js into Cubase's
# driver-scripts tree. Cubase auto-detects a driver script ONLY when both the
# folder (Local\<vendor>\<device>) AND the filename (<vendor>_<device>.js) are
# derived from makeDeviceDriver('JkDigital', 'PolyTest', ...). A mismatched name
# is silently ignored — no surface connects. Get-PolyMidiRemoteDir and
# Get-PolyMidiRemoteScriptName encode the required path/name. After running, open
# Cubase's MIDI Remote tab — with the loopMIDI 'poly-test' port present, the
# script auto-detects and connects.

[CmdletBinding()]
param()

. "$PSScriptRoot/_shared.ps1"

$root = Get-PolyRepoRoot
$scriptName = Get-PolyMidiRemoteScriptName
$src = Join-Path $root "tests\cubase\midi-remote\$scriptName"
$dest = Get-PolyMidiRemoteDir

if (-not (Test-Path $src)) {
    Write-S08 -Level fail -Message "Source script not found at $src. Run 1-sync-main.ps1 first."
    exit 1
}

# Remove the old, wrongly-named install (poly-transport.js under the old
# 'Jk Digital\Poly Test' folder) if a previous setup left it behind — a stale
# mis-named script in the tree does not connect and only adds confusion.
$driverRoot = Join-Path $env:USERPROFILE "Documents\Steinberg\Cubase\MIDI Remote\Driver Scripts\Local"
$oldDir = Join-Path $driverRoot "Jk Digital\Poly Test"
if (Test-Path $oldDir) {
    Remove-Item -Recurse -Force $oldDir
    Write-S08 -Level warn -Message "Removed stale mis-named install at $oldDir"
}

New-Item -ItemType Directory -Force -Path $dest | Out-Null
Copy-Item -Force $src (Join-Path $dest $scriptName)
Write-S08 -Level ok -Message "Installed $scriptName -> $dest"
Write-S08 -Level info -Message "Next: open Cubase's MIDI Remote tab; with the loopMIDI 'poly-test' port present the script auto-detects and connects (no manual import needed — Import Script only reads .midiremote files, not .js)."
