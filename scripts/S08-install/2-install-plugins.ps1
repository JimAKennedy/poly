# M042 S08 runner step 2: install poly_plugin.vst3 + poly_midi_probe.vst3 so
# Cubase can load both when you author the fixture.
#
# The nightly's "Install plugin for Cubase" step installs poly_plugin only;
# authoring the fixture ALSO needs poly_midi_probe (the downstream analyzer).
# This installs both from the build tree. Build first if you have not:
#   cmake -S . -B build -G "Visual Studio 17 2022" -DSMTG_RUN_VST_VALIDATOR=ON
#   cmake --build build --config Release --parallel

[CmdletBinding()]
param()

. "$PSScriptRoot/_shared.ps1"

$root = Get-PolyRepoRoot
$dest = Get-PolyVst3Dir
$buildDir = Join-Path $root "build"

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
