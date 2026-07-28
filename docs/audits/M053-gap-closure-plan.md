# M053 WebUI ↔ Native Gap-Closure Plan

> **Status:** Gap ledger complete (T01); Go Decision _pending_ (T02).
> **Lifecycle:** Decision artifact — sizes every native-only capability/affordance from the S01 parity matrix and records the close-all-vs-keep-both go decision that drives the S03–S06 roadmap reassessment.
> **Purpose:** Turn each native-only delta surfaced by `docs/audits/M053-webui-parity-matrix.md` (S01) into an effort-sized, per-gap recommendation row, then let the milestone owner decide whether to close the WebUI gaps (native decommission) or keep both surfaces and document the divergence.
> **Upstream input:** `docs/audits/M053-webui-parity-matrix.md` (the S01 parity matrix — sole source for the rows below).
> **Completeness:** Mechanically enforced by `docs/audits/gap-closure-plan.test.mjs` (`node --test`) — asserts the doc is non-empty, every gap-ledger row carries exactly one valid size token, no `_pending_` placeholder survives, a Go Decision token is present, and the roadmap-reassessment section exists.

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
| G06 | `export_controls_view` | **Drag-and-drop MIDI export** — `doDrag` writes a temp `.mid` the user drags into the DAW. | `capability-gap` | `L` | The WebView sandbox cannot initiate a native file-drag into the host DAW; reproducing this needs a native-side drag source driven by a WebUI request, or acceptance that Save-As replaces it. Design-constrained, not a simple control. | `document-as-acceptable-divergence` |
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
- **Capability gaps (9):** G01, G02, G03, G04, G05, G06, G07, G08, G15. Of these, 8 are recommended `close-in-webui`; **G06 (drag-and-drop MIDI export)** is the sole capability gap recommended `document-as-acceptable-divergence` (WebView sandbox constraint).
- **Cosmetic / viz-divergence (7):** G09, G10, G11, G12, G13, G14, G16 — all recommended `document-as-acceptable-divergence`.
- **Size distribution:** `S` ×7 (G03, G07, G08, G09, G14, G15, G16), `M` ×6 (G01, G02, G04, G05, G12, G13), `L` ×3 (G06, G10, G11). No `XL`.
- **Not in this ledger:** WebUI-**only** capabilities (e.g. the M045 desk emission overlay, S01 `webui-only` row) are advantages the native surface lacks, not native-only gaps, so they impose no close-in-webui work and are excluded here.

> The `Size distribution` bullet above is a human-readable summary; the authoritative per-row size tokens are the `Size` column of the gap ledger, which the completeness test parses row-by-row.

## Timeline reachability (MEM036)

_pending_ — resolved in T02: static reachability audit of `webui/ui.js` (deep pattern pane render + `cfg.timeline` gating) confirming whether the `timeline_step_editor_view` `parity` verdict holds (deep pane / ladder actually reachable and enabled in the shipping WebUI).

## Go Decision

_pending_ — recorded in T02: close-all (proceed with native decommission, decompose S03 per-gap) vs keep-both-document-divergence (shrink M053 to documentation-only), with rationale grounded in the ledger above.

## Roadmap reassessment recommendation

_pending_ — recorded in T02: names which of S03–S06 survive the go decision.
