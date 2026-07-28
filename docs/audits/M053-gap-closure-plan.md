# M053 WebUI ↔ Native Gap-Closure Plan

> **Status:** Gap ledger complete (T01); timeline-reachability flag resolved and Go Decision recorded — **close-all / proceed with native decommission** (T02). **Owner amendment (2026-07-28):** milestone owner (human) directed that **G06 (drag-and-drop MIDI export) be closed, not accepted-as-divergence** — via a native drag-source window opened on export, additive to the existing Save-As path. Ledger + Go Decision below updated accordingly; all 9 capability gaps now close.
> **Lifecycle:** Decision artifact — sizes every native-only capability/affordance from the S01 parity matrix and records the close-all-vs-keep-both go decision that drives the S03–S06 roadmap reassessment.
> **Purpose:** Turn each native-only delta surfaced by `docs/audits/M053-webui-parity-matrix.md` (S01) into an effort-sized, per-gap recommendation row, then let the milestone owner decide whether to close the WebUI gaps (native decommission) or keep both surfaces and document the divergence.
> **Upstream input:** `docs/audits/M053-webui-parity-matrix.md` (the S01 parity matrix — sole source for the rows below).
> **Completeness:** Mechanically enforced by `docs/audits/gap-closure-plan.test.mjs` (`node --test`) — asserts the doc is non-empty, every gap-ledger row carries exactly one valid size token, no unresolved placeholder token survives, a Go Decision token is present, and the roadmap-reassessment section exists.

## How to read this ledger

The S01 matrix verdicted all 15 native views but did **not** size effort. This doc enumerates every native-only capability or affordance — i.e. everything the native VSTGUI surface does that the shipping WebUI does **not** reach or reproduce — as one **gap-ledger row**, sizes it, and recommends how to resolve it.

Note: no S01 row carries the bare `gap` verdict. The native-only deltas live inside the `divergent` rows (materially different affordances, one surface missing overlays) and inside the two `parity` rows that flagged minor native-only conveniences. Every such delta is captured below so a future edit cannot silently drop one.

### Size tokens

| Size | Meaning |
|---|---|
| `S` | Small — a few hours. Localized change in `webui/ui.js` / `webui/ui.css`; no new bridge action or param wiring. |
| `M` | Medium — ~0.5–2 days. New bridge action/message or param wired across `plugin/source/webui/web_ui_view.cpp` + `webui/ui.js`, or a new interactive control with state. |
| `L` | Large — ~2–5 days. A new rich rendering/visualization subsystem, or a capability constrained by the WebView sandbox that needs real design work. |
| `XL` | Extra-large — >5 days or architecturally risky. Needs new host APIs or a major refactor. (None in this ledger; token defined for completeness.) |

### Categories

| Category | Meaning |
|---|---|
| `capability-gap` | The WebUI user genuinely cannot reach a data surface or action the native user can. Closing it changes what the user can *do*. |
| `cosmetic/viz-divergence` | Both surfaces reach the underlying capability/data; native renders a richer or different *visualization* of it. Closing it changes only how it *looks*, not what the user can do. |

### Recommendation tokens

| Recommendation | Meaning |
|---|---|
| `close-in-webui` | Build the missing affordance in the WebUI so native can be decommissioned without user-visible loss. |
| `close-native-drag-helper` | Close the gap with a small **platform-native** helper window (NSView*/HWND, mirroring the existing `openMidiSaveDialog` in `plugin/source/webui/platform_save_dialog.h`) triggered from the WebUI — used where the WebView sandbox structurally cannot perform the action. Not VSTGUI, so it does **not** reintroduce the native-UI codebase S05 deletes. |
| `document-as-acceptable-divergence` | Accept the difference; record it as intentional and do not build it in the WebUI. |

## Gap ledger

Each row is one native-only capability/affordance. `Size` holds exactly one token from `{S, M, L, XL}`. `Rec` holds one recommendation token. Evidence points back to the S01 matrix row (which carries the `file:line` citations on both surfaces).

