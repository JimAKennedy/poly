---
class: gated
---

# Windows Test-Runner Setup — Known Issues & Remediation Backlog

> **Purpose:** Captures issues discovered during hands-on Windows test-runner
> bring-up that need remediation work planned in the main dev environment.
> Each item includes root cause, current impact, and a recommended fix.
> Companion to `docs/windows-test-runner-setup.md`.

---

## ISSUE-001: Blank plugin window on Windows — WebView2 async init race

**Discovered:** 2026-08-03, during first Cubase load of Poly on the Windows
test runner.

**Symptom:** Poly loads in Cubase but the editor window is completely blank
(white/empty). No presets are visible because the preset picker lives inside the
web UI, which never renders. WebView2 processes are running (the runtime is
installed and HTML loads), but the UI has no data.

**Root cause (two bugs, both required to fix):**

1. **Async init race condition:** On Windows,
   `CreateCoreWebView2EnvironmentWithOptions` is asynchronous — it returns
   immediately and the WebView2 core isn't ready until a callback fires via the
   Win32 message loop. `WebUIView::attached()` was calling `addInitScript()` and
   `bind("polyHostCall")` immediately after constructing the WebView, before the
   async callback could fire. Both calls silently returned `false` because
   `coreWebView` was still null. The `polyHostCall` JS function was never
   registered, so the JS `send({ type: 'ready' })` call failed, the C++ bridge
   never received the ready signal, `webviewReady_` stayed false,
   `pushState()` never sent data, and the UI rendered blank.

2. **Missing WS_POPUP → WS_CHILD style change:** choc creates its wrapper HWND
   with `WS_POPUP` style. When reparenting into the DAW's plugin window via
   `SetParent`, the style must be changed to `WS_CHILD` — otherwise the window
   doesn't clip to the parent bounds and can fail to render content inside the
   host window. choc's own `DesktopWindow::setContent` does this
   (`choc_DesktopWindow.h:806-809` [file-line-ok]: pinned into the vendored
   choc header at the version Poly fetches), but Poly's reparenting code was
   missing it.

On macOS/Linux these don't apply: WebKit/GTK init synchronously, and NSView
reparenting works without style changes.

A prerequisite issue was also found: the WebView2 Runtime was not installed on
the test runner. This is resolved (runtime installed, runbook updated).

**Fix applied:**
- Moved `addInitScript` and `bind("polyHostCall")` into
  `options.webviewIsReady` callback (fires synchronously on macOS/Linux,
  asynchronously on Windows after WebView2 finishes init).
- Added `WS_POPUP → WS_CHILD` style change via `SetWindowLongPtr` before
  `SetParent`, plus explicit `ShowWindow(child, SW_SHOW)`.
- Changes in `WebUIView::attached()` / the Windows reparenting path of
  `plugin/source/webui/web_ui_view.cpp`.

