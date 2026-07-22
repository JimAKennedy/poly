---
class: archived
---

# Web UI Migration — Status & Remaining Work

> **Archived (2026-07-22)** — this document is a frozen snapshot from the mid-migration
> `webui-integration` branch. The migration itself is long complete. For current WebUI
> state see [engine-spec.md](engine-spec.md), the WebUI section of
> [appendix-plugin-architecture.mdx](../site/src/content/docs/appendix-plugin-architecture.mdx),
> and the `webui/` source directory. Do not treat any "remaining work" claim in this
> file as still open.

Status as of `webui-integration` branch (2026-07-04). Audience: roadmap/planning session.
This is the handoff document for completing the migration of the Cubase
plugin from the VSTGUI editor to the web UI (`webui/` bundle in a native
webview). Background contracts live in `webui/bridge-schema.md` and
`webui/host-iface.js`; design decisions in the internal design-language doc
(desk/cloth modes, Console-hero hierarchy, learn layer).

## 1. Where things stand

Everything below is already merged into main:

| Landed | What it gives the migration |
|---|---|
| M027 (#41) | Engine/state safety the bridge relies on: `sanitize` on every deserialize, aksak zero-divisor guards, stop-flush, morph field completeness, `sceneState_` race elimination (atomic handshakes), noteMap-before-capture export fix |
| M028 (#42) | Engine hardening: O(1) scene chain (no unbounded audio-thread loop), dynamic timing bounds, seed-0 fix |
| M029 (#43) | **The keystone for the bridge**: RT-safe atomic message paths for cellSizes, timeline patterns, micro-timing, and envelopes, all routed through active-scene selection (Scene B now editable). The web bridge's `action` handlers can reuse these paths as-is |
| #44 | `webui/` bundle (host-agnostic renderer + mock host + Playwright suite) and the native shell scaffold `plugin/source/webui/` behind `-DPOLY_WEB_UI=OFF` |
| branch | W1 native shell spike in progress; Playwright suite expanded to 36 tests (interaction, resilience, DOM stability); CI `webui-tests` job active |

What exists in `webui/` today: desk (channel strips with editable step
ladders, phase dials, VU), cloth (full-bleed interference weave, band-click
navigation), strip expansion with live Pattern (Euclid steppers, timeline,
additive cells) / Timing (per-step ±20 ms drag) / Envelopes panes, learn
layer, mock-host WebAudio preview. All state flows through the
`PolyHost` interface, so the plugin bridge slots in without UI changes.

**Not yet true:** the plugin cannot show this UI. The native shell is a
scaffold — 6 `TODO(spike)` markers in `plugin/source/webui/web_ui_view.cpp`
— and CI does not run the webui Playwright suite.

## 2. Remaining work

### W1 — Native shell spike completion (the go/no-go gate)

Complete `web_ui_view.cpp` on a machine with the SDK + Cubase:

1. Reparent the choc webview child into the host's NSView/HWND; wire
   `onSize` and scale-factor handling.
2. Pin the choc `GIT_TAG` to a commit SHA (currently `main`).
3. JSON parse (`choc::json`) in `handleHostCall`; dispatch `edit` →
   `beginEdit`/`performEdit`/`endEdit` via `bridge_params.h`; dispatch
   `action` → **M029's existing pending-struct message paths**.
4. Initial `state` push on `ready` + after every action/preset/scene change.
5. ~30 Hz `frame` push (choc timer, message thread) sourced from the same
   values the read-only feedback params carry (lane phase, step, transport,
   convergence).
6. Host validation matrix, timeboxed: Cubase macOS → Cubase Windows
   (WebView2 runtime present?) → Live/Reaper smoke. Test: keyboard focus,
   HiDPI, resize, automation write, CPU with cloth animating.

Exit gate: either the webview build is viable in Cubase (proceed to W2+) or
documented blockers → fall back to VSTGUI permanently and keep `webui/` for
the docs-site playground only. Everything after W1 assumes "pass".

### W2 — Bridge completeness

- **State serializer (C++)**: selected scene's `GrooveState` → the State
  JSON shape. Note the web model is currently a simplification — real
  `Envelope` has shape/curvature/phaseOffset/stepList; lanes have
  subdivision (not `stepLen`), `microTimingMs[64]`, accent masks,
  mutation/drift/phrase/kotekan/tempoMultiplier/midiChannel. Serialize the
  full model; the UI ignores what it doesn't render yet (schema is built
  for this).
- **Param table**: extend `bridge_params.h` to expression params
  (velocity, probability, spread, ghost, push, humanize, swing, duration,
  active) and globals (activeLaneCount, seed, scene select/morph, chain,
  capture length).
- **Model alignment in `webui/`**: replace the mock's simplified fields
  with the real schema (envelope model, subdivision, per-step arrays);
  compute convergence from actual cycle lengths instead of the hardcoded
  120-eighth constant (and surface the 4/4 assumption it inherits).

### W3 — UI parity (web surfaces that don't exist yet)

Current native features with no web equivalent, roughly in value order:

1. Interactive macros (web shows them read-only) + global controls
   (active lane count, seed with re-roll, tempo display from host).
2. Per-lane expression editing (vel/prob/spread/ghost/push/humanize/swing/
   duration/active) — strip "feel" rows become controls.
3. Preset browser (14 factory presets + Init) and lane rename.
4. Scene bar: A/B copy, morph slider, chain editor (entries, bars, mode).
5. Export controls: capture length, export trigger, save flow; capture
   status.
6. Note-map editor (128-slot remap grid).
7. Accent-mask editor; mutation/drift/phrase/kotekan/tempoMultiplier/
   midiChannel controls (strip expansion has room — add panes or extend
   Pattern/Timing).
8. Morph-mode editing semantics in the UI (native routes edits to A while
   morphing — mirror or improve).

### W4 — Export drag-out

The mockup's "drag cloth to DAW" needs a native drag source: `action:
exportRequest` already reaches the existing atomic export path; add
platform drag initiation (NSDraggingSource / DoDragDrop) handing the host
the temp `.mid`. Follow-up (roadmap item, not migration-blocking):
per-lane multitrack SMF.

### W5 — Testing & CI

- ~~Add a `webui` job to `ci.yml`~~ ✅ Done — `webui-tests` job runs
  `npm ci && npx playwright test` in `webui/` on every push/PR. 36 tests
  covering interaction dispatch, resilience under 60 Hz state pushes, and
  DOM stability.
- Screenshot scene set as CI artifacts (cloth / desk / learn /
  expanded × pattern·timing·env) — becomes the site screenshot pipeline
  and PR-review strips.
- Visual regression via Playwright `toHaveScreenshot` once the design
  stabilizes.

### W6 — Switchover & retirement

Only after W1–W3 hold up in Cubase daily use:

- Default `POLY_WEB_UI=ON`; one release with an escape hatch, then delete:
  VSTGUI views (~20 files), `poly.uidesc`, `vstgui_support` link, the
  native visual/interaction test harness (superseded by Playwright), and
  update the RT-safety scanner scope + NFR suppressions that reference
  VSTGUI patterns.
- Window size/scale persistence in controller state; lens preference
  (cloth vs desk default) per the design language.
- Windows: WebView2 runtime detection + friendly failure; Linux: decide
  webkit2gtk support or document web-UI-off for Linux builds.
- Docs: parameter-reference and guide screenshots regenerate from the
  Playwright pipeline; site playground swaps `mock-host.js` for the
  WASM-engine host when the (separately planned) Emscripten build lands —
  deleting the duplicated pattern math in `groove-math.js` in favor of
  engine truth.

## 3. Sequencing & dependencies

```
W1 spike ──gate──► W2 bridge ──► W3 parity ──► W6 switchover
   │                   │
   └───────────────► W5 CI (webui job can land immediately, pre-W1)
                       W4 drag-out (after W2, parallel with W3)
```

The W5 CI job has no dependencies — it protects `webui/` from day one and
is the cheapest next commit.

## 4. Rough estimates (single developer + agent support)

| Phase | Estimate |
|---|---|
| W1 spike completion + host matrix | 2–4 days (timeboxed) |
| W2 bridge completeness | 3–5 days |
| W3 UI parity | 1.5–2.5 weeks (largest item; ships incrementally behind the flag) |
| W4 drag-out | 2–3 days |
| W5 CI | 0.5 day (job) + 1–2 days (screenshot scenes) |
| W6 switchover & retirement | 3–5 days |

## 5. Open decisions for the roadmap session

1. Milestone numbering/packaging (suggestion: W5-CI folded into the next
   milestone regardless; W1+W2 as one milestone; W3 split into 2–3).
2. Whether UI parity must be 100% before default-ON, or whether the web UI
   can ship as default with the last surfaces (note map, chain editor)
   following in a point release.
3. Linux webview support: commit or explicitly document as unsupported.
4. Morph-editing semantics (mirror native behavior vs redesign in web UI).
5. When the WASM-engine host replaces `groove-math.js`/mock model — ties
   this migration to the docs-playground roadmap item.
