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
   (`choc_DesktopWindow.h:806-809`), but Poly's reparenting code was missing it.

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
- Changes in `plugin/source/webui/web_ui_view.cpp` lines 108-148.

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
in the controller. The 43 factory presets are internal-only, applied via the web
UI bridge (`applyPreset` action in `web_ui_view.cpp` lines 429-544).

**Current impact:** This is **by design** — presets are accessible from Poly's
own web UI preset picker. However, it's undocumented and may confuse Cubase
users who expect the standard preset workflow.

**Remediation options (choose one, or defer):**

1. **Document it:** Add a note to `docs/cubase-workflow.md` explaining that
   presets are accessed from Poly's internal UI, not the DAW's preset browser.

2. **Expose presets via ProgramList (feature work):** Implement `ProgramList`
   in `PolyControllerBase::initialize()` to register the 43 factory presets
   with the DAW. This would make presets appear in Cubase's browser and enable
   DAW-level preset recall (e.g. MIDI program change). Non-trivial — requires
   wiring `programChange` into the existing `applyPreset` path and testing
   state round-trip.

**Priority:** Low — cosmetic/discoverability issue, not a blocker. The web UI
preset picker works once ISSUE-001 is resolved.

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
