# Save a screenshot of the interactive desktop, for diagnosing an unattended
# Cubase run after the fact.
#
# Most of what goes wrong in a nightly session is visible on screen and nowhere
# else: a dialog nobody answered, a click that did not land, an editor that
# never opened. The phase log can only say that a later wait timed out. A
# screenshot taken at the moment of failure, uploaded with the run's
# artifacts, says why.
#
# Never fails the run: a desktop that cannot be captured (no interactive
# session, a locked workstation) is itself the finding, and is logged as such.

[CmdletBinding()]
param(
    [Parameter(Mandatory)] [string] $OutputPath
)

. "$PSScriptRoot/_common.ps1"

try {
    Add-Type -AssemblyName System.Windows.Forms, System.Drawing
    $bounds = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
    $bitmap = New-Object System.Drawing.Bitmap $bounds.Width, $bounds.Height
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $graphics.CopyFromScreen($bounds.Location, [System.Drawing.Point]::Empty, $bounds.Size)
    $dir = Split-Path -Parent $OutputPath
    if ($dir) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }
    $bitmap.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $graphics.Dispose()
    $bitmap.Dispose()
    Write-PolyPhase -Phase "capture-desktop" -State "ok" -Detail "saved" -Extra @{ path = $OutputPath }
} catch {
    Write-PolyPhase -Phase "capture-desktop" -State "ok" `
        -Detail "could not capture the desktop: $($_.Exception.Message)" -Extra @{ path = $OutputPath }
}
