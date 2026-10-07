# M004: one in-DAW spec in a Cubase session of its own -- start, run, quit.
#
# The nightly's M004 steps are each one call to this, so a step reads as what
# it tests rather than as seven lines of session plumbing, and a developer at
# the runner runs exactly what the nightly runs:
#
#   ./scripts/cubase/run-session-spec.ps1 -Name s04 -Spec editor-lifecycle.spec.ts `
#       -FixtureCpr tests/cubase/fixtures/poly-4bar.cpr -CopyFixture
#
# Cubase is quit whatever happens -- a failed start or a red spec must not leave
# a Cubase up for the next session to collide with -- and the exit code is the
# spec's, or the start's if the session never came up.

[CmdletBinding()]
param(
    [Parameter(Mandatory)] [string] $Name,
    [Parameter(Mandatory)] [string] $Spec,
    [Parameter(Mandatory)] [string] $FixtureCpr,
    [switch] $CopyFixture,
    # Exported as POLY_SESSION_MODE for specs with more than one pass
    # (session-recall.spec.ts: save | reopen).
    [string] $Mode = "",
    [string] $CubaseVersion = "14",
    [int] $CdpPort = 9222
)

. "$PSScriptRoot/_common.ps1"

$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$code = 1
try {
    $global:LASTEXITCODE = 0
    & (Join-Path $PSScriptRoot "start-session.ps1") -Name $Name -FixtureCpr $FixtureCpr `
        -CopyFixture:$CopyFixture -CubaseVersion $CubaseVersion -CdpPort $CdpPort
    if ($LASTEXITCODE -ne 0) {
        $code = $LASTEXITCODE
        Write-PolyPhase -Phase "run-session-spec" -State "fail" `
            -Detail "session '$Name' did not start" -Extra @{ exitCode = $code }
    } else {
        $env:POLY_SESSION_MODE = $Mode
        Push-Location (Join-Path $repoRoot "tests/cubase/e2e")
        try {
            npx playwright test $Spec
            $code = $LASTEXITCODE
        } finally {
            Pop-Location
        }
        Write-PolyPhase -Phase "run-session-spec" -State $(if ($code -eq 0) { "ok" } else { "fail" }) `
            -Detail "$Spec in session '$Name'" -Extra @{ exitCode = $code; mode = $Mode }
    }
} catch {
    # The scripts under start-session.ps1 fail by throwing (Invoke-PolyPhaseFailure).
    Write-Host "run-session-spec: session '$Name' failed: $($_.Exception.Message)"
    $code = 1
} finally {
    $global:LASTEXITCODE = 0
    & (Join-Path $PSScriptRoot "quit-cubase.ps1")
}
exit $code
