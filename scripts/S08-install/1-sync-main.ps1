# M042 S08 runner step 1: sync the working clone to the latest main.
#
# The runner-side clone must carry the port-match fix (#182) before you install
# the MIDI Remote script by hand, so poly-transport.js has
# expectInputNameContains (not ...Equals). Run this from anywhere inside the
# working clone.

[CmdletBinding()]
param()

. "$PSScriptRoot/_shared.ps1"

$root = Get-PolyRepoRoot
Write-S08 -Level info -Message "Syncing $root to origin/main ..."

Push-Location $root
try {
    git fetch origin
    if ($LASTEXITCODE -ne 0) { throw "git fetch failed" }
    git checkout main
    if ($LASTEXITCODE -ne 0) { throw "git checkout main failed" }
    git pull --ff-only origin main
    if ($LASTEXITCODE -ne 0) { throw "git pull failed" }

    # Confirm the port-match fix is present — the substring matcher is the
    # load-bearing line for the loopMIDI 'poly-test 1' suffix.
    $script = Join-Path $root "tests\cubase\midi-remote\poly-transport.js"
    if (Select-String -Path $script -Pattern "expectInputNameContains" -Quiet) {
        Write-S08 -Level ok -Message "poly-transport.js has the substring matcher (port-match fix present)."
    } else {
        Write-S08 -Level fail -Message "poly-transport.js is MISSING expectInputNameContains — main is stale or the fix did not land. Do NOT proceed."
        exit 1
    }
    Write-S08 -Level ok -Message "Clone is on main at $(git rev-parse --short HEAD)."
} finally {
    Pop-Location
}
