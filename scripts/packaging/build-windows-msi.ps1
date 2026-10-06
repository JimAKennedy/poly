# build-windows-msi.ps1 — build Poly's Windows installer
# (open-source-launch M008/S01, OS37).
#
#   pwsh scripts/packaging/build-windows-msi.ps1 -Bundle <poly_plugin.vst3> -Version <v> -Out <out.msi>
#
# Installs WiX 5.0.2 (pinned; v6 adds a maintenance fee) into a temporary tool
# directory if `wix` is not already on PATH, then builds poly.wxs for x64. An
# MSI's ProductVersion must be numeric, so a pre-release version such as
# 0.2.0-rc.1 builds as 0.2.0; the file name keeps the full tag. Windows only:
# WiX builds MSIs nowhere else. Unsigned until M008/S02.
param(
    [string]$Bundle,
    [string]$Version,
    [string]$Out
)
$ErrorActionPreference = 'Stop'

function Fail($msg) { [Console]::Error.WriteLine("build-windows-msi: $msg"); exit 2 }

if ([string]::IsNullOrWhiteSpace($Bundle) -or -not (Test-Path -LiteralPath $Bundle -PathType Container)) {
    Fail "no VST3 bundle at '$Bundle' (usage: -Bundle <poly_plugin.vst3> -Version <v> -Out <out.msi>)"
}
if ([string]::IsNullOrWhiteSpace($Version)) { Fail 'no version given (usage: -Bundle <poly_plugin.vst3> -Version <v> -Out <out.msi>)' }
if ([string]::IsNullOrWhiteSpace($Out)) { Fail 'no output path given (usage: -Bundle <poly_plugin.vst3> -Version <v> -Out <out.msi>)' }

$msiVersion = ($Version -split '-', 2)[0]
if ($msiVersion -notmatch '^\d+\.\d+\.\d+$') { Fail "version '$Version' has no numeric major.minor.patch for the MSI" }

$wix = (Get-Command wix -ErrorAction SilentlyContinue).Source
if (-not $wix) {
    $tools = Join-Path ([IO.Path]::GetTempPath()) 'poly-wix-5.0.2'
    if (-not (Test-Path (Join-Path $tools 'wix.exe'))) {
        dotnet tool install wix --version 5.0.2 --tool-path $tools | Out-Host
        if ($LASTEXITCODE -ne 0) { Fail 'could not install WiX 5.0.2 as a .NET tool' }
    }
    $wix = Join-Path $tools 'wix.exe'
}

$wxs = Join-Path $PSScriptRoot 'poly.wxs'
$bundleFull = (Resolve-Path -LiteralPath $Bundle).Path
& $wix build $wxs -arch x64 -d "Version=$msiVersion" -d "BundleDir=$bundleFull" -o $Out
if ($LASTEXITCODE -ne 0) { Fail "wix build failed with exit $LASTEXITCODE" }
Write-Output "built $Out (Poly $msiVersion, per-machine, installs to Common Files\VST3\poly_plugin.vst3)"
