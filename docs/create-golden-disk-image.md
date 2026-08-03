---
class: gated
---

# Golden Windows Image with Sysprep + DISM

Step-by-step guide for creating a deployable Windows golden image using the
built-in Sysprep and DISM tools.

## Prerequisites

- A **reference machine** (physical or VM) with a clean Windows install and all
  desired software/config applied
- A **bootable Windows PE (WinPE) USB** or a second partition/drive to capture
  the image to
- The **Windows ADK** (Assessment and Deployment Kit) installed on the reference
  machine or a technician workstation — you need the "Deployment Tools" and
  "Windows PE add-on" components

## Phase 1: Prepare the Reference Machine

1. **Install Windows** cleanly on the reference machine (or VM). Use the edition
   you want in your golden image.

2. **Install all Windows Updates.** Reboot and repeat until no more updates are
   offered.

3. **Install your software.** Everything you want baked into the image — drivers,
   runtimes, apps, tools. For a CI runner this likely includes:
   - Visual Studio / Build Tools
   - Git
   - CMake
   - Python
   - Node.js
   - Any other dev tooling

4. **Configure system-level settings.** These survive Sysprep:
   - Power plan (set to "High Performance" if this is a build runner)
   - Windows Defender exclusions (e.g. build directories)
   - Environment variables (system-level, not user-level)
   - Registry tweaks (e.g. disabling Windows Update auto-reboot)
   - Firewall rules

5. **Do NOT configure user-specific settings** — Sysprep deletes the current
   user profile when generalizing. Per-user config needs to go into an unattend
   answer file (Phase 2) or a first-login script.

6. **Clean up** to shrink the image:

   ```powershell
   # Clear Windows Update cache
   Dism /Online /Cleanup-Image /StartComponentCleanup /ResetBase

   # Clear temp files
   Remove-Item -Recurse -Force $env:TEMP\*
   Remove-Item -Recurse -Force C:\Windows\Temp\*

   # Clear event logs
   wevtutil el | ForEach-Object { wevtutil cl $_ }

   # Defrag (for physical disks / thick-provisioned VMs)
   Optimize-Volume -DriveLetter C -Defrag
   ```

## Phase 2: Create an Answer File (Optional but Recommended)

An unattend.xml file automates OOBE so you don't have to click through setup on
every deployment. Save this as `C:\Windows\System32\Sysprep\unattend.xml`:

```xml
<?xml version="1.0" encoding="utf-8"?>
<unattend xmlns="urn:schemas-microsoft-com:unattend">
  <settings pass="oobeSystem">
    <component name="Microsoft-Windows-International-Core"
               processorArchitecture="amd64"
               publicKeyToken="31bf3856ad364e35"
               language="neutral" versionScope="nonSxS">
      <InputLocale>en-US</InputLocale>
      <SystemLocale>en-US</SystemLocale>
      <UILanguage>en-US</UILanguage>
      <UserLocale>en-US</UserLocale>
    </component>
    <component name="Microsoft-Windows-Shell-Setup"
               processorArchitecture="amd64"
               publicKeyToken="31bf3856ad364e35"
               language="neutral" versionScope="nonSxS">
      <OOBE>
        <HideEULAPage>true</HideEULAPage>
        <HideLocalAccountScreen>true</HideLocalAccountScreen>
        <HideOEMRegistrationScreen>true</HideOEMRegistrationScreen>
        <HideOnlineAccountScreens>true</HideOnlineAccountScreens>
        <HideWirelessSetupInOOBE>true</HideWirelessSetupInOOBE>
        <ProtectYourPC>3</ProtectYourPC>
      </OOBE>
      <UserAccounts>
        <LocalAccounts>
          <LocalAccount wcm:action="add">
            <Name>polyci</Name>
            <Group>Administrators</Group>
            <Password>
              <Value>CHANGE_ME</Value>
              <PlainText>true</PlainText>
            </Password>
          </LocalAccount>
        </LocalAccounts>
      </UserAccounts>
      <AutoLogon>
        <Enabled>true</Enabled>
        <Username>polyci</Username>
        <Password>
          <Value>CHANGE_ME</Value>
          <PlainText>true</PlainText>
        </Password>
        <LogonCount>1</LogonCount>
      </AutoLogon>
      <TimeZone>UTC</TimeZone>
    </component>
  </settings>
</unattend>
```

