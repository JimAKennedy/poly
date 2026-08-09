# M042 S08 runner preflight: verify every prerequisite is in place before you
# author the fixture .cpr and dispatch the nightly. Read-only — changes nothing.
#
# Run this after steps 1-4 (and after creating the loopMIDI 'poly-test' port).
# Green across the board means you are ready to author the .cpr and dispatch.

[CmdletBinding()]
param()

. "$PSScriptRoot/_shared.ps1"

$root = Get-PolyRepoRoot
$fail = 0
$warn = 0

Write-S08 -Level info -Message "S08 preflight — checking runner prerequisites ..."

# 1. On main with the port-match fix.
Push-Location $root
try {
    $branch = (git rev-parse --abbrev-ref HEAD)
    if ($branch -eq "main") {
        Write-S08 -Level ok -Message "On branch main ($(git rev-parse --short HEAD))."
    } else {
        Write-S08 -Level warn -Message "On branch '$branch', not main. The nightly checks out main regardless, but a manual driver dry-run uses this tree."
        $warn++
    }
    $script = Join-Path $root "tests\cubase\midi-remote\JkDigital_PolyTest.js"
    if (Select-String -Path $script -Pattern "expectInputNameContains" -Quiet) {
        Write-S08 -Level ok -Message "JkDigital_PolyTest.js has the substring port matcher."
    } else {
        Write-S08 -Level fail -Message "JkDigital_PolyTest.js is MISSING expectInputNameContains. Run 1-sync-main.ps1."
        $fail++
    }
} finally {
    Pop-Location
}

# 2. Both VST3 bundles installed where Cubase loads them.
$vst3 = Get-PolyVst3Dir
foreach ($bundle in @("poly_plugin.vst3", "poly_midi_probe.vst3")) {
    if (Test-Path (Join-Path $vst3 $bundle)) {
        Write-S08 -Level ok -Message "$bundle installed in $vst3."
    } else {
        Write-S08 -Level fail -Message "$bundle NOT installed in $vst3. Run 2-install-plugins.ps1."
        $fail++
    }
}

# 3. MIDI Remote script installed at the expected path.
$mrScript = Join-Path (Get-PolyMidiRemoteDir) (Get-PolyMidiRemoteScriptName)
if (Test-Path $mrScript) {
    Write-S08 -Level ok -Message "MIDI Remote script installed at $mrScript."
} else {
    Write-S08 -Level fail -Message "MIDI Remote script NOT installed. Run 3-install-midi-remote.ps1."
    $fail++
}

# 4. loopMIDI 'poly-test' port visible to mido.
$portCheck = @'
import mido
match = [p for p in mido.get_input_names() + mido.get_output_names() if "poly-test" in p.lower()]
import sys
sys.exit(0 if match else 3)
'@
try {
    $portCheck | python - 2>$null
    switch ($LASTEXITCODE) {
        0 { Write-S08 -Level ok -Message "loopMIDI 'poly-test' port is visible to mido." }
        3 { Write-S08 -Level fail -Message "No 'poly-test' port visible. Create it in loopMIDI."; $fail++ }
        default { Write-S08 -Level warn -Message "mido not usable (exit $LASTEXITCODE). Run 4-install-driver-deps.ps1."; $warn++ }
    }
} catch {
    Write-S08 -Level warn -Message "Could not run the mido port check (Python/mido missing). Run 4-install-driver-deps.ps1."
    $warn++
}

# 5. Cubase 14 executable resolvable.
$cubase = @(
    "C:\Program Files\Steinberg\Cubase 14\Cubase14.exe",
    "C:\Program Files\Steinberg\Cubase 14\Cubase.exe"
) | Where-Object { Test-Path $_ } | Select-Object -First 1
if ($cubase) {
    Write-S08 -Level ok -Message "Cubase 14 found at $cubase."
} else {
    Write-S08 -Level fail -Message "Cubase 14 executable not found under C:\Program Files\Steinberg."
    $fail++
}

# 6. Fixture presence (informational — you author it AFTER preflight).
$fixture = Get-PolyFixturePath
if (Test-Path $fixture) {
    Write-S08 -Level ok -Message "Fixture already present: $fixture."
} else {
    Write-S08 -Level info -Message "Fixture not yet authored ($fixture) — expected before the FIRST run; author it in Cubase next."
}

# 7. Golden the compare diffs against.
$golden = Join-Path $root "tests\golden\processor_default_4bars.txt"
if (Test-Path $golden) {
    Write-S08 -Level ok -Message "Golden present: $golden."
} else {
    Write-S08 -Level fail -Message "Golden NOT present at $golden — the compare step has nothing to diff against."
    $fail++
}

Write-Host ""
if ($fail -gt 0) {
    Write-S08 -Level fail -Message "$fail blocking issue(s), $warn warning(s). Fix the FAILs before authoring the fixture."
    exit 1
} elseif ($warn -gt 0) {
    Write-S08 -Level warn -Message "0 blocking issues, $warn warning(s). Review the warnings, then author the .cpr and dispatch."
    exit 0
} else {
    Write-S08 -Level ok -Message "All preflight checks passed. Author the .cpr in Cubase, commit it, then dispatch the nightly."
    exit 0
}
