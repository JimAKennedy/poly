# M042 S09 diagnostic: dump the window topology of the running Cubase process
# tree and probe the CDP port, so we can target the RIGHT window when forcing
# the Poly editor foreground.
#
# Why this exists: the automated focus step foregrounds the Cubase MAIN project
# window (Get-Process .MainWindowHandle), but the Poly editor is a SEPARATE
# window whose WebView2 exposes CDP. We need to know that window's title/class/
# handle and whether foregrounding IT (not the main window) brings the CDP port
# up. Run this by hand in the VNC session (the context where CDP was confirmed
# working) and paste the output back.
#
# It changes nothing: pure enumeration + a read-only port probe. Safe to run
# with Cubase open on the fixture with the Poly editor visible.
#
# Usage (elevated PowerShell on the runner, with Cubase already open on the
# fixture and the Poly editor visible):
#   ./scripts/cubase/diagnose-editor-window.ps1 -CdpPort 9222

[CmdletBinding()]
param(
    [int] $CdpPort = 9222,
    # Restrict enumeration to this pid's window tree. Falls back to
    # POLY_CUBASE_PID, then to every Cubase-named process.
    [int] $ExpectedPid = 0,
    # When set, the full topology report is ALSO written here (in addition to the
    # console). The nightly points this at the artifact dir so the window
    # topology under the Actions agent survives in cubase-nightly-artifacts even
    # when the run is unattended and nobody is watching the console.
    [string] $OutputPath = ""
)

. "$PSScriptRoot/_common.ps1"

if ($ExpectedPid -le 0 -and $env:POLY_CUBASE_PID) {
    $ExpectedPid = [int]$env:POLY_CUBASE_PID
}

# Tee helper: echo to console AND, when -OutputPath is set, append to the file.
# Used for every report line so the artifact captures the same picture the
# operator would see interactively.
$script:PolyDiagLines = New-Object System.Collections.ArrayList
function Write-Diag {
    param([string] $Line = "")
    [void]$script:PolyDiagLines.Add($Line)
    Write-Host $Line
}

# --- Win32 window enumeration -------------------------------------------------
Add-Type -Namespace Win32 -Name Enum -MemberDefinition @'
[System.Runtime.InteropServices.DllImport("user32.dll")]
public static extern bool EnumWindows(EnumWindowsProc lpEnumFunc, System.IntPtr lParam);
public delegate bool EnumWindowsProc(System.IntPtr hWnd, System.IntPtr lParam);

[System.Runtime.InteropServices.DllImport("user32.dll", SetLastError = true)]
public static extern uint GetWindowThreadProcessId(System.IntPtr hWnd, out uint lpdwProcessId);

[System.Runtime.InteropServices.DllImport("user32.dll")]
public static extern bool IsWindowVisible(System.IntPtr hWnd);

[System.Runtime.InteropServices.DllImport("user32.dll", CharSet = System.Runtime.InteropServices.CharSet.Unicode)]
public static extern int GetWindowTextW(System.IntPtr hWnd, System.Text.StringBuilder lpString, int nMaxCount);

[System.Runtime.InteropServices.DllImport("user32.dll", CharSet = System.Runtime.InteropServices.CharSet.Unicode)]
public static extern int GetClassNameW(System.IntPtr hWnd, System.Text.StringBuilder lpClassName, int nMaxCount);

[System.Runtime.InteropServices.DllImport("user32.dll")]
public static extern System.IntPtr GetForegroundWindow();
'@

# Build the set of pids in the Cubase process tree we care about.
$cubasePids = @()
if ($ExpectedPid -gt 0) {
    $cubasePids += $ExpectedPid
} else {
    foreach ($n in (Get-CubaseProcessNames)) {
        Get-Process -Name $n -ErrorAction SilentlyContinue | ForEach-Object { $cubasePids += $_.Id }
    }
}
$cubasePids = $cubasePids | Sort-Object -Unique
Write-Diag "=== Cubase pids under inspection: $($cubasePids -join ', ') ==="

$fg = [Win32.Enum]::GetForegroundWindow()
Write-Diag "=== foreground window handle right now: $fg ==="

