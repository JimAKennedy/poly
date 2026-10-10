# M004 S06: press "Export Audio" in Cubase's Export Audio Mixdown dialog.
#
# remote.py `export` opens the dialog through the MIDI Remote surface (the
# File > Export Audio Mixdown key command); this presses its Export Audio button
# and waits for the render to finish, which Cubase signals by closing the
# dialog. The dialog's defaults are what the spec relies on, observed on the
# runner (Cubase 14, 2026-10-07): range = Locators, Real Time Export off (so
# this is the offline path), and the file written to <project>/Mixdown/,
# relative to the open project -- inside a session's scratch copy.
#
# The button is not exposed to UI Automation: the dialog lists only its window
# and title bar. So it is clicked at its offset from the dialog's bottom-right
# corner, which is stable because the dialog opens at a fixed size. If a Cubase
# update moves it, the symptom is this script timing out with the dialog still
# open -- the failure message says so rather than leaving a hung run.

[CmdletBinding()]
param(
    [int] $OpenTimeoutSeconds = 20,
    [int] $RenderTimeoutSeconds = 120,
    # The Export Audio button's centre, measured from the dialog's bottom-right.
    [int] $ButtonFromRight = 116,
    [int] $ButtonFromBottom = 53
)

. "$PSScriptRoot/_common.ps1"

$DialogName = 'Export Audio Mixdown'
Write-PolyPhase -Phase "export-mixdown" -State "start"

try {
    Add-Type -AssemblyName UIAutomationClient, UIAutomationTypes
    $root = [System.Windows.Automation.AutomationElement]::RootElement
    $all = [System.Windows.Automation.Condition]::TrueCondition
    function Find-Dialog {
        $root.FindAll('Children', $all) |
            Where-Object { $_.Current.Name -eq $DialogName } |
            Select-Object -First 1
    }

    $dialog = $null
    $deadline = (Get-Date).AddSeconds($OpenTimeoutSeconds)
    while (-not $dialog -and (Get-Date) -lt $deadline) {
        $dialog = Find-Dialog
        if (-not $dialog) { Start-Sleep -Milliseconds 250 }
    }
    if (-not $dialog) {
        Invoke-PolyPhaseFailure -Phase "export-mixdown" `
            -Message "the '$DialogName' dialog did not open within ${OpenTimeoutSeconds}s"
    }
    # Let the dialog finish laying out before clicking into it.
    Start-Sleep -Milliseconds 750

    $r = $dialog.Current.BoundingRectangle
    $x = [int]($r.Right - $ButtonFromRight)
    $y = [int]($r.Bottom - $ButtonFromBottom)
    # Clicks only if the point is inside this dialog (_common.ps1): a plain
    # click at these coordinates goes to whatever window is on top there.
    Invoke-PolyClickInWindow -Hwnd ([IntPtr]$dialog.Current.NativeWindowHandle) -X $x -Y $y
    Write-PolyPhase -Phase "export-mixdown" -State "ok" -Detail "pressed Export Audio" `
        -Extra @{ x = $x; y = $y }

    $deadline = (Get-Date).AddSeconds($RenderTimeoutSeconds)
    while ((Find-Dialog) -and (Get-Date) -lt $deadline) { Start-Sleep -Milliseconds 500 }
    if (Find-Dialog) {
        Invoke-PolyPhaseFailure -Phase "export-mixdown" `
            -Message ("the dialog is still open ${RenderTimeoutSeconds}s after pressing Export Audio " +
                "at ($x, $y) -- if the render did not start, the button has moved; re-measure " +
                "-ButtonFromRight/-ButtonFromBottom from a screenshot")
    }
    Write-PolyPhase -Phase "export-mixdown" -State "ok" -Detail "render finished (dialog closed)"
    exit 0
} catch {
    Invoke-PolyPhaseFailure -Phase "export-mixdown" -Message $_.Exception.Message
}
