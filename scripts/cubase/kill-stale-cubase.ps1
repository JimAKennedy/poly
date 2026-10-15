# M042 S07: terminate any lingering Cubase process before a run.
#
# A previous run that failed to quit cleanly leaves Cubase holding the audio
# device / MIDI ports / project lock. Kill it first so the launch phase starts
# from a known-clean state. Idempotent — a no-op when nothing is running.
#
# M042 S09: also kill stale msedgewebview2.exe processes. choc sets no explicit
# WebView2 userDataFolder, so all Cubase-hosted WebView2 instances share the
# default per-host-exe data dir. --remote-debugging-port (via
# WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS) is honored ONLY at first browser-process
# creation for a given data dir; if a prior run's Hub-blocked hard-kill orphaned
# an msedgewebview2.exe that still owns that dir, the new Cubase's WebView2
# attaches to it and the CDP port flag is silently DROPPED (editor visible, port
# absent). Cubase's own hard-kill does not reap its WebView2 children, so we do
# it here. Kill Cubase FIRST so it cannot respawn a child mid-sweep.
#
# Also kill the Edge browser (msedge.exe -- NOT msedgewebview2, handled above).
# A Windows feature update re-launches Edge at logon even with it removed from
# startup apps, and a stray browser window on the runner's desktop can cover the
# Cubase dialogs the session scripts click. Each kill is logged, so the run's
# artifacts show how often it comes back.

[CmdletBinding()]
param()

. "$PSScriptRoot/_common.ps1"

Write-PolyPhase -Phase "kill-stale" -State "start"

# Terminate every process matching a name set, logging each kill. Shared by the
# Cubase sweep and the WebView2 sweep below.
function Stop-StaleByName {
    param(
        [Parameter(Mandatory)] [string[]] $Names,
        [Parameter(Mandatory)] [string]   $Kind
    )
    $procs = Get-Process -Name $Names -ErrorAction SilentlyContinue
    if (-not $procs) {
        Write-PolyPhase -Phase "kill-stale" -State "ok" -Detail "no stale $Kind process"
        return
    }
    foreach ($p in $procs) {
        try {
            Stop-Process -Id $p.Id -Force -ErrorAction Stop
            Write-PolyPhase -Phase "kill-stale" -State "ok" `
                -Detail "terminated $($p.ProcessName)" -Extra @{ pid = $p.Id; kind = $Kind }
        } catch {
            # Non-fatal: one process may already be gone by the time we reach it.
            Write-PolyPhase -Phase "kill-stale" -State "ok" `
                -Detail "process $($p.Id) already exited or unkillable" -Extra @{ pid = $p.Id; kind = $Kind }
        }
    }
}

try {
    # Cubase FIRST — a live Cubase would respawn WebView2 children if we reaped
    # them while it still runs.
    Stop-StaleByName -Names (Get-CubaseProcessNames) -Kind "Cubase"

    # Then the WebView2 browser processes Cubase's hard-kill orphans. Reaping
    # these frees the shared user-data-dir so the next launch's
    # --remote-debugging-port flag is honored at first browser-process creation.
    Stop-StaleByName -Names @("msedgewebview2") -Kind "WebView2"

    # The Edge browser itself, re-launched by Windows Update (see header). Not
    # waited on below: it holds nothing the launch needs, only screen space.
    Stop-StaleByName -Names @("msedge") -Kind "Edge"

    # M004: and WAIT for them to be gone. Stop-Process -Force returns before the
    # process has exited, so a launch straight after this sweep can still find
    # a dying msedgewebview2 holding the data dir -- and attach to it, dropping
    # the CDP flag exactly as described above. Run 37710075119 lost that race:
    # three WebView2 processes were "already exited or unkillable" mid-sweep,
    # and the next Cubase's editor never opened its CDP port. Local launches
    # had always won it. Bounded, and not fatal: if something survives, the
    # launch proceeds and focus-editor-cdp.ps1 reports the missing port.
    $deadline = (Get-Date).AddSeconds(20)
    $names = @(Get-CubaseProcessNames) + @("msedgewebview2")
    while ((Get-Process -Name $names -ErrorAction SilentlyContinue) -and (Get-Date) -lt $deadline) {
        Start-Sleep -Milliseconds 250
    }
    $survivors = Get-Process -Name $names -ErrorAction SilentlyContinue
    if ($survivors) {
        Write-PolyPhase -Phase "kill-stale" -State "ok" `
            -Detail "processes still present 20s after the sweep" `
            -Extra @{ pids = (($survivors | ForEach-Object { $_.Id }) -join ",") }
    } else {
        Write-PolyPhase -Phase "kill-stale" -State "ok" -Detail "no Cubase or WebView2 process remains"
    }
} catch {
    Invoke-PolyPhaseFailure -Phase "kill-stale" -Message $_.Exception.Message
}
