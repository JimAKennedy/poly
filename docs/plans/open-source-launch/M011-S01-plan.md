# M011/S01 — The four unknowns are measured

**Slice:** M011/S01 in `docs/plans/open-source-launch/ledger.md`
**Rows:** OS45 (four premises of the design are unmeasured)
**Classification:** spike. The output is four measured answers in
`M011-decisions.md`, not code. Every script below lives in this plan and runs
from a paste; none is committed as a file, and every VM, account change and
activation made for a probe is undone by tasks 5 and 8. The design under test
is `M011-install-verify-design.md`.

## Task status

- [ ] 1. The baseline is recorded and the probe decisions are written down
- [ ] 2. A throwaway Windows guest exists with Cubase activated and a `clean` checkpoint
- [ ] 3. Unknown 1: Cubase stays activated across checkpoint restores
- [ ] 4. Unknown 2: a Hyper-V Administrator who is not an administrator drives the guest
- [ ] 5. JIMW1 is returned to its baseline
- [ ] 6. Unknown 4: Screen Recording survives a Tart clone
- [ ] 7. Unknown 3: REAPER runs a ReaScript unattended in a clone
- [ ] 8. The Mac is returned to its baseline and the answers are judged against the design

## Definition of Done

Copied verbatim from the slice:

- [ ] Each of the four questions in OS45 has a measured answer in
      `M011-decisions.md`, with how it was measured
- [ ] Any answer that changes the design has been taken to the owner, and the
      design document and this milestone are amended before S02 or S03 starts
- [ ] Nothing built for the measurement is kept; it is labelled throwaway in
      the evidence

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `doc-discipline` | `bash scripts/check-doc-discipline.sh` |

Both run after every task's commit is staged. Neither exercises a probe — the
probes' own outputs, pasted into the evidence, are what each task is judged on.

## Who runs what

- **Owner** — every step on JIMW1 (console or VNC), every Steinberg account
  check, and every step in a VM's graphical session. The owner pastes each
  command's output back to the executor.
- **Executor** — every step on the Mac's command line, and every write to the
  repo: the decisions file, the evidence file, the ledger, the commits.

A step marked **Owner** is never guessed at by the executor. If the owner is
not available, `/jk:next` stops at that step and says what it is waiting for.

## Names used throughout

- `docs/plans/open-source-launch/M011-decisions.md` — append-only, in the shape
  of `M008-decisions.md`: dated sections, each question, its answer, and how it
  was measured. Created by task 1.
- `docs/plans/open-source-launch/evidence/M011-S01.md` — one `##` entry per
  task, ending with the date, as `evidence/M008-S01.md` does. Created by task 1.
- `poly-spike-vm` — the throwaway Hyper-V guest on JIMW1. Its checkpoint is
  `clean`. Its local administrator is `polytest`.
- `spike-golden`, `spike-clone` — the throwaway Tart VMs on the Mac.
- Hyper-V Administrators is the builtin group with SID `S-1-5-32-578`;
  Administrators is `S-1-5-32-544`.
- Cubase **15** in the guest — the only version the spare seat is known to
  activate. The JIMW1 host stays on Cubase 14 for this slice.

## Task 1 — The baseline is recorded and the probe decisions are written down

**Files:** create `M011-decisions.md` and `evidence/M011-S01.md`.

1. **Owner**, in Steinberg Activation Manager on the Mac and on
   my.steinberg.net: for each Cubase licence, its edition, its version, and
   every computer it is activated on. The design assumes a free seat; this is
   the number task 2 consumes and task 5 must return to.
2. **Owner**, on JIMW1 from an elevated PowerShell:
   ```powershell
   (Get-CimInstance Win32_OperatingSystem).Caption
   Get-WindowsOptionalFeature -Online -FeatureName Microsoft-Hyper-V-All |
       Select-Object FeatureName, State
   Get-LocalGroupMember -SID S-1-5-32-578 | Select-Object Name
   Get-LocalGroupMember -SID S-1-5-32-544 | Select-Object Name
   Get-ChildItem 'C:\Program Files\Steinberg' -Directory | Select-Object Name
   ```
   And, as `polyci` from a normal PowerShell:
   `pwsh -File C:\poly\scripts\cubase\check-runner-posture.ps1 -RequireCdp`.