**Current status:**
- [x] Root cause identified — two bugs, both fixed
- [x] Build succeeds, all 462 engine tests pass, pluginval 47/47 pass
- [x] Verified working in Cubase 14 on the Windows test runner
- [x] System-level stale VST3 copy removed from `C:\Program Files\Common Files\VST3\`
- [x] Cubase VST3 cache cleared, rescanned, loads from user-level path

**Remaining remediation:**

1. **Fallback diagnostic (deferred code change):** After the `webview_`
   construction, check for null and render a native fallback message telling the
   user to install the WebView2 Runtime. Prevents blank-screen UX if the
   runtime is genuinely missing (separate from the race condition fix).

**Priority:** Resolved — verified working in Cubase.

---

## ISSUE-002: VST3 preset browser shows no presets (by design, but undocumented)

**Discovered:** 2026-08-03, alongside ISSUE-001.

**Symptom:** Cubase's standard VST3 preset browser (the dropdown at the top of
the plugin header strip) shows no presets for Poly.

**Root cause:** Poly does not implement the VST3 `ProgramList` API. There are
no `getProgramListCount`, `getProgramName`, or `UnitInfo` with program list IDs
in the controller. The factory presets <!-- counts-ok: incidental reference to the internal preset inventory, not a count this doc owns --> are internal-only, applied via the web
UI bridge (the `applyPreset` action in `web_ui_view.cpp`'s `handleAction`).

**Current impact:** This is **by design** — presets are accessible from Poly's
own web UI preset picker. However, it's undocumented and may confuse Cubase
users who expect the standard preset workflow.

**Remediation options (choose one, or defer):**

1. **Document it:** Add a note to `docs/cubase-workflow.md` explaining that
   presets are accessed from Poly's internal UI, not the DAW's preset browser.

2. **Expose presets via ProgramList (feature work):** Implement `ProgramList`
   in `PolyControllerBase::initialize()` to register the factory presets <!-- counts-ok: incidental reference to the internal preset inventory, not a count this doc owns -->
   with the DAW. This would make presets appear in Cubase's browser and enable
   DAW-level preset recall (e.g. MIDI program change). Non-trivial — requires
   wiring `programChange` into the existing `applyPreset` path and testing
   state round-trip.

**Priority:** Low — cosmetic/discoverability issue, not a blocker. The web UI
preset picker works once ISSUE-001 is resolved.

---

## ISSUE-003: Cubase nightly fails on `pwsh: command not found`

**Discovered:** 2026-08-04, first `workflow_dispatch` of the Cubase nightly (L4)
on the self-hosted runner `JIMW1`.

**Symptom:** The run reaches the runner and checks out the repo, then fails in
~13 s. The first `run:` step (`Prepare artifact staging dir`) errors with
`pwsh: command not found`, and every later `pwsh` step (`Quit Cubase`, `Archive
logs`) fails the same way. The run never configures, builds, or launches Cubase.
The uploaded artifact set is empty (`_artifacts` was never created, because its
creation step is the one that first hit the missing shell).

**Root cause:** `.github/workflows/cubase-nightly.yml` sets `shell: pwsh` on
every `run:` step, and all `scripts/cubase/*.ps1` are PowerShell **Core** (7+)
scripts. A fresh self-hosted Windows box ships only **Windows PowerShell 5.1**
(`powershell.exe`) — it has no `pwsh`. GitHub-hosted `windows-2022` images
pre-install `pwsh`, which is why the same workflow would pass there; a
self-hosted runner is responsible for its own toolchain. Checkout succeeded
because it runs on Node, not `pwsh`, so the run *looked* like it started before
dying on the first script step.

**Fix applied:**
- Documented `pwsh` (PowerShell 7) as a **required** Part 2 toolchain install in
  `docs/windows-test-runner-setup.md`
  (`winget install --id Microsoft.PowerShell`), with the caveat that the runner
  logon task must be **restarted/rebooted** afterward so the runner process
  picks up the new PATH.
- Added `pwsh --version` to Part 2's verify block (must print 7.x, not 5.1).

**Current status:**
- [x] Root cause identified (missing PowerShell Core on the runner)
- [x] Install + restart step documented in the setup runbook (Part 2)
- [ ] `pwsh` installed on `JIMW1` and the runner restarted (owner action)
- [ ] Nightly re-dispatched; run gets past the shell step

**Remediation options (belt-and-braces, optional):**

1. **Preflight shell check (recommended follow-up):** Add an early workflow step
   that verifies `pwsh` is present and fails with an explicit, self-explaining
   message (e.g. "PowerShell 7 not installed — see runbook Part 2") instead of
   the cryptic `command not found` cascade. Makes the next missing-dependency
   failure diagnosable at a glance.
2. **Fall back to Windows PowerShell 5.1** (`shell: powershell`): avoids the
   runner install, but the `.ps1` scripts would need verification against 5.1
   (it lacks `??`, `ForEach-Object -Parallel`, and some 7-only cmdlets). Not
   recommended — it diverges from CI's `pwsh` environment.

**Priority:** Medium — hard-blocks every nightly run until `pwsh` is installed,
but the fix is a one-time install with no code change.

---

## ISSUE-004: `archive-logs.ps1` self-copy crash on `probe.jsonl`

**Discovered:** 2026-08-04, first full Cubase launch/quit cycle in run
30865168727 on `JIMW1`.

**Symptom:** The `archive-logs` step fails with `Cannot overwrite the item
...\probe.jsonl with itself.` The Cubase launch/quit cycle itself completes
(Cubase launches, wait-ready succeeds, quit hard-kills after graceful timeout),
but the archive step crashes and marks the run as failed.

**Root cause:** `POLY_PROBE_OUTPUT` is set to
`${{ github.workspace }}\_artifacts\probe.jsonl`, which is **inside**
`POLY_ARTIFACT_DIR` (`${{ github.workspace }}\_artifacts`). When the probe
creates that file (even an empty one), `archive-logs.ps1`'s `Copy-IfPresent`
tries to copy it from `_artifacts/probe.jsonl` into `_artifacts/probe.jsonl` —
the same path. PowerShell's `Copy-Item` rejects copying a file onto itself.

**Fix applied:** Added a same-path guard to `Copy-IfPresent` in
`scripts/cubase/archive-logs.ps1` — resolves both source and destination to
full paths and skips the copy when they match.

**Current status:**
- [x] Root cause identified (source path is inside artifact dir)
- [x] Fix applied in `archive-logs.ps1`
- [ ] Fix verified in a nightly run

**Priority:** Medium — fails every nightly run that reaches the archive step.

---

## Template for new issues

```
## ISSUE-NNN: <short title>

**Discovered:** <date>, <context>

**Symptom:** <what the user sees>

**Root cause:** <why it happens>

**Current impact:** <severity and scope>

**Remediation:**
<numbered options with effort estimates>

**Priority:** <High / Medium / Low> — <one-line justification>
```
