# M042 S07: shared helpers for the Cubase launch/quit machinery.
#
# Dot-source this at the top of each phase script:
#   . "$PSScriptRoot/_common.ps1"
#
# Provides structured phase logging and durable status/error persistence so an
# unattended nightly failure is diagnosable from artifacts alone (observability
# discipline: log decisions, fail loud, persist the reason). Nothing here
# touches Cubase — it is pure plumbing reused by every phase script.

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# Where phase status is persisted. Defaults to a repo-local _artifacts dir so a
# local dry-run works without the workflow env; the workflow sets
# POLY_ARTIFACT_DIR to the run's staging dir.
function Get-PolyArtifactDir {
    if ($env:POLY_ARTIFACT_DIR) { return $env:POLY_ARTIFACT_DIR }
    return (Join-Path (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)) "_artifacts")
}

function Get-PolyStatusPath {
    return (Join-Path (Get-PolyArtifactDir) "cubase-run-status.jsonl")
}

# One structured JSONL line per phase transition. `phase` is the script name
# (kill-stale / launch / wait-ready / quit / archive); `state` is one of
# start / ok / fail; `detail` is a short human note; extra is a hashtable of
# domain fields (pid, path, elapsedSeconds, ...).
function Write-PolyPhase {
    param(
        [Parameter(Mandatory)] [string] $Phase,
        [Parameter(Mandatory)] [ValidateSet("start", "ok", "fail")] [string] $State,
        [string] $Detail = "",
        [hashtable] $Extra = @{}
    )
    $dir = Get-PolyArtifactDir
    New-Item -ItemType Directory -Force -Path $dir | Out-Null

    $record = [ordered]@{
        ts     = (Get-Date).ToUniversalTime().ToString("o")
        phase  = $Phase
        state  = $State
        detail = $Detail
    }
    foreach ($k in $Extra.Keys) { $record[$k] = $Extra[$k] }

    $json = ($record | ConvertTo-Json -Compress -Depth 5)
    Add-Content -Path (Get-PolyStatusPath) -Value $json

    # Also echo to the workflow log so it is visible in the live run, not only
    # in the archived artifact.
    $marker = switch ($State) { "fail" { "::error::" } default { "" } }
    Write-Host "$marker[cubase:$Phase] $State $Detail"
}

# Persist a terminal failure to a dedicated file a cold-start reader checks
# first, then rethrow so the workflow step fails loud (no swallowing).
function Invoke-PolyPhaseFailure {
    param(
        [Parameter(Mandatory)] [string] $Phase,
        [Parameter(Mandatory)] [string] $Message,
        [hashtable] $Extra = @{}
    )
    Write-PolyPhase -Phase $Phase -State "fail" -Detail $Message -Extra $Extra
    $errPath = Join-Path (Get-PolyArtifactDir) "cubase-last-error.json"
    $payload = [ordered]@{
        ts      = (Get-Date).ToUniversalTime().ToString("o")
        phase   = $Phase
        message = $Message
    }
    foreach ($k in $Extra.Keys) { $payload[$k] = $Extra[$k] }
    Set-Content -Path $errPath -Value ($payload | ConvertTo-Json -Depth 5)
    throw "$Phase failed: $Message"
}

# Resolve the Cubase executable path for a given major version. Windows install
# layout is stable: C:\Program Files\Steinberg\Cubase <ver>\Cubase<ver>.exe.
# Returns $null if not found (caller decides whether that is fatal).
function Get-CubaseExePath {
    param([Parameter(Mandatory)] [string] $CubaseVersion)
    $candidates = @(
        "C:\Program Files\Steinberg\Cubase $CubaseVersion\Cubase$CubaseVersion.exe",
        "C:\Program Files\Steinberg\Cubase $CubaseVersion\Cubase.exe"
    )
    foreach ($c in $candidates) {
        if (Test-Path $c) { return $c }
    }
    return $null
}

# All Cubase process names we might need to match, version-agnostic.
function Get-CubaseProcessNames {
    return @("Cubase", "Cubase14", "Cubase13", "Cubase12")
}

# Click a point that belongs to one specific top-level window, or refuse.
#
# The scripts that answer Cubase's dialogs used to raise the dialog with
# SetForegroundWindow and click at screen coordinates. Windows ignores
# SetForegroundWindow from a background process more often than not, so the
# click went to whatever window was on top at that point -- observed on the
# runner (2026-10-10): the moved-project dialog sat behind an open Windows
# Update window and the click expanded that window's update list. That made
# every M004 session in the 2026-10-09 and -10 nightlies fail, and a click into
# another application is unsafe, not just flaky.
#
# So the window is made TOPMOST first -- which needs no foreground permission --
# and the click happens only once WindowFromPoint confirms the point is inside
# that window. Otherwise this throws without clicking anything.
function Invoke-PolyClickInWindow {
    param(
        [Parameter(Mandatory)] [IntPtr] $Hwnd,
        [Parameter(Mandatory)] [int] $X,
        [Parameter(Mandatory)] [int] $Y,
        [int] $TimeoutMs = 3000
    )
    if (-not ("PolyWin32Click" -as [type])) {
        Add-Type @"
using System;
using System.Runtime.InteropServices;
public static class PolyWin32Click {
    [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X; public int Y; }
    [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr h, IntPtr after, int x, int y, int cx, int cy, uint flags);
    [DllImport("user32.dll")] public static extern IntPtr WindowFromPoint(POINT p);
    [DllImport("user32.dll")] public static extern IntPtr GetAncestor(IntPtr h, uint flags);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
    [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
    [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint x, uint y, uint d, UIntPtr e);
}
"@
    }
    $topmost = [IntPtr](-1)
    $notTopmost = [IntPtr](-2)
    $noMoveNoSizeShow = 0x0001 -bor 0x0002 -bor 0x0040
    [PolyWin32Click]::SetWindowPos($Hwnd, $topmost, 0, 0, 0, 0, $noMoveNoSizeShow) | Out-Null
    try {
        [PolyWin32Click]::SetForegroundWindow($Hwnd) | Out-Null

        $point = New-Object PolyWin32Click+POINT
        $point.X = $X
        $point.Y = $Y
        $deadline = (Get-Date).AddMilliseconds($TimeoutMs)
        $owner = [IntPtr]::Zero
        do {
            $hit = [PolyWin32Click]::WindowFromPoint($point)
            $owner = [PolyWin32Click]::GetAncestor($hit, 2) # GA_ROOT
            if ($owner -eq $Hwnd) { break }
            Start-Sleep -Milliseconds 100
        } while ((Get-Date) -lt $deadline)
        if ($owner -ne $Hwnd) {
            throw ("refusing to click ($X, $Y): the window there is $owner, not the target $Hwnd " +
                "-- another window covers it even after making the target topmost")
        }

        [PolyWin32Click]::SetCursorPos($X, $Y) | Out-Null
        Start-Sleep -Milliseconds 150
        [PolyWin32Click]::mouse_event(0x2, 0, 0, 0, [UIntPtr]::Zero) # left down
        [PolyWin32Click]::mouse_event(0x4, 0, 0, 0, [UIntPtr]::Zero) # left up
        # Injected input is hit-tested asynchronously; stay topmost until the
        # click has been delivered.
        Start-Sleep -Milliseconds 300
    } finally {
        # A dialog answered by the click is gone; one refused stays put, and
        # must not be left floating over everything else on the desktop.
        [PolyWin32Click]::SetWindowPos($Hwnd, $notTopmost, 0, 0, 0, 0, 0x0001 -bor 0x0002) | Out-Null
    }
}