3. **Executor**, on the Mac: `tart --version` (expect "command not found"),
   `uname -m`, `sw_vers -productVersion`, `df -h /`.
4. **Executor**: create `M011-decisions.md` with a `## 2026-10-13 — planning
   M011/S01` section recording these choices, each with its reason:
   - The guest runs **Cubase 15**: the spare seat is a Cubase 15 licence.
     Upgrading JIMW1's host from 14 to 15 is **not** this slice's — the
     nightly's fixture `poly-4bar.cpr` is tied to the Cubase version it was
     saved in, so the upgrade is S02's first step, gated on a green nightly on
     15 before the guest is built to match.
   - The spike guest is installed from the **Windows 11 Enterprise evaluation**
     ISO, so the throwaway spends no Windows licence. S02's guest uses a Pro
     licence, per the design.
   - Everything a probe builds is undone: the guest is deleted after Cubase is
     deactivated in it, `polyci` leaves Hyper-V Administrators, the Tart VMs
     are deleted. Two things stay, as installed software rather than anything
     built: the Hyper-V feature on JIMW1 if task 2 had to enable it, and
     `tart` with its downloaded base image on the Mac. S02 and S03 need both.
5. **Executor**: write the evidence entry quoting steps 1–3.
6. `format`, `doc-discipline`. Commit with `Rows:` empty.

**Done when** the evidence holds the seat count per licence, JIMW1's edition
and Hyper-V state and both group memberships, a posture check result, and the
Mac's facts; and the decisions file holds the three choices.

## Task 2 — A throwaway Windows guest exists with Cubase activated and a `clean` checkpoint

**Files:** the evidence file.

Run outside the nightly's window (it starts at 02:00). If Hyper-V has to be
enabled, JIMW1 reboots; afterwards run the posture check again.

1. **Owner**, elevated, only if task 1 found Hyper-V `Disabled`:
   `Enable-WindowsOptionalFeature -Online -FeatureName Microsoft-Hyper-V-All`,
   then reboot, log `polyci` back on, and confirm the runner shows online.
2. **Owner**, elevated:
   ```powershell
   New-VM -Name poly-spike-vm -Generation 2 -MemoryStartupBytes 8GB `
          -NewVHDPath 'C:\HyperV\poly-spike-vm.vhdx' -NewVHDSizeBytes 100GB `
          -SwitchName 'Default Switch'
   Set-VMProcessor -VMName poly-spike-vm -Count 4
   Set-VMKeyProtector -VMName poly-spike-vm -NewLocalKeyProtector
   Enable-VMTPM -VMName poly-spike-vm
   Enable-VMIntegrationService -VMName poly-spike-vm -Name 'Guest Service Interface'
   Add-VMDvdDrive -VMName poly-spike-vm -Path '<path to the Windows 11 Enterprise evaluation ISO>'
   ```
   The ISO path is the owner's download location; record it in the evidence.
3. **Owner**, in the guest's console: install Windows; create local
   administrator `polytest`; turn on automatic logon for it; turn off the lock
   screen and sleep; install Cubase 15 and the Steinberg Activation Manager;
   **activate Cubase**; launch it once, so first-run dialogs are answered, and
   quit it.
4. **Owner**: record the seat count again — it must be task 1's plus one.
5. **Owner**, elevated, with the guest shut down:
   `Checkpoint-VM -Name poly-spike-vm -SnapshotName clean`.
6. **Executor**: evidence entry — the Windows build, Cubase's version, the seat
   counts before and after, and `Get-VMSnapshot -VMName poly-spike-vm` output.
   Mark the guest **throwaway**.
7. `format`, `doc-discipline`. Commit with `Rows:` empty.

**Done when** the evidence shows one more seat in use than task 1 and a
checkpoint named `clean`.

## Task 3 — Unknown 1: Cubase stays activated across checkpoint restores

**Files:** the decisions file, the evidence file.