| Gap ID | Source native view | Native-only capability / affordance | Category | Size | Effort rationale | Rec |
|---|---|---|---|---|---|---|
| G01 | `cell_editor_view` | Cell size range 1–16 via continuous vertical drag. WebUI only cycles cells 2→3→4 (range 2–4, no 5–16). | `capability-gap` | `M` | Widen the WebUI cell control from a 3-value cycle to a 1–16 drag/stepper and pass the value straight through the existing `setCells`→`sendCellSizes` bridge; no new bridge action, but a new interactive control + range validation. | `close-in-webui` |
| G02 | `lane_edit_view` | Inline lane **rename** (keyboard edit → `setLaneName`). WebUI lane names are display-only, no handler. | `capability-gap` | `M` | Add an editable name field in the WebUI strip and a new `setLaneName` bridge action across `web_ui_view.cpp` + `ui.js`; string round-trip + param persistence make this more than a slider wire-up. | `close-in-webui` |
| G03 | `lane_edit_view` | MIDI channel **"Auto"** (ch index 0). WebUI channel slider maps `(ch-1)/15` = CH 1–16 only, so index 0 is unreachable. | `capability-gap` | `S` | Extend the WebUI channel control's low end to include an "Auto"/0 position and adjust the normalization; single-control change, existing param. | `close-in-webui` |
| G04 | `envelope_curve_view` | Continuous envelope **depth** editing (vertical drag) + right-click depth→1.0 reset. WebUI depth is display-only (fixed 0.3 on add; no drag/slider). | `capability-gap` | `M` | Add a depth drag/slider + reset gesture to the WebUI `env` deep-pane and route it through the existing `setEnvelope`→`sendEnvelopeUpdate` bridge (depth field already carried); UI control + gesture, no new bridge action. | `close-in-webui` |
| G05 | `note_map_view` | **Lane association** (8 lane source-note rows) + **named GM-drum picker** (46 GM drum names). WebUI shows all 128 raw note rows with ±1 steppers and no GM names. | `capability-gap` | `M` | Add a GM-drum name table + optional lane-scoped view to the WebUI note-map modal; data is a static 46-entry name map, but the picker UX and lane filtering are real work. WebUI already exceeds native on raw coverage (128 vs 8). | `close-in-webui` |
| G06 | `export_controls_view` | **Drag-and-drop MIDI export** — `doDrag` writes a temp `.mid` the user drags into the DAW. | `capability-gap` | `L` | The WebView sandbox cannot initiate a native file-drag into the host DAW, so the WebUI "Export" action opens a small native drag-source window (NSView*/HWND, same platform seam as `openMidiSaveDialog`) holding a draggable MIDI-clip proxy the user drags straight into the DAW; the native side registers the drag pasteboard/`IDataObject` with the temp `.mid`. Additive to Save-As, not a replacement. Sized `L` for the two platform implementations (mac drag pasteboard + win OLE drag) plus WebUI trigger wiring. | `close-native-drag-helper` |
| G07 | `export_controls_view` | **Continuous capture length 1–32 bars** (`kCaptureLength`). WebUI cycles a fixed `{4,8,16,32}` set only. | `capability-gap` | `S` | Replace the 4-value bars cycle with a 1–32 stepper/drag bound to the same `setCaptureBars` path; single-control change. | `close-in-webui` |
| G08 | `micro_timing_editor_view` | **Double-click-to-zero** micro-timing reset gesture. WebUI drag reaches 0 but has no one-gesture reset. | `capability-gap` | `S` | Add a dblclick handler on the WebUI `.mtbars` that writes 0 via the existing `setMicroTiming` path; trivial convenience affordance. | `close-in-webui` |
| G09 | `lane_grid_view` | Inline per-row **probability drag-bar** + **orbit-dot** phase indicator (compact grid-row layout). WebUI reaches probability via the `expr` deep-pane slider and phase via an SVG ring. | `cosmetic/viz-divergence` | `S` | Both surfaces expose name/active/phase/mode/probability; only layout and the phase-dot style differ. Matching native's inline bar is cosmetic re-skinning, not new capability. | `document-as-acceptable-divergence` |
| G10 | `cross_rhythm_view` | Convergence **diamonds/lines**, **kotekan ghost dots**, **humanize whiskers**, and **swing displacement** overlays on a shared LCM/PPQ timeline. WebUI weave has the convergence countdown but none of these overlays. | `cosmetic/viz-divergence` | `L` | Read-only visualization only (no editing on either surface). Reproducing the LCM-span overlays + ghost/whisker/swing glyphs in the WebUI weave is a substantial new rendering pass; the convergence *information* (countdown/meter) is already present. | `document-as-acceptable-divergence` |
| G11 | `phase_alignment_view` | **All-lanes-on-one-concentric-plot** phase-relationship framing, plus **drift-rate trails** and **phrase play/gap arcs**. WebUI shows a per-strip ring with a live needle only. | `cosmetic/viz-divergence` | `L` | Both show each lane's live phase; native adds a dedicated relational plot + drift/phrase overlays. A new SVG concentric-plot view is real rendering work, but no capability is lost — phase is already live per strip. | `document-as-acceptable-divergence` |
| G12 | `velocity_view` | Read-only **live velocity meter** with translucent **spread band** + dashed **ghost-floor line** per lane. WebUI edits vel/ghost/spread and shows live velocity as step-dot magnitude, but has no dedicated meter. | `cosmetic/viz-divergence` | `M` | The velocity/ghost/spread data is editable in the WebUI (native meter is read-only); adding the spread-band + ghost-floor meter overlay is a visualization enhancement, not a capability the user lacks. | `document-as-acceptable-divergence` |
| G13 | `envelope_curve_view` | **All-lane curve overlay plot** with a live phase+env-value marker (every active lane's envelope on one plot). WebUI renders per-envelope SVG curves without the multi-lane overlay. | `cosmetic/viz-divergence` | `M` | Distinct from G04 (depth editing): this is the multi-lane *visualization*. WebUI already draws per-envelope curves + a live phase line; the all-lane overlay is a viz add, not a lost capability. | `document-as-acceptable-divergence` |
| G14 | `phrase_edit_view` | **Phrase cycle schematic** bar (`drawPhraseSchematic`). WebUI edits all six phrase knobs but has no schematic visual. | `cosmetic/viz-divergence` | `S` | Every phrase knob is already editable in the WebUI `adv` pane (verdicted `parity`); the schematic is a decorative diagram with no editing. | `document-as-acceptable-divergence` |
| G15 | `phrase_edit_view` | **Gap/Offset gating** — native disables Gap/Offset when Length is off. WebUI leaves the sliders always active. | `capability-gap` | `S` | Small behavioral-polish fix: mirror the native enable/disable gate in the WebUI `adv` sliders. No data-model change; prevents editing inert params. | `close-in-webui` |
| G16 | `header_view` | **POLY title** + plugin **version string** in the header. WebUI header has no title/version chrome. | `cosmetic/viz-divergence` | `S` | Static branding text; add to the WebUI `renderChrome` header if desired. No capability. | `document-as-acceptable-divergence` |

### Ledger tally

- **16 gap rows** total, covering every native-only delta from the S01 matrix (10 `divergent` rows + native-only conveniences flagged in the 2 `parity` rows `micro_timing_editor_view` and `phrase_edit_view`).
- **Capability gaps (9 — all closed):** G01, G02, G03, G04, G05, G06, G07, G08, G15. Eight are `close-in-webui`; **G06 (drag-and-drop MIDI export)** is `close-native-drag-helper` — closed via a native drag-source window rather than pure WebUI, per the 2026-07-28 owner amendment. No capability gap is left as an accepted divergence.
- **Cosmetic / viz-divergence (7):** G09, G10, G11, G12, G13, G14, G16 — all recommended `document-as-acceptable-divergence`.
- **Size distribution:** `S` ×7 (G03, G07, G08, G09, G14, G15, G16), `M` ×6 (G01, G02, G04, G05, G12, G13), `L` ×3 (G06, G10, G11). No `XL`.
- **Not in this ledger:** WebUI-**only** capabilities (e.g. the M045 desk emission overlay, S01 `webui-only` row) are advantages the native surface lacks, not native-only gaps, so they impose no close-in-webui work and are excluded here.

> The `Size distribution` bullet above is a human-readable summary; the authoritative per-row size tokens are the `Size` column of the gap ledger, which the completeness test parses row-by-row.

## Timeline reachability (MEM036)

**Verdict: RESOLVED — the S01 `timeline_step_editor_view` `parity` verdict holds.** Static reachability audit of `webui/ui.js` confirms the deep pattern pane, the Timeline-mode toggle, and per-step editing are all reachable and enabled in the shipping WebUI, with **no `cfg.timeline` (or any config) gate** hiding or disabling them.

Reachability chain (all in `webui/ui.js`):

1. **Deep pane opens unconditionally.** Each lane strip renders an expand affordance (`.ex` button, `ui.js:726`) whose click calls `expandStrip()` (`ui.js:789`). The `pattern` pane is the default-active pane (`<div class="pane on" data-pane="pattern">`, `ui.js:709`). No lane-mode or config predicate guards the expand or the pane — any lane can open its pattern pane.
2. **Timeline-mode toggle is always rendered and enabled.** `buildPanes()` renders the `Timeline mode` switch in *both* branches — timeline-on (`ui.js:865`) and timeline-off (`ui.js:869`) — so the toggle is present regardless of current mode. Its handler (`ui.js:880-886`) edits the real `lane.<li>.timeline` param (begin/perform/end), i.e. it is live-wired, not a display stub.
3. **Per-step editing is reachable two ways once timeline is on.** (a) The strip **ladder** buttons attach a `toggleStep` action when `l.timeline` is true (`buildLadder()`, `ui.js:846-847`); (b) the deep pane's **fixed-pulse row** (`data-fixed` buttons) attaches a `setFixedStep` action (`ui.js:888-893`). Both drive the engine through the bridge — genuine step editing, matching native `timeline_step_editor_view`.

Grep-anchored negative check: `grep -in "cfg.timeline" webui/ui.js` returns nothing; the only `timeline` gating in the file is per-lane runtime state (`l.timeline`), not a build/config feature flag. The `parity` verdict is therefore trusted for the go decision below; full DAW-runtime confirmation of the pane remains explicitly deferred to the downstream execution slices (S02 is a static-analysis slice, per its Proof Level).

## Go Decision

**Decision: `close-all` — proceed with native-UI decommission (S03–S06).** (Milestone owner: autonomous default, per the S02 plan.)

The two options were `close-all` (build the missing WebUI affordances, then delete the native VSTGUI surface) versus `keep-both-document-divergence` (shrink M053 to a documentation-only outcome and maintain both UIs indefinitely).

Rationale, grounded in the gap ledger above:

- **The capability gaps are small and finite.** Of the 9 genuine `capability-gap` rows, 8 are recommended `close-in-webui` and every one of them is sized `S` or `M` — no `L`, no `XL`. Specifically: `S` ×4 (G03, G07, G08, G15) and `M` ×4 (G01, G02, G04, G05). None requires a new host API or an architectural change; each is a WebUI control + (at most) one new bridge action. Closing them is a bounded batch of work, not open-ended.
- **The one expensive capability gap is closed with a native drag helper, not carved out.** *(Amended 2026-07-28 by milestone owner.)* G06 (drag-and-drop MIDI export, `L`) was initially recommended `document-as-acceptable-divergence` because the WebView sandbox cannot initiate a native file-drag. The owner instead directed closing it: the WebUI "Export" action opens a small **native** drag-source window from which the user drags the `.mid` straight into the DAW, additive to the existing Save-As path. Crucially this reuses the existing platform seam (`plugin/source/webui/platform_save_dialog.h`'s NSView*/HWND helper), **not** VSTGUI — so it does not resurrect the native-UI codebase that S05 deletes, and the decommission goal is preserved.
- **Every remaining gap is cosmetic.** The 7 `cosmetic/viz-divergence` rows (G09–G14, G16) lose no capability — the underlying data/action is already reachable in the WebUI; only a richer native *visualization* differs. All are `document-as-acceptable-divergence`. They impose zero decommission-blocking work.
- **The timeline flag is clear.** MEM036 is resolved above: timeline step editing genuinely reaches parity in the shipping WebUI, so there is no hidden capability gap lurking behind the `parity` verdict.
- **Payoff justifies the small cost.** Decommissioning the native VSTGUI surface removes an entire parallel UI codebase (15 native views), drops the VSTGUI dependency, and collapses the project to a single UI to maintain and test. The maintenance win outweighs the bounded close-in-webui batch.

Net: because the only work required to reach *capability* parity is 8 small/medium WebUI controls (with G06 and all cosmetic deltas explicitly accepted as documented divergences), the sensible call is to close the gaps and retire the native UI rather than carry two surfaces forever.

## Roadmap reassessment recommendation

The `close-all` decision keeps the full decommission chain: **all of S03, S04, S05, and S06 survive.** (Under `keep-both`, S03–S06 would have been removed and M053 reduced to this document; that path was not taken.)

| Slice | Disposition under `close-all` | Note |
|---|---|---|
| S03 — Close parity gaps (per-gap batch, `[sketch]`) | **Survives; decompose per-gap.** | Refine into per-gap slices covering all **9** capability gaps: the 8 `close-in-webui` rows (G01, G02, G03, G04, G05, G07, G08, G15) plus **G06** via `close-native-drag-helper` (native mac + win drag-source window, WebUI trigger). Only the `cosmetic/viz-divergence` rows (G09–G14, G16) stay out of scope as documented divergences. |
| S04 — Remove `POLY_WEB_UI` CMake option + build scripts | **Survives.** | Runs after S03 lands the closed gaps. |
| S05 — Delete native `plugin/source/ui/*`, drop VSTGUI dep, update CMakeLists | **Survives.** | The core payoff of the decommission. |
| S06 — Docs: rewrite chapter 18 for WebUI-only, archive native-UI refs | **Survives.** | Records the 7 cosmetic `viz-divergence` rows (G09–G14, G16) as the intentional, accepted WebUI-vs-native deltas, and documents the new native drag-source window (G06) as the one surviving platform-native affordance alongside the WebUI. |

Actioning this reassessment (the `gsd_reassess_roadmap` write that decomposes S03 and confirms S04–S06) is owned by the post-slice orchestrator, not this task.
