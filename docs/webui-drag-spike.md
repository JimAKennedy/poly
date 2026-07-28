---
class: archived
---

# WebUI drag-to-DAW spike — finding

> **Archived (2026-07-27)** — this is a frozen spike finding recording the M051 S06
> investigation and its decision. The current delivery mechanism is the WebUI Save-As
> path; drag-to-DAW remains deferred to M067. Update this doc only if a future drag
> spike is run (see the closing section).

**Date:** 2026-07-27
**Slice:** M051 S06 (WebUI MIDI export)
**Verdict:** **Deferred — M067-blocked.** The shipping Save-As path stands alone as the delivery mechanism.

## Question

Can the shipping WebUI (a `choc::ui::WebView` hosting `webui/`) source a real file drag that Cubase — or any DAW — accepts as a MIDI-file drop, matching the drag-to-DAW affordance that Playbeat, HY-RPE2, and Stepic ship?

## Finding: no drag-source seam in choc WebView

`choc::ui::WebView` exposes only:

- `getViewHandle()` — returns the native view (`WKWebView*` on macOS, `HWND` on Windows).
- `bind()` / `evaluateJavascript()` — the JS↔C++ bridge.
- `fetchResource` — the embedded-asset resource callback.

There is **no drag-source hook** and no way to originate an outbound `NSDraggingSession` (macOS) or `DoDragDrop` (Windows) through the choc API. On macOS the `WKWebView` owns and consumes its own `mouseDragged` events, so a WebUI drag gesture never surfaces to native code as a draggable payload.

To source a real drag from inside the WebView we would have to:

1. Reach through `getViewHandle()` to the raw `WKWebView*`.
2. Subclass or swizzle it into an `NSDraggingSource`, intercept the drag gesture, and originate an `NSDraggingSession` carrying the SMF file promise.
3. Do the parallel `IDropSource`/`DoDragDrop` work on Windows.

That is high-effort, fragile across macOS versions (private-ish interception of WKWebView event handling), and success is uncertain against real DAW drop targets.

## Why the native VSTGUI variant is not a shortcut

The **native VSTGUI editor already has working drag-to-DAW** — `ExportControlsView::startDrag()` (in `plugin/source/ui/export_controls_view.cpp`) writes `dragSmfData()` to `std::filesystem::temp_directory_path()/poly-drag-export.mid`, wraps the path in a `VSTGUI::CDropSource(..., IDataPackage::kFilePath)`, and calls `doDrag()`. Cubase accepts that drop.

But this does **not** transfer to the WebUI variant: `doDrag()` is driven by VSTGUI's own mouse-event pipeline on a `CView`. The WebUI variant has no `CView` under the mouse — the choc `WKWebView` is in the way and owns the events. The two UI stacks don't share a drag pipeline.

## Decision

- **Ship:** the native Save-As dialog (`platform_save_dialog_{mac.mm,win.cpp}` + the `exportSaveAs` handler in `web_ui_view.cpp`). It writes a real, Cubase-importable SMF to a user-chosen path. This is the guaranteed S06 deliverable and matches the slice's stated fallback: "if not, drag is documented as M067-blocked and the save path stands alone."
- **Defer:** drag-to-DAW from the WebUI to **M067**. The natural future seam is the note-tick capture timeline (see `51-06-DESIGN.md`): once the frozen SMF is a visible, complete region on the Cloth, dragging from that region is a clearer affordance than a naked-button drag — and M067 is the milestone where the per-scene automation and WebView work is already scheduled to reopen this surface.

## If a future spike is attempted

Start at `choc::ui::WebView::getViewHandle()` (returns `WKWebView*`). Prove, in order:

1. That a native drag gesture can be intercepted before WKWebView consumes it (this is the risky part).
2. That an `NSFilePromiseProvider` / file-promise drag carrying the temp SMF is accepted by Cubase's arrange-window drop target (the native VSTGUI path already proves the payload format Cubase accepts — reuse `IDataPackage::kFilePath` semantics).

Time-box it strictly and update this doc with the outcome regardless of success.