1. **Owner**, elevated, twice in a row:
   ```powershell
   Restore-VMCheckpoint -VMName poly-spike-vm -Name clean -Confirm:$false
   Start-VM -Name poly-spike-vm
   ```
   Each time, in the guest's console after automatic logon: open Activation
   Manager and note whether Cubase shows as activated; launch Cubase and note
   whether it asks for activation, offers a trial, or starts normally; quit it;
   then `Stop-VM -Name poly-spike-vm -TurnOff`.
2. **Owner**: the seat count after the second restore.
3. **Executor**: the answer goes in the decisions file. **Yes** only if both
   restores started Cubase normally *and* the seat count equals task 2's. Any
   other outcome is **No**, recorded with exactly what was seen.
4. Record one limit in the answer whatever it is: two restores minutes apart do
   not show what happens when Steinberg revalidates a licence weeks later.
   S02's guest will meet that, and its evidence should say so if it does.
5. Evidence entry; `format`, `doc-discipline`. Commit with `Rows:` empty.

**Done when** the decisions file answers unknown 1 yes or no, with the two
launches and the seat count behind it.

## Task 4 — Unknown 2: a Hyper-V Administrator who is not an administrator drives the guest

**Files:** the decisions file, the evidence file.

1. **Owner**, as `theji` from an elevated PowerShell:
   `Add-LocalGroupMember -SID S-1-5-32-578 -Member "$env:COMPUTERNAME\polyci"`.
   Then log `polyci` out and back in, since group membership is fixed at logon.
   Confirm the runner shows online afterwards.
