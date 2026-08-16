# M042 S08: shared helpers for the S08 runner install/verify scripts.
#
# Dot-source at the top of each S08-install script:
#   . "$PSScriptRoot/_shared.ps1"
#
# These scripts automate the retype-heavy runner steps in the S08 walkthrough
# (docs/windows-test-runner-setup.md + tests/cubase/fixtures/README.md) so the
# owner can run one command per step in a VNC session instead of retyping.
# Nothing here authors the .cpr (only Cubase can) — these prepare everything
# around it and verify the result.

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# Repo root = two levels up from this script (scripts/S08-install/_shared.ps1).
function Get-PolyRepoRoot {
    return (Split-Path -Parent (Split-Path -Parent $PSScriptRoot))
}

# The VST3 install dir Cubase loads plugins from (matches the workflow's
# POLY_VST3_INSTALL_DIR). Overridable via env for a non-standard box.
#
# The default is the PER-USER VST3 location, not the machine-wide
# C:\Program Files\Common Files\VST3: Cubase scans both, but only this one is
# writable without admin, and the runner deliberately runs non-elevated so that
# WebView2 honors the CDP debugging flag (docs/windows-test-runner-setup.md
# Part 12).
function Get-PolyVst3Dir {
    if ($env:POLY_VST3_INSTALL_DIR) { return $env:POLY_VST3_INSTALL_DIR }
    return (Join-Path $env:LOCALAPPDATA "Programs\Common\VST3")
}

# The committed fixture path the nightly opens (matches the workflow's
# POLY_FIXTURE_CPR target and launch-cubase.ps1 -FixtureCpr).
function Get-PolyFixturePath {
    return (Join-Path (Get-PolyRepoRoot) "tests\cubase\fixtures\poly-4bar.cpr")
}

# The Cubase MIDI Remote install dir. Cubase auto-detects a driver script by a
# STRICT convention: Local\<vendor>\<device>\<vendor>_<device>.js, all derived
# from makeDeviceDriver('JkDigital', 'PolyTest', ...). A mismatched folder OR
# filename means the script is silently ignored and no surface connects.
function Get-PolyMidiRemoteDir {
    return (Join-Path $env:USERPROFILE `
        "Documents\Steinberg\Cubase\MIDI Remote\Driver Scripts\Local\JkDigital\PolyTest")
}

# The required script filename (must be <vendor>_<device>.js to be detected).
function Get-PolyMidiRemoteScriptName {
    return "JkDigital_PolyTest.js"
}

function Write-S08 {
    param(
        [Parameter(Mandatory)] [ValidateSet("info", "ok", "warn", "fail")] [string] $Level,
        [Parameter(Mandatory)] [string] $Message
    )
    $prefix = switch ($Level) {
        "ok"   { "[ OK ] " }
        "warn" { "[WARN] " }
        "fail" { "[FAIL] " }
        default { "[ .. ] " }
    }
    $color = switch ($Level) {
        "ok"   { "Green" }
        "warn" { "Yellow" }
        "fail" { "Red" }
        default { "Cyan" }
    }
    Write-Host "$prefix$Message" -ForegroundColor $color
}
