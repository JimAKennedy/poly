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
# pattern (observed on the runner, 2026-10-07), so the pane named "New (1)" is
# clicked at its centre -- through Invoke-PolyClickInWindow, which makes the
# dialog topmost and refuses to click unless the point is inside it. A plain
# click at those coordinates once landed in a Windows Update window that
# covered the dialog (2026-10-10).
#
# A project can instead raise a "Set Project Folder" picker -- observed on the
# runner (2026-10-07) for copies of a fixture that had itself been saved from a
# scratch copy, while copies of poly-4bar.cpr raise the prompt above and copies
# of the committed poly-2instance.cpr raise nothing. Which one Cubase chooses
# depends on what the .cpr recorded, so this answers either. The picker is a standard
# Windows dialog, answered entirely through UI Automation -- the project's own
# folder set in its field, Select Folder invoked -- which keeps the copy's
# writes inside the copy for the same reason "New" does.
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
            # No keystrokes: SendKeys goes to whichever window has focus, which
            # need not be this dialog. The folder field is set through UI
            # Automation and Select Folder invoked the same way. A folder
            # picker given a path navigates to it on the first Select Folder
            # and selects it on the second, so it is invoked until the picker
            # closes, at most three times.
            $edit = $picker.FindAll('Descendants', $all) | Where-Object {
                $_.Current.ControlType -eq [System.Windows.Automation.ControlType]::Edit -and
                $_.Current.Name -eq 'Folder:'
            } | Select-Object -First 1
            $select = $picker.FindAll('Descendants', $all) | Where-Object {
                $_.Current.Name -eq 'Select Folder' -and
                $_.Current.ControlType -eq [System.Windows.Automation.ControlType]::Button
            } | Select-Object -First 1
            if (-not ($edit -and $select)) {
                Invoke-PolyPhaseFailure -Phase "dismiss-moved-project" `
                    -Message "the 'Set Project Folder' picker has no folder field or Select Folder button"
            }
            $edit.GetCurrentPattern([System.Windows.Automation.ValuePattern]::Pattern).SetValue($ProjectDir)
            for ($attempt = 0; $attempt -lt 3; $attempt++) {
                try {
                    $select.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
                } catch {
                    break # the picker closed under us: done
                }
                Start-Sleep -Milliseconds 1000
                $still = $root.FindAll('Children', $all) |
                    Where-Object { $_.Current.Name -eq 'Set Project Folder' } | Select-Object -First 1
                if (-not $still) { break }
            }
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

            $r = $newPane.Current.BoundingRectangle
            $x = [int]($r.X + $r.Width / 2)
            $y = [int]($r.Y + $r.Height / 2)
            # Clicks only if the point is inside this dialog (_common.ps1).
            Invoke-PolyClickInWindow -Hwnd ([IntPtr]$dialog.Current.NativeWindowHandle) -X $x -Y $y
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