# Enumerate every top-level window; keep those owned by a Cubase pid.
$rows = New-Object System.Collections.ArrayList
$cb = [Win32.Enum+EnumWindowsProc] {
    param([System.IntPtr] $hWnd, [System.IntPtr] $lParam)
    # NB: do NOT name this $pid — that is a read-only PowerShell automatic
    # variable (the current process id) and [ref]$pid would throw.
    $winPid = 0
    [void][Win32.Enum]::GetWindowThreadProcessId($hWnd, [ref]$winPid)
    if ($cubasePids -contains [int]$winPid) {
        $sbT = New-Object System.Text.StringBuilder 512
        [void][Win32.Enum]::GetWindowTextW($hWnd, $sbT, 512)
        $sbC = New-Object System.Text.StringBuilder 256
        [void][Win32.Enum]::GetClassNameW($hWnd, $sbC, 256)
        [void]$rows.Add([pscustomobject]@{
            Pid     = [int]$winPid
            HWnd    = [int64]$hWnd
            Visible = [Win32.Enum]::IsWindowVisible($hWnd)
            IsFg    = ($hWnd -eq $fg)
            Class   = $sbC.ToString()
            Title   = $sbT.ToString()
        })
    }
    return $true
}
[void][Win32.Enum]::EnumWindows($cb, [System.IntPtr]::Zero)

Write-Diag ""
Write-Diag "=== top-level windows owned by the Cubase process tree ==="
$rows | Sort-Object Visible -Descending |
    Format-Table Pid, HWnd, Visible, IsFg, Class,
        @{ N = "Title"; E = { if ($_.Title.Length -gt 60) { $_.Title.Substring(0, 60) } else { $_.Title } } } -AutoSize |
    Out-String -Width 200 | Write-Diag

# The Poly editor materializes as two windows even when NOT visible (MEM115):
# the `01 - Poly` frame (SteinbergWindowClass) and the `choc_####` WebView2 host.
# Their PRESENCE — regardless of Visible — is the real signal that the editor
# instantiated. This is the decisive check for the unattended-agent session:
# if these are ABSENT while Cubase is otherwise up, the plugin editor never
# materialized (matches process_calls=0 + no CDP port).
Write-Diag "=== Poly-editor materialization check (presence, not visibility) ==="
$editorFrame = $rows | Where-Object { $_.Class -match 'SteinbergWindow' -and $_.Title -match 'Poly' }
$chocHost = $rows | Where-Object { $_.Class -match '^choc_' }
Write-Diag "  editor frame (`01 - Poly` / SteinbergWindow): $(if ($editorFrame) { 'PRESENT' } else { 'ABSENT' })"
Write-Diag "  WebView2 host (choc_#### class):             $(if ($chocHost) { 'PRESENT' } else { 'ABSENT' })"
if ($editorFrame) { $editorFrame | Format-Table Pid, HWnd, Visible, Class, Title -AutoSize | Out-String -Width 200 | Write-Diag }
if ($chocHost) { $chocHost | Format-Table Pid, HWnd, Visible, Class, Title -AutoSize | Out-String -Width 200 | Write-Diag }

# Highlight likely-editor candidates: a Chrome/WebView2 window class, or a title
# mentioning Poly, or an empty-title visible window (plugin editors often have
# no caption).
Write-Diag "=== likely Poly-editor / WebView2 candidates ==="
$rows | Where-Object {
    $_.Visible -and (
        $_.Class -match 'Chrome_WidgetWin|WebView2|Intermediate D3D' -or
        $_.Title -match 'Poly' -or
        ($_.Title.Trim() -eq '')
    )
} | Format-Table Pid, HWnd, Visible, IsFg, Class, Title -AutoSize |
    Out-String -Width 200 | Write-Diag

# --- CDP port probe (read-only, focus-independent) ---------------------------
Write-Diag "=== CDP listener check on 127.0.0.1:$CdpPort ==="
$listen = Get-NetTCPConnection -State Listen -ErrorAction SilentlyContinue |
    Where-Object { $_.LocalPort -eq $CdpPort }
if ($listen) {
    $listen | Format-Table LocalAddress, LocalPort, OwningProcess -AutoSize |
        Out-String | Write-Diag
    Write-Diag "CDP LISTENER: YES"
    # If listening, try the actual CDP handshake so we know it's a real endpoint.
    try {
        $v = Invoke-RestMethod -Uri "http://127.0.0.1:$CdpPort/json/version" -TimeoutSec 3
        Write-Diag "  /json/version -> $($v.Browser)  (webSocketDebuggerUrl present: $([bool]$v.webSocketDebuggerUrl))"
    } catch {
        Write-Diag "  /json/version request failed: $($_.Exception.Message)"
    }
} else {
    Write-Diag "CDP LISTENER: NO (nothing listening on $CdpPort at snapshot time)"
}

# Persist the full report to the artifact file when requested, so the topology
# under the unattended agent survives in cubase-nightly-artifacts.
if ($OutputPath) {
    $dir = Split-Path -Parent $OutputPath
    if ($dir) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }
    Set-Content -Path $OutputPath -Value ($script:PolyDiagLines -join "`n")
    Write-Host "[diagnose-editor-window] topology written to $OutputPath"
}
