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
| `lane_grid_view` | `plugin/source/ui/lane_grid_view.cpp` | _tbd_ | _pending_ | Filled by T02. |
| `cell_editor_view` | `plugin/source/ui/cell_editor_view.cpp` | _tbd_ | _pending_ | Filled by T02. |
| `timeline_step_editor_view` | `plugin/source/ui/timeline_step_editor_view.cpp` | _tbd_ | _pending_ | Filled by T02. |
| `micro_timing_editor_view` | `plugin/source/ui/micro_timing_editor_view.cpp` | _tbd_ | _pending_ | Filled by T02. |

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
| _to be populated by lane audits_ | | none | `webui-only` | |

## Coverage checklist

All 15 native views must resolve to at least one non-`_pending_` row before S01 closes:

- [ ] `cell_editor_view`
- [ ] `lane_grid_view`
- [ ] `timeline_step_editor_view`
- [ ] `micro_timing_editor_view`
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
