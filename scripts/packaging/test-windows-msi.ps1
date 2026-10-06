# test-windows-msi.ps1 — install Poly's MSI on a clean Windows machine and
# prove uninstalling removes it (open-source-launch M008/S01, OS37).
#
#   pwsh scripts/packaging/test-windows-msi.ps1 -Msi <poly.msi> [-SelfSign]
#
# With -SelfSign (CI only) it first signs the MSI once with a throwaway
# self-signed certificate it trusts on this machine, and verifies the
# signature: the one-step signtool sign M008/S02 will do with Azure Artifact
# Signing. Then it installs silently, checks the bundle is in
# Common Files\VST3 and that Apps & features lists Poly, uninstalls silently,
# and checks both are gone and the VST3 folder is as it was.
#
# Installs into the real plug-in folder and needs administrator rights, so it
# runs only on a clean machine — a fresh CI runner, or anywhere Poly is not
# installed. Where it is, it refuses rather than remove a real copy.
param(
    [string]$Msi,
    [switch]$SelfSign
)
$ErrorActionPreference = 'Stop'

function Say($msg) { Write-Output "test-windows-msi: $msg" }
function Fail($msg) { [Console]::Error.WriteLine("test-windows-msi: FAIL: $msg"); exit 1 }

if ([string]::IsNullOrWhiteSpace($Msi) -or -not (Test-Path -LiteralPath $Msi -PathType Leaf)) {
    [Console]::Error.WriteLine("test-windows-msi: usage: -Msi <poly.msi> [-SelfSign] (no MSI at '$Msi')"); exit 2
}
$Msi = (Resolve-Path -LiteralPath $Msi).Path
$vst3 = Join-Path $env:CommonProgramFiles 'VST3'
$bundle = Join-Path $vst3 'poly_plugin.vst3'
if (Test-Path -LiteralPath $bundle) {
    [Console]::Error.WriteLine("test-windows-msi: refusing to run: $bundle already exists, so this is not a clean machine;")
    [Console]::Error.WriteLine('test-windows-msi: installing and uninstalling would replace and then remove that copy'); exit 2
}

function Find-Signtool {
    $kits = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10\bin'
    $found = Get-ChildItem -Path $kits -Recurse -Filter signtool.exe -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -match '\\x64\\' } | Sort-Object FullName -Descending | Select-Object -First 1
    if (-not $found) { Fail "no x64 signtool.exe under $kits" }
    $found.FullName
}

function Uninstall-Entry {
    Get-ItemProperty 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\*' -ErrorAction SilentlyContinue |
        Where-Object { $_.DisplayName -eq 'Poly' }
}

function Run-Msiexec([string[]]$arguments, [string]$what) {
    $p = Start-Process -FilePath msiexec.exe -ArgumentList $arguments -Wait -PassThru
    if ($p.ExitCode -ne 0) { Fail "$what exited $($p.ExitCode) (see the log)" }
}

if ($SelfSign) {
    $signtool = Find-Signtool
    $cert = New-SelfSignedCertificate -Type CodeSigningCert -Subject 'CN=Poly CI self-signed (M008-S01)' -CertStoreLocation Cert:\CurrentUser\My
    $cer = Join-Path ([IO.Path]::GetTempPath()) 'poly-ci-selfsigned.cer'
    Export-Certificate -Cert $cert -FilePath $cer | Out-Null
    Import-Certificate -FilePath $cer -CertStoreLocation Cert:\LocalMachine\Root | Out-Null
    Import-Certificate -FilePath $cer -CertStoreLocation Cert:\LocalMachine\TrustedPublisher | Out-Null
    & $signtool sign /fd SHA256 /sha1 $cert.Thumbprint $Msi | Out-Host
    if ($LASTEXITCODE -ne 0) { Fail 'signtool could not sign the MSI in one step' }
    & $signtool verify /pa $Msi | Out-Host
    if ($LASTEXITCODE -ne 0) { Fail 'signtool verify /pa rejected the signed MSI' }
    Say "signed in one signtool step and verified (/pa) with a throwaway certificate, $($cert.Thumbprint)"
}

$existedBefore = Test-Path -LiteralPath $vst3
$before = if ($existedBefore) { (Get-ChildItem -LiteralPath $vst3 -Force | Select-Object -ExpandProperty Name | Sort-Object) -join '|' } else { '' }
Say "before: VST3 folder $(if ($existedBefore) { "exists ($before)" } else { 'absent' })"

$logDir = Split-Path -Parent $Msi
Run-Msiexec @('/i', "`"$Msi`"", '/qn', '/l*v', "`"$(Join-Path $logDir 'install.log')`"") 'install'
$dll = Get-ChildItem -LiteralPath $bundle -Recurse -File -Filter '*.vst3' -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $dll) { Fail "no plugin binary under $bundle after installing" }
$entry = Uninstall-Entry
if (-not $entry) { Fail 'Apps & features does not list Poly after installing' }
if ($entry.Publisher -ne 'jk.digital') { Fail "Apps & features lists Poly with publisher '$($entry.Publisher)', not jk.digital" }
Say "installed: $($dll.FullName); Apps & features lists Poly $($entry.DisplayVersion) by $($entry.Publisher)"

Run-Msiexec @('/x', "`"$Msi`"", '/qn', '/l*v', "`"$(Join-Path $logDir 'uninstall.log')`"") 'uninstall'
if (Test-Path -LiteralPath $bundle) { Fail "$bundle is still there after uninstalling" }
if (Uninstall-Entry) { Fail 'Apps & features still lists Poly after uninstalling' }
$existsAfter = Test-Path -LiteralPath $vst3
$after = if ($existsAfter) { (Get-ChildItem -LiteralPath $vst3 -Force | Select-Object -ExpandProperty Name | Sort-Object) -join '|' } else { '' }
if ($existsAfter -ne $existedBefore -or $after -ne $before) {
    Fail "the VST3 folder differs after uninstalling (before: $(if ($existedBefore) { $before } else { 'absent' }); after: $(if ($existsAfter) { $after } else { 'absent' }))"
}
Say 'uninstalled: the bundle and its Apps & features entry are gone, and the VST3 folder is as it was'
Say 'PASS — installs and uninstalls cleanly'
