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

**Root cause:** Windows-specific race condition in choc WebView2 initialization.
On Windows, `CreateCoreWebView2EnvironmentWithOptions` is asynchronous — it
returns immediately and the WebView2 core isn't ready until a callback fires via
the Win32 message loop. `WebUIView::attached()` was calling `addInitScript()`
and `bind("polyHostCall")` immediately after constructing the WebView, before
the async callback could fire. Both calls silently returned `false` because
`coreWebView` was still null. The `polyHostCall` JS function was never
registered, so the JS `send({ type: 'ready' })` call got a console error
(`polyHostCall is not a function`), the C++ bridge never received the ready
signal, `webviewReady_` stayed false, `pushState()` never sent data, and the
UI rendered blank.

On macOS/Linux this works fine because WebKit/GTK initialize synchronously
within the constructor.

A secondary prerequisite issue was also found: the WebView2 Runtime was not
installed on the test runner. This is now resolved (runtime installed, runbook
updated).

**Fix applied:** Moved `addInitScript` and `bind("polyHostCall")` into
`options.webviewIsReady`, which fires synchronously on macOS/Linux (during the
constructor) and asynchronously on Windows (after WebView2 finishes its async
init). Change is in `plugin/source/webui/web_ui_view.cpp` lines 108-119.

**Current status:**
- [x] Root cause identified and fix applied in source
- [x] Build succeeds, all 462 engine tests pass, pluginval 47/47 pass
- [x] User-level VST3 copy updated with fixed binary
- [ ] System-level VST3 copy (`C:\Program Files\Common Files\VST3\`) still has
  the old binary — needs elevated shell to update (see installation note below)
- [ ] Cubase UI verification pending — requires interactive desktop session
- [x] Cubase VST3 cache cleared to force rescan on next launch

**Installation note (for interactive desktop session):**
From an elevated PowerShell:
```powershell
Copy-Item -Force `
  "C:\Users\polyci\dev\poly\build\VST3\Release\poly_plugin.vst3\Contents\x86_64-win\poly_plugin.vst3" `
  "C:\Program Files\Common Files\VST3\poly_plugin.vst3\Contents\x86_64-win\poly_plugin.vst3"
```
Or remove the stale system-level copy so Cubase only loads the user-level one:
```powershell
Remove-Item -Recurse -Force "C:\Program Files\Common Files\VST3\poly_plugin.vst3"
```

**Remaining remediation:**

1. **Fallback diagnostic (deferred code change):** After the `webview_`
   construction, check for null and render a native fallback message telling the
   user to install the WebView2 Runtime. Prevents blank-screen UX if the
   runtime is genuinely missing (separate from the race condition fix).

**Priority:** High — fix applied, pending verification in Cubase.

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