**Change `CHANGE_ME`** to the desired password. Adjust locale, timezone, and
username as needed.

## Phase 3: Run Sysprep

1. Open an **elevated Command Prompt** on the reference machine.

2. Run Sysprep:

   ```cmd
   C:\Windows\System32\Sysprep\sysprep.exe /generalize /oobe /shutdown /unattend:C:\Windows\System32\Sysprep\unattend.xml
   ```

   Flags:
   - `/generalize` — strips hardware-specific drivers and SIDs so the image
     works on different hardware
   - `/oobe` — boots into Out-of-Box Experience on next start (the answer file
     automates this)
   - `/shutdown` — shuts down when done (don't boot the machine again before
     capturing!)
   - `/unattend:...` — points to the answer file

3. **Wait for shutdown.** The machine will generalize itself and power off.
   **Do not boot it again** — the next boot will re-specialize the image for
   whatever hardware it's on.

## Phase 4: Capture the Image

Boot from WinPE (not the installed Windows) to capture the disk.

1. **Boot from WinPE USB** (or mount the VM disk to a technician machine).

2. **Identify the Windows partition:**

   ```cmd
   diskpart
   list volume
   exit
   ```

   Note the drive letter of the Windows partition (usually `D:` in WinPE since
   `X:` is the RAM disk).

3. **Capture the image with DISM:**

   ```cmd
   Dism /Capture-Image /ImageFile:E:\golden-image.wim /CaptureDir:D:\ /Name:"Poly CI Runner v1" /Description:"Windows 11 Pro + VS2022 + dev tools" /Compress:maximum
   ```

   - `/ImageFile:E:\golden-image.wim` — where to save (use a USB drive or
     network share with enough space)
   - `/CaptureDir:D:\` — the Windows partition to capture
   - `/Compress:maximum` — best compression; use `fast` if time matters more
     than size

4. The capture takes 15–60 minutes depending on image size and disk speed. The
   resulting `.wim` file is the golden image.

## Phase 5: Deploy the Image to a New Machine

1. **Boot the target machine from WinPE.**

2. **Partition the disk:**

   ```cmd
   diskpart
   select disk 0
   clean
   convert gpt
   create partition efi size=260
   format fs=fat32 quick label="EFI"
   assign letter=S
   create partition msr size=16
   create partition primary
   format fs=ntfs quick label="Windows"
   assign letter=W
   exit
   ```

3. **Apply the image:**

   ```cmd
   Dism /Apply-Image /ImageFile:E:\golden-image.wim /Index:1 /ApplyDir:W:\
   ```

4. **Set up the boot loader:**

   ```cmd
   bcdboot W:\Windows /s S: /f UEFI
   ```

5. **Reboot.** Remove the WinPE USB. The machine will boot into OOBE (automated
   by the answer file) and be ready.

## Tips

- **Version your images.** Name them `golden-image-v1.wim`,
  `golden-image-v2.wim`, etc. Keep a changelog of what's in each.
- **For VMs**, you can skip WinPE entirely — snapshot the VM after Sysprep
  shutdown, then clone/template it. VMware, Hyper-V, and cloud providers all
  support this directly.
- **Test the image** on a throwaway machine or VM before rolling it out widely.
- **If Sysprep fails**, check `C:\Windows\System32\Sysprep\Panther\setuperr.log`
  — common causes are AppX packages that block generalization (especially
  pre-installed Microsoft Store apps).

## Quick Reference

```
1. Build reference machine with all software
2. Clean up temp files and update cache
3. Place unattend.xml in C:\Windows\System32\Sysprep\
4. sysprep /generalize /oobe /shutdown /unattend:...\unattend.xml
5. Boot WinPE, capture with DISM /Capture-Image
6. Deploy with DISM /Apply-Image + bcdboot
```
