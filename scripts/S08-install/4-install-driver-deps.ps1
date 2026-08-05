# M042 S08 runner step 4: install the mido driver's Python dependencies.
#
# Installs mido + python-rtmidi (pinned in requirements.txt) so
# tests/cubase/driver/play_scenario.py can open the loopMIDI port. The nightly
# does this itself; this is for manual driver dry-runs on the runner.

[CmdletBinding()]
param()

. "$PSScriptRoot/_shared.ps1"

$root = Get-PolyRepoRoot
$req = Join-Path $root "tests\cubase\driver\requirements.txt"

if (-not (Test-Path $req)) {
    Write-S08 -Level fail -Message "requirements.txt not found at $req. Run 1-sync-main.ps1 first."
    exit 1
}

Write-S08 -Level info -Message "Installing driver deps from $req ..."
python -m pip install -r $req
if ($LASTEXITCODE -ne 0) {
    Write-S08 -Level fail -Message "pip install failed. Is Python on PATH?"
    exit 1
}

# Confirm mido can enumerate ports and that a 'poly-test' port is visible.
$portCheck = @'
import mido
ins = mido.get_input_names()
outs = mido.get_output_names()
match = [p for p in ins + outs if "poly-test" in p.lower()]
print("inputs:", ins)
print("outputs:", outs)
print("poly-test match:", match)
import sys
sys.exit(0 if match else 3)
'@
$portCheck | python -
switch ($LASTEXITCODE) {
    0 { Write-S08 -Level ok -Message "mido installed and a 'poly-test' port is visible." }
    3 { Write-S08 -Level warn -Message "mido installed, but NO 'poly-test' port is visible. Create the loopMIDI port before dispatching." }
    default { Write-S08 -Level warn -Message "mido port check exited $LASTEXITCODE (mido may be installed but the check errored)." }
}
