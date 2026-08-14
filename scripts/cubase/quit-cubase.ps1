# M042 S07: quit Cubase, with a hard-kill fallback.
#
# First attempt: CloseMainWindow (WM_CLOSE) on each Cubase process. On a normal
# desktop this triggers project-close → Hub transition, but Cubase does NOT exit
# — the Steinberg Hub takes over the process and waits for user interaction.
# The "show Hub on startup" preference only controls cold-launch, not this
# close-project transition, so the Hub cannot be suppressed here.
#
# After $GraceSeconds (default 15) we hard-kill. This is the expected path for
# unattended runs — a clean process exit via CloseMainWindow alone would require
# a second WM_CLOSE on the Hub window, which is timing-fragile. The hard-kill
# may cause Cubase's crash reporter to appear on the NEXT launch; if that
# becomes a problem, the kill-stale preflight or a registry key to suppress the
# reporter can handle it.
#
# Runs under the workflow's `if: always()` so it executes even when
# wait-for-ready failed, ensuring the runner is left clean.

[CmdletBinding()]
param(
    [int] $GraceSeconds = 15
)

. "$PSScriptRoot/_common.ps1"

Write-PolyPhase -Phase "quit" -State "start"

# Clear the WebView2 CDP registry policy set by launch-cubase.ps1 (-EnableCdp),
# so no stale global policy lingers on the runner and silently affects other
# WebView2 apps. Best-effort — a leftover value is a hygiene issue, not a run
# failure — and a no-op when the key was never written (S07/S08 flows). Runs
# before the process-quit logic so it executes even on the early-return paths.
try {
    $policyKey = "HKCU:\Software\Policies\Microsoft\Edge\WebView2\AdditionalBrowserArguments"
    if (Test-Path $policyKey) {
        Remove-Item -Path $policyKey -Force -Recurse -ErrorAction SilentlyContinue
        Write-PolyPhase -Phase "quit" -State "ok" -Detail "cleared WebView2 CDP registry policy"
    }
} catch {
    Write-PolyPhase -Phase "quit" -State "ok" `
        -Detail "WebView2 CDP registry policy clear skipped: $($_.Exception.Message)"
}

try {
    $names = Get-CubaseProcessNames
    $procs = Get-Process -Name $names -ErrorAction SilentlyContinue
    if (-not $procs) {
        Write-PolyPhase -Phase "quit" -State "ok" -Detail "no Cubase process to quit"
        return
    }

    # Ask nicely first.
    foreach ($p in $procs) {
        try { $p.CloseMainWindow() | Out-Null } catch { }
    }

    # Wait for graceful exit.
    $deadline = (Get-Date).AddSeconds($GraceSeconds)
    while ((Get-Date) -lt $deadline) {
        $still = Get-Process -Name $names -ErrorAction SilentlyContinue
        if (-not $still) {
            Write-PolyPhase -Phase "quit" -State "ok" -Detail "clean graceful exit"
            return
        }
        Start-Sleep -Seconds 1
    }

    # Hard-kill fallback. Expected path — Cubase transitions to the Hub on
    # WM_CLOSE and won't exit on its own without further interaction.
    $remaining = Get-Process -Name $names -ErrorAction SilentlyContinue
    foreach ($p in $remaining) {
        try {
            Stop-Process -Id $p.Id -Force -ErrorAction Stop
            Write-PolyPhase -Phase "quit" -State "ok" `
                -Detail "hard-killed Cubase after graceful timeout (Hub transition blocks clean exit)" `
                -Extra @{ pid = $p.Id; graceSeconds = $GraceSeconds }
        } catch {
            Write-PolyPhase -Phase "quit" -State "ok" `
                -Detail "process $($p.Id) exited during hard-kill" -Extra @{ pid = $p.Id }
        }
    }
} catch {
    Invoke-PolyPhaseFailure -Phase "quit" -Message $_.Exception.Message
}
