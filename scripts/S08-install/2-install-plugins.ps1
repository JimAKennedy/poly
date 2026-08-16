# M042 S08 runner step 2: install poly_plugin.vst3 + poly_midi_probe.vst3 so
# Cubase can load both when you author the fixture.
#
# The nightly installs both bundles too; this is the by-hand equivalent for
# fixture authoring. Both target the PER-USER VST3 dir
# (%LOCALAPPDATA%\Programs\Common\VST3) — see Get-PolyVst3Dir — because the
# runner runs non-elevated and cannot write the machine-wide folder.
#
# Build first if you have not:
#   cmake -S . -B build -G "Visual Studio 17 2022" -DSMTG_RUN_VST_VALIDATOR=ON
#   cmake --build build --config Release --parallel

[CmdletBinding()]
param(
    # Install anyway when a shadow copy exists in the other VST3 folder. Off by
    # default: a shadow means Cubase may load the OTHER bundle and the run
    # silently tests a stale build.
    [switch] $AllowShadow
)

. "$PSScriptRoot/_shared.ps1"

$root = Get-PolyRepoRoot
$dest = Get-PolyVst3Dir
$buildDir = Join-Path $root "build"

# Refuse to install alongside a shadow copy. Cubase scans BOTH the per-user and
# machine-wide VST3 folders, and two bundles sharing a VST3 class ID resolve to
# whichever was scanned first — so installing a fresh bundle here while a stale
# one sits in the other folder produces a green run against the wrong binary.
# This is the ISSUE-001 trap; it is cheap to detect and expensive to diagnose.
$shadowDir = Get-PolyVst3ShadowDir
$shadows = @("poly_plugin.vst3", "poly_midi_probe.vst3") |
    ForEach-Object { Join-Path $shadowDir $_ } |
    Where-Object { Test-Path $_ }
if ($shadows -and -not $AllowShadow) {
    Write-S08 -Level fail -Message "Shadow bundle(s) found in $shadowDir — Cubase may load these instead of what this script installs:"
    foreach ($s in $shadows) { Write-S08 -Level fail -Message "  $s" }
    Write-S08 -Level info -Message "Delete them (elevated shell if under Program Files), then re-run:"
    foreach ($s in $shadows) { Write-S08 -Level info -Message "  Remove-Item -Recurse -Force '$s'" }
    Write-S08 -Level info -Message "Override with -AllowShadow only if you know why both copies must exist."
    exit 1
}

if (-not (Test-Path $buildDir)) {
    Write-S08 -Level fail -Message "No build/ dir at $buildDir. Configure + build first (see this script's header)."
    exit 1
}

New-Item -ItemType Directory -Force -Path $dest | Out-Null

$bundles = @("poly_plugin.vst3", "poly_midi_probe.vst3")
$installed = 0
foreach ($bundle in $bundles) {
    $src = Get-ChildItem -Path $buildDir -Recurse -Filter $bundle -Directory -ErrorAction SilentlyContinue |
        Select-Object -First 1
    if (-not $src) {
        Write-S08 -Level fail -Message "$bundle not found under build/. Did the Release build succeed?"
        exit 1
    }
    $target = Join-Path $dest $bundle
    if (Test-Path $target) { Remove-Item -Recurse -Force $target }
    Copy-Item -Recurse -Force $src.FullName $target
    Write-S08 -Level ok -Message "Installed $($src.FullName) -> $target"
    $installed++
}

Write-S08 -Level ok -Message "$installed/2 VST3 bundles installed to $dest. Restart Cubase so it rescans."