2. **Owner**, as `polyci` from a **normal, non-elevated** PowerShell 7. The
   script prints one line per check and saves a screenshot:
   ```powershell
   $vm = 'poly-spike-vm'
   $ErrorActionPreference = 'Stop'
   function Probe($name, [scriptblock]$body) {
       try { & $body; "PROBE OK   $name" } catch { "PROBE FAIL $name -- $($_.Exception.Message)" }
   }
   Probe 'not in Administrators' { if (whoami /groups | Select-String 'S-1-5-32-544') { throw 'polyci is an administrator' } }
   Probe 'in Hyper-V Administrators' { if (-not (whoami /groups | Select-String 'S-1-5-32-578')) { throw 'not a member' } }
   Probe 'process not elevated' { if (([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) { throw 'elevated' } }
   Probe 'restore checkpoint' { Restore-VMCheckpoint -VMName $vm -Name clean -Confirm:$false }
   Probe 'start guest' { Start-VM -Name $vm }
   $cred = Get-Credential -UserName polytest -Message 'polytest password'
   Probe 'PowerShell Direct' {
       $deadline = (Get-Date).AddMinutes(5)
       while ($true) {
           try { $r = Invoke-Command -VMName $vm -Credential $cred -ScriptBlock { "$(hostname) $(whoami)" }; "  guest says: $r"; break }
           catch { if ((Get-Date) -gt $deadline) { throw } ; Start-Sleep 10 }
       }
   }
   Probe 'Copy-VMFile into guest' {
       Copy-VMFile -Name $vm -SourcePath "$env:WINDIR\win.ini" -DestinationPath 'C:\probe\win.ini' -FileSource Host -CreateFullPath -Force
       Invoke-Command -VMName $vm -Credential $cred -ScriptBlock { if (-not (Test-Path 'C:\probe\win.ini')) { throw 'not copied' } }
   }
   Probe 'Copy-Item out of guest' {
       $s = New-PSSession -VMName $vm -Credential $cred
       Invoke-Command -Session $s -ScriptBlock { "probe $(Get-Date -Format o)" | Set-Content 'C:\probe\out.txt' }
       Copy-Item -FromSession $s -Path 'C:\probe\out.txt' -Destination "$env:TEMP\probe-out.txt"
       Remove-PSSession $s
       "  copied out: $(Get-Content "$env:TEMP\probe-out.txt")"
   }
   Probe 'screen capture' {
       Add-Type -AssemblyName System.Drawing
       $ns = 'root\virtualization\v2'
       $svc = Get-CimInstance -Namespace $ns -ClassName Msvm_VirtualSystemManagementService
       $cs = Get-CimInstance -Namespace $ns -ClassName Msvm_ComputerSystem -Filter "ElementName='$vm'"
       $sd = Get-CimAssociatedInstance -InputObject $cs -ResultClassName Msvm_VirtualSystemSettingData |
             Where-Object VirtualSystemType -eq 'Microsoft:Hyper-V:System:Realized'
       $w = 1024; $h = 768
       $r = Invoke-CimMethod -InputObject $svc -MethodName GetVirtualSystemThumbnailImage `
            -Arguments @{ TargetSystem = $sd; WidthPixels = [uint16]$w; HeightPixels = [uint16]$h }
       if ($r.ReturnValue -ne 0) { throw "GetVirtualSystemThumbnailImage returned $($r.ReturnValue)" }
       $bmp = New-Object System.Drawing.Bitmap($w, $h, [System.Drawing.Imaging.PixelFormat]::Format16bppRgb565)
       $data = $bmp.LockBits((New-Object System.Drawing.Rectangle(0, 0, $w, $h)), 'WriteOnly', $bmp.PixelFormat)
       [System.Runtime.InteropServices.Marshal]::Copy([byte[]]$r.ImageData, 0, $data.Scan0, $r.ImageData.Length)
       $bmp.UnlockBits($data)
       $bmp.Save("$env:USERPROFILE\Desktop\probe-guest-screen.png", [System.Drawing.Imaging.ImageFormat]::Png)
       "  saved probe-guest-screen.png ($($r.ImageData.Length) bytes of RGB565)"
   }
   Probe 'turn guest off' { Stop-VM -Name $vm -TurnOff -Force }
   ```
3. **Owner**: open `probe-guest-screen.png` and say whether it shows the
   guest's desktop. A black or garbled image counts as a failed capture, even
   though the call returned 0. Send the image if it is not obviously right.
4. **Owner**: run the posture check as `polyci` again and paste the result.
   Hyper-V Administrators must not have broken any of its four invariants.
5. **Executor**: the answer goes in the decisions file — **yes** only if every
   line says `PROBE OK`, the image shows the desktop, and the posture check
   still passes. A `PROBE FAIL` is recorded with its message, and with
   whichever elevated step would replace it.
6. Evidence entry quoting every probe line; `format`, `doc-discipline`. Commit
   with `Rows:` empty.

**Done when** the decisions file answers unknown 2 yes or no, check by check.

## Task 5 — JIMW1 is returned to its baseline

**Files:** the evidence file.

1. **Owner**: start the guest, open Activation Manager in its console,
   **deactivate Cubase**, and confirm the deactivation went through. This comes
   first: deleting an activated guest would lose the seat until Steinberg
   support frees it.
2. **Owner**: the seat count — it must equal task 1's.
3. **Owner**, elevated:
   ```powershell
   Stop-VM -Name poly-spike-vm -TurnOff -Force
   Remove-VM -Name poly-spike-vm -Force
   Remove-Item 'C:\HyperV\poly-spike-vm.vhdx', 'C:\HyperV\poly-spike-vm' -Recurse -Force -ErrorAction SilentlyContinue
   Remove-LocalGroupMember -SID S-1-5-32-578 -Member "$env:COMPUTERNAME\polyci"
   Get-VM | Select-Object Name
   Get-LocalGroupMember -SID S-1-5-32-578 | Select-Object Name
   ```
   Log `polyci` out and back in; confirm the runner shows online.
4. **Owner**: the posture check as `polyci`; then dispatch **Cubase Nightly
   (L4)** and confirm it is green. The executor can dispatch it with
   `gh workflow run cubase-nightly.yml` and watch the run.
5. **Executor**: evidence entry — the seat count back at baseline, no VM, the
   group back to task 1's members, the posture result, and the nightly's run
   ID and conclusion. Label the guest and the group change **throwaway,
   removed**.
6. `format`, `doc-discipline`. Commit with `Rows:` empty.

**Done when** the seat count, the VM list and the group match task 1, and a
nightly run after the cleanup is green.

## Task 6 — Unknown 4: Screen Recording survives a Tart clone

**Files:** the decisions file, the evidence file.

1. **Executor**:
   ```bash
   brew install cirruslabs/cli/tart
   tart --version
   tart clone ghcr.io/cirruslabs/macos-tahoe-vanilla:latest spike-golden
   tart run --help | grep -i -E 'graphics|vnc' || true
   ```
   The image is about 25 GB. Its user is `admin`, password `admin`.
2. **Executor**: `tart run spike-golden &` (with a window). **Owner**, in that
   window: open System Settings → Privacy & Security → Screen & System Audio
   Recording, add `/usr/libexec/sshd-keygen-wrapper` (the process macOS holds
   responsible for commands run over SSH), and turn it on. Then open TextEdit
   with a document reading `POLY PROBE WINDOW` and leave it open.
3. **Executor**, before cloning, to prove the grant works on the golden image:
   ```bash
   ssh admin@"$(tart ip spike-golden)" 'screencapture -x /tmp/golden.png && ls -l /tmp/golden.png'
   scp admin@"$(tart ip spike-golden)":/tmp/golden.png "$SCRATCH/golden.png"
   ```
   where `$SCRATCH` is the session's scratchpad directory. Read the image: it
   must show the TextEdit window, not only the wallpaper.
4. **Executor**: shut it down cleanly — `ssh admin@"$(tart ip spike-golden)"
   'sudo shutdown -h now'`, then wait for `tart list` to show it stopped. Then
   `tart clone spike-golden spike-clone` and `tart run --no-graphics
   spike-clone &` (if `--help` in step 1 named no such flag, run it with a
   window and say so in the evidence).
5. **Executor**, in the clone: reopen the TextEdit window over SSH with
   `open -a TextEdit` on a file containing `POLY PROBE CLONE`, then
   `screencapture -x /tmp/clone.png`, copy it out, and read it.
6. **Executor**: the answer goes in the decisions file — **yes** only if the
   clone's capture shows the TextEdit window. If the golden image's capture
   already failed in step 3, the answer is about the grant, not the clone, and
   says so.
7. Evidence entry, naming both images and what each shows; leave both VMs for
   task 7. `format`, `doc-discipline`. Commit with `Rows:` empty.

**Done when** the decisions file answers unknown 4 yes or no, with the golden
and clone captures described.

## Task 7 — Unknown 3: REAPER runs a ReaScript unattended in a clone

**Files:** the decisions file, the evidence file.

1. **Executor**, on the Mac: download REAPER 7.82's macOS Universal disk image
   from `https://www.reaper.fm/download.php`, copy it into `spike-clone`, mount
   it there and copy `REAPER.app` into `/Applications`. Copy the local build
   `build/VST3/Release/poly_plugin.vst3` into the clone's
   `~/Library/Audio/Plug-Ins/VST3/` with `scp -r`, then remove its quarantine
   attribute with `xattr -dr com.apple.quarantine`.
