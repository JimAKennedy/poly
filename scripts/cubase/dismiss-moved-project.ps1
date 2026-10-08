# M004: answer Cubase's prompts about a project opened from a new folder.
#
# Why this exists: a .cpr records the folder it was saved in. Opened from any
# other folder, Cubase stops before loading and asks which working directory to
# use -- "New (1)" (where the file is now) or "Old (2)" (where it was saved).
# The committed fixtures were saved under the runner's checkout, so the nightly's
# own launch never sees this; the M004 session specs do, because S01 saves and
# reopens a SCRATCH COPY of the fixture and S06 bounces from one. A scratch copy
# must never write back into tests/cubase/fixtures/, so "New" is the only correct
# answer: it keeps every write inside the copy's own folder.
#
# The buttons are exposed to UI Automation as plain panes with no Invoke
# pattern (observed on the runner, 2026-10-07), and dismiss-safe-mode.ps1
# records that SendKeys did not close the dialog it targets. So rather than
# trusting a keystroke, this finds the pane named "New (1)" and clicks the
# centre of its bounding rectangle.
#
# A project can instead raise a "Set Project Folder" picker -- observed on the
# runner (2026-10-07) for copies of a fixture that had itself been saved from a
# scratch copy, while copies of poly-4bar.cpr raise the prompt above and copies
# of the committed poly-2instance.cpr raise nothing. Which one Cubase chooses
# depends on what the .cpr recorded, so this answers either. The picker is a standard
# Windows dialog: the project's own folder is typed in (Enter navigates into it)
# and its Select Folder button invoked through UI Automation, which keeps the
# copy's writes inside the copy for the same reason "New" does.
#
# A no-op when neither appears within the window: a launch on the fixture's own
# folder shows nothing, so running this on every launch is safe.

[CmdletBinding()]
param(
    # The opened project's folder, typed into a "Set Project Folder" picker.
    [string] $ProjectDir = "",
    [int] $TimeoutSeconds = 30,
    [int] $PollMilliseconds = 500
)

. "$PSScriptRoot/_common.ps1"

Write-PolyPhase -Phase "dismiss-moved-project" -State "start" `
    -Extra @{ timeoutSeconds = $TimeoutSeconds }

try {
    Add-Type -AssemblyName UIAutomationClient, UIAutomationTypes
    Add-Type @"
using System;
using System.Runtime.InteropServices;
public static class PolyMouse {
    [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
    [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint x, uint y, uint d, UIntPtr e);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
}
"@

    $root = [System.Windows.Automation.AutomationElement]::RootElement
    $all = [System.Windows.Automation.Condition]::TrueCondition
    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)

    while ((Get-Date) -lt $deadline) {
        $picker = $root.FindAll('Children', $all) |
            Where-Object { $_.Current.Name -eq 'Set Project Folder' } | Select-Object -First 1
        if ($picker) {
            if (-not $ProjectDir) {
                Invoke-PolyPhaseFailure -Phase "dismiss-moved-project" `
                    -Message "a 'Set Project Folder' picker appeared and no -ProjectDir was given"
            }
            Add-Type -AssemblyName System.Windows.Forms
            [PolyMouse]::SetForegroundWindow([IntPtr]$picker.Current.NativeWindowHandle) | Out-Null
            Start-Sleep -Milliseconds 300
            [System.Windows.Forms.SendKeys]::SendWait($ProjectDir)
            Start-Sleep -Milliseconds 300
            [System.Windows.Forms.SendKeys]::SendWait('{ENTER}')
            Start-Sleep -Milliseconds 1000
            $select = $picker.FindAll('Descendants', $all) | Where-Object {
                $_.Current.Name -eq 'Select Folder' -and
                $_.Current.ControlType -eq [System.Windows.Automation.ControlType]::Button
            } | Select-Object -First 1
            if (-not $select) {
                Invoke-PolyPhaseFailure -Phase "dismiss-moved-project" `
                    -Message "the 'Set Project Folder' picker has no Select Folder button"
            }
            $select.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
            Write-PolyPhase -Phase "dismiss-moved-project" -State "ok" `
                -Detail "answered 'Set Project Folder' with the project's own folder" `
                -Extra @{ projectDir = $ProjectDir }
            exit 0
        }

        $dialogs = $root.FindAll('Children', $all) |
            Where-Object { $_.Current.Name -eq 'Cubase Pro' }
        foreach ($dialog in $dialogs) {
            $panes = $dialog.FindAll('Descendants', $all)
            $newPane = $panes | Where-Object { $_.Current.Name -eq 'New (1)' } | Select-Object -First 1
            $oldPane = $panes | Where-Object { $_.Current.Name -eq 'Old (2)' } | Select-Object -First 1
            # Both buttons, or it is some other "Cubase Pro" dialog.
            if (-not ($newPane -and $oldPane)) { continue }

            [PolyMouse]::SetForegroundWindow([IntPtr]$dialog.Current.NativeWindowHandle) | Out-Null
            $r = $newPane.Current.BoundingRectangle
            $x = [int]($r.X + $r.Width / 2)
            $y = [int]($r.Y + $r.Height / 2)
            [PolyMouse]::SetCursorPos($x, $y) | Out-Null
            Start-Sleep -Milliseconds 150
            [PolyMouse]::mouse_event(0x2, 0, 0, 0, [UIntPtr]::Zero) # left down
            [PolyMouse]::mouse_event(0x4, 0, 0, 0, [UIntPtr]::Zero) # left up
            Write-PolyPhase -Phase "dismiss-moved-project" -State "ok" `
                -Detail "answered 'project file has been moved' with New (1)" `
                -Extra @{ x = $x; y = $y }
            exit 0
        }
        Start-Sleep -Milliseconds $PollMilliseconds
    }

    Write-PolyPhase -Phase "dismiss-moved-project" -State "ok" `
        -Detail "no moved-project prompt or folder picker appeared (expected on the fixture's own folder)"
    exit 0
} catch {
    Invoke-PolyPhaseFailure -Phase "dismiss-moved-project" -Message $_.Exception.Message
}
