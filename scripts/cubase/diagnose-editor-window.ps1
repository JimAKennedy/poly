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
# Usage (a NORMAL, non-elevated PowerShell on the runner — an elevated shell
# would report its own elevation, not the nightly's — with Cubase already open
# on the fixture and the Poly editor visible):
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

[System.Runtime.InteropServices.DllImport("kernel32.dll")]
public static extern bool ProcessIdToSessionId(uint dwProcessId, out uint pSessionId);

[System.Runtime.InteropServices.DllImport("kernel32.dll")]
public static extern uint WTSGetActiveConsoleSessionId();

// OpenInputDesktop succeeds only when the interactive input desktop is
// available to this process (i.e. the console is unlocked and attached). It
// returns NULL when the session is locked / no interactive input desktop is
// reachable — the discriminating signal for "editor paints but WebView2's CDP
// pipe can't bind because the input desktop is locked/headless".
[System.Runtime.InteropServices.DllImport("user32.dll", SetLastError = true)]
public static extern System.IntPtr OpenInputDesktop(uint dwFlags, bool fInherit, uint dwDesiredAccess);

[System.Runtime.InteropServices.DllImport("user32.dll", SetLastError = true)]
public static extern bool CloseDesktop(System.IntPtr hDesktop);
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

# --- Session / window-station / lock-state block -----------------------------
# Context for the unattended-agent CDP failure (the editor + WebView2 host
# materialize, but the CDP port never binds). Three candidate causes were in
# play; the raw facts below distinguish them:
#   (a) Cubase runs on a NON-interactive session/station (Session 0 service
#       isolation) — WebView2's remote-debugging pipe can't bind. Fix: run the
#       runner as an interactive logon task on the console session (runbook Part
#       7/9), NOT a service.
#   (b) Cubase IS on the interactive session but the console is LOCKED / has no
#       attached display at WebView2-init time — the input desktop is
#       unreachable. Fix: keep-awake + dummy-HDMI + auto-unlock (keep-awake doc).
#   (c) Cubase runs ELEVATED, so WebView2 ignores the CDP flag by design. Fix:
#       register the runner's logon task with -RunLevel Limited.
# Runs #40-#47 reported session 1, console matched, input desktop reachable —
# ruling out (a) and (b) — and the cause was (c). The elevation line below is
# therefore the one to read first when the port is missing.
Write-Diag ""
Write-Diag "=== session / window-station / desktop-lock state ==="
$activeConsole = [Win32.Enum]::WTSGetActiveConsoleSessionId()
Write-Diag "  active physical console session id: $activeConsole"
foreach ($cp in $cubasePids) {
    $sid = [uint32]0
    $ok = [Win32.Enum]::ProcessIdToSessionId([uint32]$cp, [ref]$sid)
    $sidText = if ($ok) { "$sid" } else { "unknown" }
    $sameAsConsole = if ($ok -and $sid -eq $activeConsole) { "YES" } else { "NO" }
    Write-Diag "  Cubase pid $cp -> session id: $sidText (matches active console: $sameAsConsole)"
}
# Session 0 => service-isolated, no interactive desktop => cause (a).
# A pid on the active console session id => interactive => points at cause (b).

# Input-desktop reachability: OpenInputDesktop(0, false, DESKTOP_READOBJECTS=0x1)
# returns a handle only when the interactive input desktop is available (console
# unlocked + attached). NULL => locked/headless => cause (b).
$inputDesk = [Win32.Enum]::OpenInputDesktop(0, $false, 0x0001)
if ($inputDesk -ne [System.IntPtr]::Zero) {
    Write-Diag "  interactive input desktop reachable: YES (console unlocked / attached)"
    [void][Win32.Enum]::CloseDesktop($inputDesk)
} else {
    Write-Diag "  interactive input desktop reachable: NO (console LOCKED or headless — WebView2 CDP pipe likely blocked)"
}

# Elevation of THIS process — which is also Cubase's, since launch-cubase.ps1
# starts Cubase as a child of this same job shell. This is the discriminator
# that actually mattered (MEM118 superseded): WebView2 ignores browser flags set
# "via the local device environment" (env var + registry policy) whenever the
# host app is elevated, so an elevated Cubase can never expose CDP.
# See docs/windows-test-runner-setup.md Part 12.
$isElevated = ([Security.Principal.WindowsPrincipal]`
    [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator)
if ($isElevated) {
    Write-Diag "  this process elevated: YES — WebView2 WILL ignore the CDP env var/registry flag (expect no listener)"
} else {
    Write-Diag "  this process elevated: NO (required for CDP — WebView2 honors the env var only for non-elevated hosts)"
}

# The most direct evidence: read the WebView2 browser child's command line and
# confirm whether --remote-debugging-port actually reached the process. If the
# msedgewebview2.exe children exist but NONE carry the flag despite env var +
# registry policy, the flag is being stripped/ignored at spawn — a delivery
# problem, not a session problem.
Write-Diag "=== WebView2 (msedgewebview2.exe) child command lines ==="
$wv = Get-CimInstance Win32_Process -Filter "Name='msedgewebview2.exe'" -ErrorAction SilentlyContinue
if ($wv) {
    foreach ($p in $wv) {
        $hasFlag = if ($p.CommandLine -match 'remote-debugging-port') { 'HAS --remote-debugging-port' } else { 'no debug flag' }
        $typeArg = if ($p.CommandLine -match '--type=(\S+)') { $Matches[1] } else { '(browser/root)' }
        Write-Diag "  pid $($p.ProcessId) [$typeArg] $hasFlag"
    }
    $anyFlag = $wv | Where-Object { $_.CommandLine -match 'remote-debugging-port' }
    Write-Diag "  => any WebView2 process carries --remote-debugging-port: $(if ($anyFlag) { 'YES' } else { 'NO' })"
} else {
    Write-Diag "  no msedgewebview2.exe processes found"
}

$fg = [Win32.Enum]::GetForegroundWindow()
Write-Diag ""
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