2. **Executor**, over SSH in the clone: record REAPER's own description of its
   arguments. Run `/Applications/REAPER.app/Contents/MacOS/REAPER -h` with a
   30-second timeout, capturing both output streams.
3. **Executor**: write the probe script into the clone as `/tmp/probe.lua`:
   ```lua
   local out = "/tmp/reaper-probe.txt"
   local f = io.open(out, "w")
   reaper.InsertTrackAtIndex(0, false)
   local tr = reaper.GetTrack(0, 0)
   local idx = reaper.TrackFX_AddByName(tr, "VST3i:Poly", false, -1)
   local _, name = reaper.TrackFX_GetFXName(tr, idx, "")
   f:write("fx_index\t" .. tostring(idx) .. "\n")
   f:write("fx_name\t" .. tostring(name) .. "\n")
   f:write("audio_device\t" .. tostring(reaper.GetAudioDeviceInfo("IDENT_OUT", "")) .. "\n")
   f:close()
   reaper.Main_OnCommand(40004, 0) -- File: Quit REAPER
   ```
4. **Executor**, mechanism A, a script on the command line: in the GUI
   session, `open -W -a REAPER --args -nosplash /tmp/probe.lua`, with a
   three-minute timeout; then read `/tmp/reaper-probe.txt` and take a
   `screencapture` if REAPER is still running. Kill REAPER if it is.
