# M042 S07: launch Cubase with the fixture project (or empty, in S07).
#
# Cubase has no CLI transport/automation API (testing-strategy.md §3.1) — the
# only supported invocation is file-association open: `Cubase <path>.cpr`. This
# script resolves the Cubase.exe for the target version, exports POLY_PROBE_OUTPUT
# (so poly_midi_probe, S06, flushes JSONL on deactivate), and starts the process
# detached. It does NOT wait for readiness — that is wait-for-ready.ps1 — so the
# launch and readiness concerns stay separable and independently diagnosable.

[CmdletBinding()]
param(
    [Parameter(Mandatory)] [string] $CubaseVersion,
    # S07: empty string => launch Cubase with no project. S08 passes the
    # committed fixture .cpr here.
    [string] $FixtureCpr = "",
    [string] $ProbeOutput = "",
    # S09 (L4-web): when set, Cubase's embedded WebView2 (choc, the Poly editor
    # host on Windows) is launched with a CDP remote-debugging port so the
    # Playwright-over-CDP e2e can attach. Off by default so the S07/S08 flows
    # are unaffected. See docs/windows-test-runner-setup.md Part 12.
    [switch] $EnableCdp,
    [int] $CdpPort = 9222
)

. "$PSScriptRoot/_common.ps1"

Write-PolyPhase -Phase "launch" -State "start" `
    -Detail "Cubase $CubaseVersion" -Extra @{ fixture = $FixtureCpr }

try {
    $exe = Get-CubaseExePath -CubaseVersion $CubaseVersion
    if (-not $exe) {
        Invoke-PolyPhaseFailure -Phase "launch" `
            -Message "Cubase $CubaseVersion executable not found under C:\Program Files\Steinberg" `
            -Extra @{ cubaseVersion = $CubaseVersion }
    }

    # Export the probe output path for the child process to inherit.
    if ($ProbeOutput) {
        $probeDir = Split-Path -Parent $ProbeOutput
        if ($probeDir) { New-Item -ItemType Directory -Force -Path $probeDir | Out-Null }
        $env:POLY_PROBE_OUTPUT = $ProbeOutput
        Write-PolyPhase -Phase "launch" -State "ok" `
            -Detail "POLY_PROBE_OUTPUT set" -Extra @{ probeOutput = $ProbeOutput }
    }

    # S09: expose WebView2's CDP port so the Playwright e2e can attach to the
    # editor running inside Cubase. WebView2 reads
    # WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS from its process environment at
    # startup, so this must be set BEFORE Start-Process (the child inherits it).
    # The port binds to localhost only; the box's own network posture (Part 10:
    # LAN-gated, no inbound) is what keeps it off the wider network. WKWebView
    # (macOS) exposes no CDP, which is why this flow is Windows-only.
    #
    # NOTE: this automated path never brought the CDP port up under the GitHub
    # Actions logon-task agent (M042 S09, runs #40-#47: 0 of 18 msedgewebview2.exe
    # children ever carried the flag, no delivery mechanism — env var, HKCU
    # registry policy, ICoreWebView2EnvironmentOptions patch, or dedicated
    # user-data-dir — reached the browser process). The working path is the
    # MANUAL-CDP flow: the owner launches Cubase by hand in their VNC session with
    # this env var set (the #215 recipe), and the workflow's "Await manual Cubase"
    # gate waits for the port. This script is SKIPPED in that flow
    # (POLY_MANUAL_CUBASE == 'true'); it stays here for the S07/S08 launch/quit +
    # transport smokes, which do not need CDP.
    if ($EnableCdp) {
        $env:WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS = "--remote-debugging-port=$CdpPort"
        Write-PolyPhase -Phase "launch" -State "ok" `
            -Detail "WebView2 CDP enabled (env var)" -Extra @{ cdpPort = $CdpPort }
    }

    if ($FixtureCpr) {
        if (-not (Test-Path $FixtureCpr)) {
            Invoke-PolyPhaseFailure -Phase "launch" `
                -Message "fixture .cpr not found" -Extra @{ fixture = $FixtureCpr }
        }
        $proc = Start-Process -FilePath $exe -ArgumentList "`"$FixtureCpr`"" -PassThru
    } else {
        # No project: Cubase opens to its default state (Hub disabled per
        # runbook Part 5, so this lands on an empty project).
        $proc = Start-Process -FilePath $exe -PassThru
    }

    # Publish the launched pid so wait-for-ready.ps1 can match THIS process
    # rather than any Cubase-named process. Each workflow step runs a fresh
    # pwsh, so a process-scoped $env var would not survive — persist it:
    #   - GITHUB_ENV (workflow): the next step inherits POLY_CUBASE_PID.
    #   - a pid file in the artifact dir: local dry-runs and diagnostics.
    if ($env:GITHUB_ENV) {
        Add-Content -Path $env:GITHUB_ENV -Value "POLY_CUBASE_PID=$($proc.Id)"
    }
    $env:POLY_CUBASE_PID = "$($proc.Id)"
    $pidPath = Join-Path (Get-PolyArtifactDir) "cubase-pid.txt"
    Set-Content -Path $pidPath -Value "$($proc.Id)"

    Write-PolyPhase -Phase "launch" -State "ok" `
        -Detail "launched" -Extra @{ pid = $proc.Id; exe = $exe }
} catch {
    Invoke-PolyPhaseFailure -Phase "launch" -Message $_.Exception.Message
}
