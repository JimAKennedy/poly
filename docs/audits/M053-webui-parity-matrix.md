# M053 WebUI ↔ Native Feature Parity Matrix

> **Status:** In progress (scaffolded by S01/T01, 2026-07-28)
> **Lifecycle:** Audit snapshot — reflects the tree at the commit that closes M053/S01. Re-verify any row by opening its cited `file:line`.
> **Purpose:** Enumerate every native VSTGUI capability and its WebUI status so S02 can size each gap and make the close-all-native vs keep-both decision. This doc is the sole input to S02.

## How to read this matrix

Each **native view** (`plugin/source/ui/*_view.{cpp,h}`) gets one or more rows. A row names the native control, the WebUI counterpart (or `none`), a **parity verdict**, and `file:line` evidence on both sides so a future agent can re-open the source and re-verify.

### Verdict tokens

| Verdict | Meaning |
|---|---|
| `parity` | WebUI reproduces the native control's behavior and data surface; no user-visible capability gap. |
| `gap` | Native exposes a capability the shipping WebUI does not reach (missing, stubbed, or unreachable). |
| `webui-only` | Capability exists only in the WebUI with no native equivalent. |
| `divergent` | Both surfaces implement the capability but with materially different behavior, data model, or affordances. |

Every populated row **must** carry exactly one verdict token from `{parity, gap, webui-only, divergent}`. The completeness test (`docs/audits/parity-matrix.test.mjs`, added in T06) asserts all 15 native views appear and every row carries a verdict token.

## Dual-surface inventory

### Native UI surface — 15 views (`plugin/source/ui/`)

Every file below must be represented by at least one matrix row.

| # | Native view | Header | Source | Audit lane |
|---|---|---|---|---|
| 1 | `cell_editor_view` | `plugin/source/ui/cell_editor_view.h` | `plugin/source/ui/cell_editor_view.cpp` | A |
| 2 | `lane_grid_view` | `plugin/source/ui/lane_grid_view.h` | `plugin/source/ui/lane_grid_view.cpp` | A |
| 3 | `timeline_step_editor_view` | `plugin/source/ui/timeline_step_editor_view.h` | `plugin/source/ui/timeline_step_editor_view.cpp` | A |
| 4 | `micro_timing_editor_view` | `plugin/source/ui/micro_timing_editor_view.h` | `plugin/source/ui/micro_timing_editor_view.cpp` | A |
| 5 | `lane_edit_view` | `plugin/source/ui/lane_edit_view.h` | `plugin/source/ui/lane_edit_view.cpp` | B |
| 6 | `cross_rhythm_view` | `plugin/source/ui/cross_rhythm_view.h` | `plugin/source/ui/cross_rhythm_view.cpp` | B |
| 7 | `phase_alignment_view` | `plugin/source/ui/phase_alignment_view.h` | `plugin/source/ui/phase_alignment_view.cpp` | B |
| 8 | `phrase_edit_view` | `plugin/source/ui/phrase_edit_view.h` | `plugin/source/ui/phrase_edit_view.cpp` | B |
| 9 | `envelope_curve_view` | `plugin/source/ui/envelope_curve_view.h` | `plugin/source/ui/envelope_curve_view.cpp` | C |
| 10 | `velocity_view` | `plugin/source/ui/velocity_view.h` | `plugin/source/ui/velocity_view.cpp` | C |
| 11 | `note_map_view` | `plugin/source/ui/note_map_view.h` | `plugin/source/ui/note_map_view.cpp` | C |
| 12 | `header_view` | `plugin/source/ui/header_view.h` | `plugin/source/ui/header_view.cpp` | D |
| 13 | `scene_bar_view` | `plugin/source/ui/scene_bar_view.h` | `plugin/source/ui/scene_bar_view.cpp` | D |
| 14 | `chain_popover_view` | `plugin/source/ui/chain_popover_view.h` | `plugin/source/ui/chain_popover_view.cpp` | D |
| 15 | `export_controls_view` | `plugin/source/ui/export_controls_view.h` | `plugin/source/ui/export_controls_view.cpp` | D |

### WebUI surface (`webui/`)

