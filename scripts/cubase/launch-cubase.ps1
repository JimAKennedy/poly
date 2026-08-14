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
    if ($EnableCdp) {
        $cdpArgs = "--remote-debugging-port=$CdpPort"
        $env:WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS = $cdpArgs

        # Belt-and-suspenders: also set the per-app-exe WebView2 policy in the
        # registry. Runner evidence (M042 S09, run #42): the env var alone was
        # NOT honored on the unattended Actions-agent launch even though the
        # editor + choc WebView2 host DID materialize (topology diagnostic:
        # editor frame PRESENT, choc host PRESENT, CDP port ABSENT). The
        # additionalBrowserArguments in WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS
        # only takes effect at the first WebView2 environment creation for a
        # given user-data-dir, so timing/session differences can silently drop
        # it. The registry policy is honored regardless of that timing.
        #
        # Under HKCU\...\WebView2\AdditionalBrowserArguments the VALUE NAME is
        # the host exe (Cubase<ver>.exe) and the DATA is the args string. We set
        # the specific exe name AND the "*" wildcard fallback so we are covered
        # regardless of which exe name choc's WebView2 attributes to. Values are
        # overwritten (Force) each launch and cleared on quit, so no stale global
        # policy lingers on the runner.
        $exeName = Split-Path -Leaf $exe
        $policyKey = "HKCU:\Software\Policies\Microsoft\Edge\WebView2\AdditionalBrowserArguments"
        New-Item -Path $policyKey -Force | Out-Null
        New-ItemProperty -Path $policyKey -Name $exeName -Value $cdpArgs `
            -PropertyType String -Force | Out-Null
        New-ItemProperty -Path $policyKey -Name "*" -Value $cdpArgs `
            -PropertyType String -Force | Out-Null

        Write-PolyPhase -Phase "launch" -State "ok" `
            -Detail "WebView2 CDP enabled (env var + registry policy)" `
            -Extra @{ cdpPort = $CdpPort; policyExe = $exeName }
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