5. **Executor**, mechanism B, only if A did not produce the file: copy the
   script to `~/Library/Application Support/REAPER/Scripts/__startup.lua`,
   which REAPER runs on every start, and repeat step 4 without the script
   argument. Remove `__startup.lua` afterwards.
6. Both mechanisms record what stood in the way, if anything: an audio-device
   dialog, the evaluation reminder REAPER shows unlicensed, a first-run
   prompt, Poly not found. Each is described with a capture.
7. **Executor**: the answer goes in the decisions file — **yes, by mechanism A
   or B** only if `fx_index` is 0 or more and REAPER quit by itself. If it
   worked only after a dialog was dismissed by hand, the answer is **no**, and
   names the dialog and whether a licence or a one-time setting in the golden
   image would remove it.
8. Evidence entry quoting `-h`, the probe file and the captures;
   `format`, `doc-discipline`. Commit with `Rows:` empty.

**Done when** the decisions file answers unknown 3 yes or no, naming the
mechanism and anything that blocked it.

## Task 8 — The Mac is returned to its baseline and the answers are judged against the design

**Files:** the decisions file, the evidence file, the ledger; possibly
`M011-install-verify-design.md`.

1. **Executor**: `tart stop spike-clone; tart stop spike-golden; tart delete
   spike-clone; tart delete spike-golden; tart list`. The listing may show the
   cached `macos-tahoe-vanilla` image, which stays (task 1's decision), and no
   `spike-` VM. Delete the scratch captures.
2. **Executor**: in the decisions file, under a `## 2026-10-13 — the four
   answers` section (dated with the day this task runs, if later), set the
   four answers side by side, each with the design section it tests:
   - unknown 1 → "Windows: a Hyper-V VM on JIMW1", the guest paragraph
   - unknown 2 → the same section, "The host side"
   - unknown 3 → "macOS: disposable Tart VMs", "Phases"
   - unknown 4 → the same section, "The golden image"
3. **If every answer is yes:** say so in the decisions file. Then, in the
   ledger, tick all three DoD boxes here and in the slice, set OS45 and the
   slice `done`, and run `jk-standards ledger`.
4. **If any answer is no:** stop. Tell the owner which design section it
   breaks and the options the probe revealed — for unknown 1 the design
   already names them: a Cubase Elements seat of its own, or no Cubase check on
   Windows. Record the owner's choice in the decisions file. Amend the design
   document and M011's slices to match. Only then tick the DoD, and set OS45
   and the slice `done`. S02 and S03 stay unstarted until this commit lands.
5. Evidence entry: the Mac back at baseline, the VMs labelled **throwaway,
   removed**, and a pointer to the answers. `format`, `doc-discipline`,
   `jk-standards ledger`. Commit with `Rows: OS45`.

**Done when** the slice is `done`, OS45 is `done`, and either the design
stands as written or it has been amended with the owner's decision recorded.

## Self-review

- **DoD 1** (four measured answers, with how) — tasks 3, 4, 6, 7, each writing
  one answer and its method into `M011-decisions.md`; task 8 sets them side by
  side.
- **DoD 2** (a design-changing answer is taken to the owner, and the design and
  milestone are amended before S02 or S03) — task 8 step 4.
- **DoD 3** (nothing built is kept; labelled throwaway) — task 5 (guest,
  activation, group membership) and task 8 (Tart VMs); the two things kept,
  Hyper-V and `tart`, are named in task 1's decisions as installed software,
  not anything built for a probe. No script is committed: every one lives in
  this plan.
- **OS45** verification ("each answer records the command run and what it
  returned; (1) records Activation Manager's seat count before and after two
  restores") — task 3 steps 1–2 for the restores, tasks 1, 2 and 5 for the
  seat counts either side, and each probe task's evidence entry for the
  commands and their output.
- **Placeholders** — one deliberate input, the owner's ISO path in task 2,
  which is recorded in the evidence rather than guessed.
- **Names** — `poly-spike-vm`, `clean`, `polytest`, `spike-golden`,
  `spike-clone`, `M011-decisions.md`, `evidence/M011-S01.md`, used the same way
  throughout.
