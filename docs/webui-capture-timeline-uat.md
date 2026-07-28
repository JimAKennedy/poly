---
class: archived
---

# WebUI capture-timeline export — UAT

> **Archived (2026-07-28)** — this is a frozen UAT record for M051 S08: the automated-proof
> snapshot below reflects the checkout it was run against, and the operator checklist is the
> manual DAW leg to tick during a live session. It is a point-in-time verification artifact,
> not a living contract; the current capture-machine behavior lives in the code and its tests.

**Date:** 2026-07-28
**Slice:** M051 S08 (Cloth as export timeline — arm → capture → complete)
**Scope:** WebUI variant only (the macOS/Windows editor). The native VSTGUI export path
(`export_controls_view.*`, `kExportTrigger`) is unchanged and out of scope until M053.

## What this slice ships

MIDI export in the WebUI is gated behind an explicit four-state capture machine driven
from the Cloth header, replacing the S06 `kExportTrigger` edge + 500 ms background prefetch:

- **Processor** — `CaptureState { Idle, Armed, Capturing, Complete }`
  (in `plugin/source/processor.h`). `Armed → Capturing` latches `captureStartPpq_`
  on the next absolute-PPQ bar boundary; `Capturing → Complete`
  freezes the `[captureStartPpq_, captureStartPpq_ + N·ppqPerBar)` window into the
  pre-allocated `exportEvents_` buffer and sets `exportReady_`,
  reusing the existing `RequestMidiExport`/`MidiExportData` reply path.
- **UISnapshot atomics** — `captureState`, `captureBars`, `captureProgressBars`
  (in `plugin/source/ui_snapshot.h`) surface the progression to the UI. No new
  logging surfaces; the Cloth timeline is the receipt.
- **WebUI bridge** — `armCapture`/`resetCapture` actions send `ArmCapture`/`ResetCapture`
  messages (in `web_ui_view.cpp`); `exportSaveAs` opens the Save-As dialog only when
  `captureState == 3` / Complete.
- **Cloth** — bar-anchored capture timeline (X = bars 1..N, Y = lanes, L→R playhead
  during Capturing, note-tick overlay, gold selvage at bar N) plus the header cluster
  (bars picker {4,8,16,32} bound to `kCaptureLength`, Arm/Reset chip, state-driven Export chip).

## Automated proof (this checkout, 2026-07-28)

> **Editorial note (M053 S04, 2026-07-28):** the build commands below originally used a
> `-D` UI-selection flag to pick the WebUI vs native VSTGUI editor on one machine. That
> flag was retired in M053 S04 — the WebUI editor is now the macOS/Windows default and the
> native VSTGUI editor the Linux default (platform-gated in `plugin/CMakeLists.txt`). The
> commands are shown de-flagged; the recorded pass/fail results are unchanged.

| Gate | Command | Result |
|------|---------|--------|
| WebUI build | `cmake -S . -B build && cmake --build build` (macOS default) | pass (0) |
| Native build | native VSTGUI build (now the Linux default; formerly flag-forced) | pass (0) |
| Full ctest | `ctest --test-dir build --output-on-failure` | **450/450 pass** |
| RT safety | `bash scripts/check-realtime-safety.sh` | pass (0) |
| Snippet regions | `bash scripts/check-snippet-regions.sh` | 38 references OK |
| WebUI Playwright | `npm --prefix webui test` | **271 passed** |
| Embedded regen | `build/plugin/generated/poly_webui_assets.h` mtime newer than `webui/ui.js` | regenerated |

The host tests (`tests/host/host_tests.cpp`) assert every transition, Reset from each state,
`exportReady_` false outside Complete, frozen-byte stability after further play, transport
stop-pause (state retained), and seek/loop/tempo-jump cancel-to-Armed. The engine test
`MidiCaptureTests` proves absolute-PPQ window extraction is byte-stable after later `push()`es.
The Playwright suite (`webui/tests/capture-timeline.spec.mjs`) covers the chip states and the
Cloth timeline render across the four capture states.

## Live Cubase round-trip (operator UAT)

The arm → capture → complete → Export → Save → re-import round-trip is a DAW-only signal that
cannot be exercised headlessly (same live-only limitation as S06 — see `webui-drag-spike.md`).
Automated proof above covers every seam up to the OS Save-As dialog and the frozen SMF bytes;
the steps below are the operator checklist confirming the DAW leg end to end.

Protocol (Cubase, macOS AU/VST3 host):

1. Load Poly (WebUI build), open the Cloth tab. Header shows the bars picker (default 8),
   an **Arm** chip, and a dim **Export** chip.
2. Set a bar length via the picker (e.g. 8). Confirm it binds to `kCaptureLength`.
3. Start transport, click **Arm**. Chip flips to **Reset**; Cloth enters timeline mode
   (bars 1..N ruler, lanes as rows). State → Armed.
4. On the next bar boundary the playhead begins sweeping L→R; note ticks accrue over the
   emitted notes. State → Capturing.
5. **Stop transport mid-capture** → playhead pauses, state and progress retained.
   Resume → capture continues from where it paused.
6. **Seek / loop-jump mid-capture** → capture cancels back to Armed (playhead resets);
   re-latches on the next bar boundary.
7. At bar N the playhead reaches the selvage; state → Complete, gold selvage marks bar N,
   and the **Export** chip becomes active.
8. Click **Export** → Save-As dialog opens (only reachable in Complete). Save the `.mid`.
9. Continue playback for several bars, then re-open the saved file / drag it into a Cubase
   MIDI track: the imported notes match the frozen bar-1..N window exactly, unchanged by the
   continued playback (frozen-window stability).
10. Click **Reset** at any point → returns to Idle, Cloth restores its idle annotations,
    Export chip dims.

**Operator sign-off:**

| # | Check | Status |
|---|-------|--------|
| 1 | Header cluster renders (bars picker / Arm / Export) | ☐ |
| 2 | Bars picker binds `kCaptureLength` | ☐ |
| 3 | Arm → Armed; chip flips to Reset | ☐ |
| 4 | Playhead sweeps L→R; note ticks overlay emitted notes | ☐ |
| 5 | Stop mid-capture pauses (state retained), resume continues | ☐ |
| 6 | Seek/loop/tempo jump cancels to Armed | ☐ |
| 7 | Complete: selvage at bar N, Export chip active | ☐ |
| 8 | Export active only in Complete; Save-As writes `.mid` | ☐ |
| 9 | Re-import matches frozen window; stable after continued play | ☐ |
| 10 | Reset → Idle; annotations restored | ☐ |

> Live-DAW status: **pending operator confirmation.** This checkout is verified through the
> full automated proof above; the boxes above are the manual DAW leg for the operator to
> tick during the live session, consistent with the S06 live-only convention.