| Artifact | Path | Role |
|---|---|---|
| Render + interaction logic | `webui/ui.js` (1350 lines) | All WebUI views, chrome, and host-message handlers. |
| Markup shell | `webui/index.html` (125 lines) | Root DOM containers the JS renders into. |
| Styles | `webui/ui.css` | WebUI presentation. |
| Host bridge (JS side) | `webui/host-iface.js`, `webui/plugin-host.js` | Message transport to/from the plugin. |
| Native host embed | `plugin/source/webui/web_ui_view.cpp` (999 lines) | Native-side webview host + message pump. |
| Bridge param map | `plugin/source/webui/bridge_params.h` (112 lines) | Param ID ↔ webui key mapping. |

## Parity matrix

One row per native view/control. Verdict cells are filled by the per-lane audit tasks (T02–T05); rows scaffolded here carry `_pending_` until their lane audits them. The `_pending_` marker is **not** a verdict token — the T06 completeness test requires every row to have been resolved to a `{parity, gap, webui-only, divergent}` token before the slice closes.

### Lane A — grid and timeline step editing (T02)

| Native View | Native Path (file:line) | WebUI Path (file:line) | Parity Verdict | Evidence / Notes |
|---|---|---|---|---|
| `lane_grid_view` | `plugin/source/ui/lane_grid_view.cpp:48-157` (draw), `:187-246` (select / active toggle / probability drag) | `webui/ui.js:684-798` (desk strips), active toggle `:728-731`, live phase ring `:817-835`,`:1322-1332`, mode badge `:812`, probability slider `webui/ui.js:998-1030` | `divergent` | Both surfaces expose the full per-lane overview: name, active toggle, live phase, mode indicator, probability. Native = one compact grid row per lane with an inline probability drag-bar (`lane_grid_view.cpp:122-129`,`:216-223`) and an orbit-dot phase indicator (`:131-153`). WebUI = vertical strips with an SVG ring phase indicator + ladder, and probability lives in the `expr` deep-pane slider (`ui.js:1001`). No capability gap; layout and affordances differ materially → divergent. |
| `cell_editor_view` | `plugin/source/ui/cell_editor_view.cpp:61-158` (draw / +/- buttons), `:243-257` (drag-size → `sendCellSizes`) | `webui/ui.js:873-918` (additive-cells toggle + per-cell edit), bridge `plugin/source/webui/web_ui_view.cpp:280-300` (`setCells` → `sendCellSizes`) | `divergent` | Both edit the additive `cellCount`/`cellSizes` data surface and call `sendCellSizes`. Native: `+`/`−` buttons add/remove cells and a **vertical drag** sets each cell size continuously 1–16 (`cell_editor_view.cpp:247-253`). WebUI: clicking a cell **cycles** it 2→3→4→2 and `+` appends a cell (`ui.js:908-917`); WebUI size range is limited to 2–4 vs native's 1–16. Same data model, materially different affordance and range → divergent. |
| `timeline_step_editor_view` | `plugin/source/ui/timeline_step_editor_view.cpp:77-150` (draw + `onMouseDown` toggle), gated on `cfg.timeline` (`:65`,`:93`) → `sendTimelinePattern` (`:147`) | `webui/ui.js:846-847` (ladder step → `toggleStep`), `:864-893` (deep pattern pane `data-fixed` → `setFixedStep`); bridge `plugin/source/webui/web_ui_view.cpp:234-244` (`toggleStep`, gated on `cfg.timeline`) & `:303-312` (`setFixedStep`) → `sendTimelinePattern` | `parity` | WebUI reproduces the native fixed-pattern step toggle, gated identically on timeline mode, wired end-to-end through the **real native bridge** (not only `webui/mock-host.js`) to `sendTimelinePattern`. **This contradicts the milestone's motivating premise that timeline editing is "unreachable in shipping WebUI":** both `toggleStep` and `setFixedStep` are handled in `web_ui_view.cpp`. Static evidence = parity; S02 should confirm at runtime that the deep pane / ladder is visually reachable and enabled. |
| `micro_timing_editor_view` | `plugin/source/ui/micro_timing_editor_view.cpp:88-154` (draw), `:156-199` (drag ±20 ms, double-click reset, → `sendMicroTiming`) | `webui/ui.js:936-957` (`.mtbars` drag ±20 ms + live ms readout), bridge `plugin/source/webui/web_ui_view.cpp:314-322` (`setMicroTiming`, clamps ±20 → `sendMicroTiming`) | `parity` | Both provide per-step ±20 ms micro-timing via vertical drag with a live ms readout, calling `sendMicroTiming`. Same data surface (`microTimingMs`) and range. Minor delta: native adds a **double-click-to-zero** reset gesture (`micro_timing_editor_view.cpp:164-171`) that the WebUI lacks (drag toward centre only) — a convenience affordance, not a capability gap → parity. |

