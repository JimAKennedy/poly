# M007 — decisions

Append-only. Every question `/jk:auto` asked before running, every answer, and
every choice taken on the owner's behalf.

## 2026-10-06 — planning M007/S01

Why this run is S01 alone: the owner parked open-source-launch on
2026-09-30 while sorting out Apple and Windows signing, and asked on
2026-10-06 for `/jk:auto M007/S01` — the half of M007 that needs no
purchase. S02 (sign, notarize, staple) waits on the Apple Developer Program
enrolment and both Developer ID certificates; it is a **deferred** boundary,
below.

Measured before asking, on `main` at `19f81d6`. `release.yml` zips the
universal `.vst3` on its macOS leg and publishes zips, `SHA256SUMS` and a
provenance attestation; `scripts/check-release-workflow.mjs` locks step order
by step name. CI's `build` job uploads the arm64 bundle as `plugin-macos-14`
and `pluginval-macos` consumes it. M003/S03 declined Logic for the first
release, so no AU ships and the package carries the VST3 alone. The AU target
uses the bundle domain `digital.jk.poly`. The README already names both VST3
folders (`~/Library/…` per-user, `/Library/…` all users).

- **Q:** macOS packages have no uninstaller; what does "removing Poly" mean?
  — **A:** delete the bundle.
- **Decision:** removal is deleting `poly_plugin.vst3` from the VST3 folder;
  the test removes exactly what `pkgutil --files` lists for the package and
  forgets its receipt, then checks the folder matches its before-state. No
  uninstall script ships — **Why:** the owner's call; a script would be one
  more artifact for S02 to sign and notarize.
- **Q:** how is "installs and removes cleanly on a clean Mac" proved? —
  **A:** a CI job on every PR.
- **Decision:** a `package-macos` CI job builds the `.pkg` from the PR's
  plugin and installs it on the fresh hosted runner, system-wide and
  per-user, checking where the bundle lands, then removes it and checks the
  folders are back as they were. Locally the package's payload is inspected
  without installing. This milestone's PR run is the evidence, recorded at
  `/jk:ship` as M006's was.
- **Q:** the per-user option's form? — **A:** the installer's own choice.
- **Decision:** the distribution enables both the local-system and the
  current-user-home domains, so the installer offers "all users" or "only
  me", defaulting to all users.

### Taken on the owner's behalf

- **Package identifier `digital.jk.poly.vst3`**, in the AU's bundle domain;
  install location `/Library/Audio/Plug-Ins/VST3`, which the current-user
  domain maps to `~/Library/Audio/Plug-Ins/VST3`.
- **The bundle is not relocatable.** `pkgbuild`'s component property list
  sets `BundleIsRelocatable` false, so the installer never "updates" a copy
  of `poly_plugin.vst3` it finds elsewhere on disk instead of installing to
  the VST3 folder.
- **Asset name** `poly-<tag>-macos-universal.pkg`, beside the zip, included in
  `SHA256SUMS` and the provenance attestation like every zip.
- **The release leg tests its own package** on its fresh runner before
  upload, with the same script the PR job uses, so every release proves the
  package it publishes.
- **No installer pages beyond the domain choice** — no welcome, licence or
  readme pages in S01; the package is the smallest thing that installs.
- **The README and guide are not touched.** Describing the installers is
  M009/S01's row (OS40); S01's removal procedure is proved by the test and
  recorded for M009 to document.
- **Version** — the release passes the tag's version; CI passes the version
  in `CMakeLists.txt`'s `project()`.

### Deferred

- **M007/S02 — signing, notarizing and stapling the package**, waiting at the
  S01→S02 boundary on the Apple Developer Program enrolment, the Developer
  ID Application and Developer ID Installer certificates, and the seven
  repository secrets. A planned pause, not a failure.
