# M008 — decisions

Append-only. Every question `/jk:auto` asked before running, every answer, and
every choice taken on the owner's behalf.

## 2026-10-06 — planning M008/S01

Why this run is S01 alone: the owner asked for `/jk:auto M008/S01`, the half
of M008 that needs no Azure account. S02 (Azure Artifact Signing) is a
**deferred** boundary, below.

Measured before asking, on `main` at `d2085fb`. GitHub's `windows-2022` runner
image lists WiX Toolset 3.14.1, Inno Setup 6.7.1, NSIS 3.10, .NET SDKs 8–10
and Windows SDKs up to 10.0.26100 (so `signtool`). WiX v4 and later ship as a
.NET tool (`wix` on NuGet); from v6 (April 2025) the project's licence adds an
Open Source Maintenance Fee for organisations above US$10,000 annual revenue,
while v5 (latest 5.0.2) predates it. CI's `build` job uploads the Windows
bundle as `plugin-windows-2022`. The plugin factory names the vendor
`jk.digital`. WiX builds MSIs only on Windows, so nothing here can be built on
the owner's Mac.

- **Q:** Which installer format? — **A:** MSI via WiX v5.
- **Decision and the alternatives (OS37's record).** The ledger asks for the
  format "chosen on which one signs and uninstalls cleanly in CI":
  - **MSI (WiX v5) — chosen.** One `.msi`, which `signtool` signs in one
    step; there is no separate uninstaller binary to sign. Windows Installer
    records every file it installs, so Apps & features' Uninstall removes
    exactly that and nothing else. `msiexec /i … /qn` and `/x … /qn` install
    and remove silently, which is what a CI test needs. Pinned at 5.0.2 as a
    .NET tool, free of v6's fee.
  - **Inno Setup — declined.** A single `setup.exe`, common with plugin
    vendors and preinstalled, but its uninstaller (`unins000.exe`) is written
    at install time and is signed only if the signing service runs inside the
    compiler — not one `signtool` step after the build, which is OS37's
    requirement.
  - **NSIS — declined.** Preinstalled, but the uninstaller is hand-written
    logic, so a clean removal is a property of our script rather than of the
    platform, and signing the uninstaller needs a two-pass build.
  - **WiX 3 (preinstalled) — declined.** The legacy line; v5 is the
    supported, fee-free one.
- **Q:** How is it proved in CI with no certificate yet? — **A:** the chosen
  format plus a self-signed signature.
- **Decision:** a `package-windows` CI job builds the MSI, signs it with a
  throwaway self-signed certificate it trusts on the runner (so
  `signtool verify /pa` passes, proving the one-step sign S02 will use),
  installs it silently, checks the bundle is in
  `C:\Program Files\Common Files\VST3` and that Apps & features lists Poly,
  uninstalls, and checks both are gone. Inno and NSIS are recorded as
  assessed, not built.
- **Q:** MSI builds only on Windows; how does the work get run? — **A:** a
  draft PR during the slice.
- **Decision:** the branch is pushed and a draft PR opened once task 2 adds
  the CI job, so every push runs `package-windows`; WiX mistakes are fixed as
  ordinary task commits. `/jk:ship` marks the PR ready.

### Taken on the owner's behalf

- **UpgradeCode `CAC811BE-A55D-452F-A377-9442360217CD`**, generated once on
  2026-10-06 and never changed: it is how every future Poly MSI recognises an
  older one to upgrade.
- **`MajorUpgrade`**, so installing a newer Poly replaces the older one, and
  a downgrade is refused with a message.
- **Per-machine only**, to `C:\Program Files\Common Files\VST3`, the folder
  the VST3 specification names; the DoD asks for nothing per-user on Windows.
- **Manufacturer `jk.digital`**, as the plugin factory already says; product
  name "Poly".
- **No installer dialogs in S01** beyond Windows Installer's own progress:
  the smallest thing that installs, as M007/S01 did on macOS.
- **The release leg builds and install-tests the MSI on its own runner,
  unsigned** — the self-signed signature is only the CI proof; S02 replaces
  it with Azure Artifact Signing.
- **The README and guide are untouched;** describing the installers is
  M009's (OS40).
- **The install test refuses to run where Poly is already installed**, as the
  macOS one does, so it can never remove a real copy.

### Deferred

- **M008/S02 — signing through Azure Artifact Signing**, waiting at the
  S01→S02 boundary on the Azure account and its identity validation. A
  planned pause, not a failure.