### Lane B — lane config and rhythm generation (T03)

| Native View | Native Path (file:line) | WebUI Path (file:line) | Parity Verdict | Evidence / Notes |
|---|---|---|---|---|
| `lane_edit_view` | `plugin/source/ui/lane_edit_view.cpp` | _tbd_ | _pending_ | Filled by T03. |
| `cross_rhythm_view` | `plugin/source/ui/cross_rhythm_view.cpp` | _tbd_ | _pending_ | Filled by T03. |
| `phase_alignment_view` | `plugin/source/ui/phase_alignment_view.cpp` | _tbd_ | _pending_ | Filled by T03. |
| `phrase_edit_view` | `plugin/source/ui/phrase_edit_view.cpp` | _tbd_ | _pending_ | Filled by T03. |

### Lane C — per-note expression (T04)

| Native View | Native Path (file:line) | WebUI Path (file:line) | Parity Verdict | Evidence / Notes |
|---|---|---|---|---|
| `envelope_curve_view` | `plugin/source/ui/envelope_curve_view.cpp` | _tbd_ | _pending_ | Filled by T04. |
| `velocity_view` | `plugin/source/ui/velocity_view.cpp` | _tbd_ | _pending_ | Filled by T04. |
| `note_map_view` | `plugin/source/ui/note_map_view.cpp` | _tbd_ | _pending_ | Filled by T04. |

### Lane D — chrome (header, scene, chain, export) (T05)

| Native View | Native Path (file:line) | WebUI Path (file:line) | Parity Verdict | Evidence / Notes |
|---|---|---|---|---|
| `header_view` | `plugin/source/ui/header_view.cpp` | _tbd_ | _pending_ | Filled by T05. |
| `scene_bar_view` | `plugin/source/ui/scene_bar_view.cpp` | _tbd_ | _pending_ | Filled by T05. |
| `chain_popover_view` | `plugin/source/ui/chain_popover_view.cpp` | _tbd_ | _pending_ | Filled by T05. |
| `export_controls_view` | `plugin/source/ui/export_controls_view.cpp` | _tbd_ | _pending_ | Filled by T05. |

## WebUI-only capabilities

Capabilities present in `webui/ui.js` with **no** native equivalent. Populated during the lane audits (T02–T05) and finalized in T06. Each entry carries the `webui-only` verdict and `file:line` evidence in `webui/ui.js`.

| Capability | WebUI Path (file:line) | Native equivalent | Verdict | Notes |
|---|---|---|---|---|
| Per-step live emission overlay on the desk ladder (Lane A grid) | `webui/ui.js:1188-1223` | none | `webui-only` | M045 desk emission overlay marks the ladder step that most recently emitted a note per lane (kind-diffed for cheap redraws). Native `lane_grid_view` draws only the base pattern + a single orbit-dot phase indicator; it has no per-step live-emission marker. |

## Coverage checklist

All 15 native views must resolve to at least one non-`_pending_` row before S01 closes:

- [x] `cell_editor_view`
- [x] `lane_grid_view`
- [x] `timeline_step_editor_view`
- [x] `micro_timing_editor_view`
- [ ] `lane_edit_view`
- [ ] `cross_rhythm_view`
- [ ] `phase_alignment_view`
- [ ] `phrase_edit_view`
- [ ] `envelope_curve_view`
- [ ] `velocity_view`
- [ ] `note_map_view`
- [ ] `header_view`
- [ ] `scene_bar_view`
- [ ] `chain_popover_view`
- [ ] `export_controls_view`
