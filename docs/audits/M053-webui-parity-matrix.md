---
class: archived
---

# M053 WebUI ↔ Native Feature Parity Matrix

> **Milestone numbering (legacy scheme):** the `M0xx` identifiers in this
> document — `M053` and any others it cites — belong to the repo's earlier
> commit-message milestone numbering. They do **not** correspond to the GSD
> milestone scheme (`M001` onwards) now used for planned work: a GSD `M001` is
> a different milestone from anything named here. Read these references as
> historical labels, not as pointers into the current roadmap.

> **Archived (2026-07-28)** — frozen audit snapshot from M053/S01. Its `file:line`
> citations point into `plugin/source/ui/*`, the native VSTGUI views deleted in
> M053/S05; re-verify any row by opening its cited `file:line` at the pre-S05
> commit, not against `HEAD`. Not maintained against the current tree.

> **Status:** Complete (all 15 native views verdicted; S01/T06, 2026-07-28)
> **Lifecycle:** Audit snapshot — reflects the tree at the commit that closes M053/S01. Re-verify any row by opening its cited `file:line`.
> **Purpose:** Enumerate every native VSTGUI capability and its WebUI status so S02 can size each gap and make the close-all-native vs keep-both decision. This doc is the sole input to S02.
> **Completeness:** Mechanically enforced by `docs/audits/parity-matrix.test.mjs` (`node --test`) — asserts all 15 native views carry a verdicted row, every verdict cell holds one `{parity, gap, webui-only, divergent}` token, and no `_pending_` placeholder survives.

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
| `lane_grid_view` | `plugin/source/ui/lane_grid_view.cpp:48-157` (draw), `:187-246` (select / active toggle / probability drag) | `webui/ui.js:684-798` (desk strips), active toggle `:728-731`, live phase ring `:817-835`,`:1322-1332`, mode badge `:812`, probability slider `webui/ui.js:998-1030` | `divergent` | Both surfaces expose the full per-lane overview: name, active toggle, live phase, mode indicator, probability. Native = one compact grid row per lane with an inline probability drag-bar (`lane_grid_view.cpp:122-129`,`:216-223`) and an orbit-dot phase indicator (`:131-153`). WebUI = vertical strips with an SVG ring phase indicator + ladder, and probability lives in the `expr` deep-pane slider (`ui.js:1001`). No capability gap; layout and affordances differ materially → divergent. | <!-- [file-line-ok]: audit snapshot; refs pinned to the pre-S05 commit (native plugin/source/ui/* since deleted) -->
| `cell_editor_view` | `plugin/source/ui/cell_editor_view.cpp:61-158` (draw / +/- buttons), `:243-257` (drag-size → `sendCellSizes`) | `webui/ui.js:873-918` (additive-cells toggle + per-cell edit), bridge `plugin/source/webui/web_ui_view.cpp:280-300` (`setCells` → `sendCellSizes`) | `divergent` | Both edit the additive `cellCount`/`cellSizes` data surface and call `sendCellSizes`. Native: `+`/`−` buttons add/remove cells and a **vertical drag** sets each cell size continuously 1–16 (`cell_editor_view.cpp:247-253`). WebUI: clicking a cell **cycles** it 2→3→4→2 and `+` appends a cell (`ui.js:908-917`); WebUI size range is limited to 2–4 vs native's 1–16. Same data model, materially different affordance and range → divergent. | <!-- [file-line-ok]: audit snapshot; refs pinned to the pre-S05 commit (native plugin/source/ui/* since deleted) -->
| `timeline_step_editor_view` | `plugin/source/ui/timeline_step_editor_view.cpp:77-150` (draw + `onMouseDown` toggle), gated on `cfg.timeline` (`:65`,`:93`) → `sendTimelinePattern` (`:147`) | `webui/ui.js:846-847` (ladder step → `toggleStep`), `:864-893` (deep pattern pane `data-fixed` → `setFixedStep`); bridge `plugin/source/webui/web_ui_view.cpp:234-244` (`toggleStep`, gated on `cfg.timeline`) & `:303-312` (`setFixedStep`) → `sendTimelinePattern` | `parity` | WebUI reproduces the native fixed-pattern step toggle, gated identically on timeline mode, wired end-to-end through the **real native bridge** (not only `webui/mock-host.js`) to `sendTimelinePattern`. **This contradicts the milestone's motivating premise that timeline editing is "unreachable in shipping WebUI":** both `toggleStep` and `setFixedStep` are handled in `web_ui_view.cpp`. Static evidence = parity; S02 should confirm at runtime that the deep pane / ladder is visually reachable and enabled. | <!-- [file-line-ok]: audit snapshot; refs pinned to the pre-S05 commit (native plugin/source/ui/* since deleted) -->
| `micro_timing_editor_view` | `plugin/source/ui/micro_timing_editor_view.cpp:88-154` (draw), `:156-199` (drag ±20 ms, double-click reset, → `sendMicroTiming`) | `webui/ui.js:936-957` (`.mtbars` drag ±20 ms + live ms readout), bridge `plugin/source/webui/web_ui_view.cpp:314-322` (`setMicroTiming`, clamps ±20 → `sendMicroTiming`) | `parity` | Both provide per-step ±20 ms micro-timing via vertical drag with a live ms readout, calling `sendMicroTiming`. Same data surface (`microTimingMs`) and range. Minor delta: native adds a **double-click-to-zero** reset gesture (`micro_timing_editor_view.cpp:164-171`) that the WebUI lacks (drag toward centre only) — a convenience affordance, not a capability gap → parity. | <!-- [file-line-ok]: audit snapshot; refs pinned to the pre-S05 commit (native plugin/source/ui/* since deleted) -->

### Lane B — lane config and rhythm generation (T03)

| Native View | Native Path (file:line) | WebUI Path (file:line) | Parity Verdict | Evidence / Notes |
|---|---|---|---|---|
| `lane_edit_view` | `plugin/source/ui/lane_edit_view.cpp:27-49` (12 lane + 6 phrase knob defs), `:443-471` (draw knobs + phrase), `:476-560` (tab select + vertical-drag edit), name edit `:562-657` | Pattern steppers `webui/ui.js:870-872`→`setEuclid` `:895-900`; subdivision chips `:1077-1078`,`:1112-1118`; kotekan chips `:1080-1085`,`:1119-1125`; voice sliders (vel/ghost/spread/swing/humanize/duration/note/channel) `webui/ui.js:999-1039`; probability `:1001`; bridge map `plugin/source/webui/bridge_params.h:27-46` | `divergent` | Every one of the 12 lane knobs (Steps, Subdiv, Hits, Rot, Note, Vel, Ghost, Spread, Swing, Hum, Kotek, Ch) and 6 phrase knobs is reachable in the WebUI with matching normalized ranges, but the single native radial-knob cluster is **reorganized** across three WebUI surfaces (desk-strip pattern steppers + `expr` sliders + `adv` sliders/chips). Two genuine native-only deltas: (1) **inline lane rename** (`lane_edit_view.cpp:562-657` keyboard edit → `setLaneName`) has **no WebUI handler** — lane names are display-only in `webui/ui.js` (`:509`,`:691`,`:814`); (2) MIDI channel **"Auto"** (ch index 0, `lane_edit_view.cpp:211-216`) is unreachable from the WebUI channel slider which maps `(l.ch-1)/15` = CH 1–16 only (`webui/ui.js:1008`). Same data model, materially different affordances + two capability gaps → divergent. | <!-- [file-line-ok]: audit snapshot; refs pinned to the pre-S05 commit (native plugin/source/ui/* since deleted) -->
| `cross_rhythm_view` | `plugin/source/ui/cross_rhythm_view.cpp:68-530` (draw): per-lane step dots on a shared LCM-derived PPQ span `:177-206`, convergence detection + diamonds `:229-453`, playhead `:501-521`, countdown label `:455-499`, kotekan ghost dots `:370-421`, humanize whiskers `:324-336`, swing offset `:306-311` | WebUI cloth/loom convergence weave `webui/ui.js:517-590` (`drawLoom`/`drawConvergence`): stacked lane bands + step grid `:547-567`, per-lane cycle markers `:569`, gold selvage `:573`, sweeping playhead `:581-589`; numeric convergence countdown `#conv` `:1314` and `#cmeter` fill `:1344-1345` fed by `frame.convLeft` | `divergent` | Both surfaces visualize the multi-lane polyrhythm and the convergence horizon (read-only; no editing on either). Materially different renderings: native lays each lane's hits on one shared PPQ timeline with LCM-based bar span, explicit gold convergence diamonds/lines, a beats-away countdown, kotekan ghost circles, humanize whiskers and swing displacement; the WebUI weave stacks lanes as coloured bands over a fixed 120-eighth window with cycle-boundary ticks and a single numeric convergence countdown, and has **no** convergence-diamond/kotekan-ghost/humanize-whisker overlays. Same underlying data, different visualization → divergent. | <!-- [file-line-ok]: audit snapshot; refs pinned to the pre-S05 commit (native plugin/source/ui/* since deleted) -->
| `phase_alignment_view` | `plugin/source/ui/phase_alignment_view.cpp:44-180` (draw): concentric per-lane rings `:91-127`, live phase dot from `lanePhaseOutput` `:129-160`, phrase play/gap arcs from `phrasePhaseOutput` `:96-119`, drift trails from `kDriftRate` `:134-154`, selected-lane emphasis | WebUI per-strip SVG rings `webui/ui.js:697` (markup), `:817-835` (`drawRing` onset geometry), live needle rotated by `frame.lanes[li].ph` `:1328` | `divergent` | Both surfaces show each lane's live phase (native = a moving dot on concentric rings; WebUI = a needle rotating on each desk strip's ring, updated every frame from `frame.lanes[li].ph`). But the native view's **phase-relationship framing** — all lanes on one concentric plot to read relative alignment — plus its **drift-rate trails** (`:134-154`) and **phrase play/gap arcs** (`:96-119`) have no WebUI equivalent; the WebUI ring shows only static onset geometry + a live needle per isolated strip. Same live-phase capability, materially different presentation and missing overlays → divergent. | <!-- [file-line-ok]: audit snapshot; refs pinned to the pre-S05 commit (native plugin/source/ui/* since deleted) -->
| `phrase_edit_view` | `plugin/source/ui/phrase_edit_view.cpp:24-31` (6 knob defs), `:297-355` (draw tabs + knobs + cycle schematic), `:357-417` (tab select + vertical-drag edit), gating `:339-345` (Gap/Ofs disabled when Length off) | WebUI `adv` pane sliders `webui/ui.js:1043-1050` (Length/Gap/Offset), `:1051-1056` (Mutation/Drift), `:1057-1062` (T.Offset), wired `:1087-1111`; bridge map `plugin/source/webui/bridge_params.h:33-38` | `parity` | All six phrase knobs (Length, Gap, Offset, Mutation, Drift, Timing-Offset) are editable in the WebUI `adv` pane with identical normalized ranges and formatting. `phrase_edit_view` is a standalone native panel that duplicates the phrase knobs already in `lane_edit_view`; the WebUI consolidates them into one `adv` section. Two cosmetic/minor deltas, not capability gaps: the native **phrase cycle schematic** bar (`drawPhraseSchematic`, `:184-295`) has no WebUI counterpart, and the WebUI sliders do not disable Gap/Offset when Length is off (native gates them at `:339-345`,`:375-379`). Editing surface matches → parity. | <!-- [file-line-ok]: audit snapshot; refs pinned to the pre-S05 commit (native plugin/source/ui/* since deleted) -->

### Lane C — per-note expression (T04)

| Native View | Native Path (file:line) | WebUI Path (file:line) | Parity Verdict | Evidence / Notes |
|---|---|---|---|---|
| `envelope_curve_view` | `plugin/source/ui/envelope_curve_view.cpp:79-140` (multi-lane live curve overlay + phase/env-value marker dot, selected-lane emphasis), `:150-195` (drag Y → `envelopes[0].depth`, right-click reset depth→1.0, both → `sendEnvelopeUpdate`) | `webui/ui.js:968-995` (`env` deep-pane: per-envelope SVG curve, target·period·sine label, depth %, ON/OFF chip, `+ add envelope`), live phase line `webui/ui.js:973` (`data-envph`); bridge `plugin/source/webui/web_ui_view.cpp:325-352` (`setEnvelope` → target/period/depth/on → `sendEnvelopeUpdate`) | `divergent` | Both surfaces edit the envelope data surface via the real native bridge (`sendEnvelopeUpdate`), but through materially different models. Native: **one plot overlaying every active lane's curve** with a live phase+env-value marker, editing **only `envelopes[0].depth`** by vertical drag (`envelope_curve_view.cpp:181-183`) plus a right-click depth→1.0 reset (`:154-158`); no add/remove, no target/period control. WebUI: a **per-lane multi-envelope manager** — add N superimposed envelopes each with a target (Velocity/Density/Probability, `web_ui_view.cpp:335-341`), period, and ON/OFF toggle (`ui.js:984-995`) — but depth is **display-only** (fixed 0.3 on add, no drag/slider) and there is no multi-lane overlay plot. Native-only: continuous depth drag + reset + all-lane curve visualization. WebUI-only: add/remove envelopes + target/period selection. Same data model, divergent affordances → divergent; both deltas flagged for S02 gap sizing. | <!-- [file-line-ok]: audit snapshot; refs pinned to the pre-S05 commit (native plugin/source/ui/* since deleted) -->
| `velocity_view` | `plugin/source/ui/velocity_view.cpp:49-145` (draw): per-lane live velocity bar from `velocityOutput` `:77`,`:86-93`, velocity-spread band `:95-107`, ghost-floor dashed line `:110-124`, selected-lane outline `:126-131`; **read-only, no mouse handler** | `webui/ui.js:1000` (Velocity slider), `:1002` (Ghost), `:1003` (Spread) in the `expr` deep-pane, wired `:1015-1040` via `host.edit('lane.N.{velocity,ghostFloor,spread}')`; live velocity rendered as ladder step-dot magnitude via `hitVelocity` `webui/ui.js:550`,`:612`; bridge params `plugin/source/webui/web_ui_view.cpp:712-722` (`kBaseVelocity`, `kVelocitySpread`), `plugin/source/webui/bridge_params.h:27-46` | `divergent` | The underlying velocity / spread / ghost-floor data is reachable on both surfaces, but the native and WebUI treatments are inverted. Native `velocity_view` is a **read-only live meter**: per-lane bars driven by `velocityOutput`, a translucent spread band (`velocity_view.cpp:95-107`) and a dashed ghost-floor line (`:110-124`); it has **no editing** (no `onMouseDown`). WebUI has **no dedicated velocity meter**; instead it makes velocity/ghost/spread **editable** via `expr` sliders and renders live velocity as step-dot magnitude on the desk ladder (`hitVelocity`). Native-only: the spread-band + ghost-floor live visualization. WebUI-only: direct velocity/ghost/spread editing. Same params, materially different presentation + editing surface → divergent. | <!-- [file-line-ok]: audit snapshot; refs pinned to the pre-S05 commit (native plugin/source/ui/* since deleted) -->
| `note_map_view` | `plugin/source/ui/note_map_view.cpp:63-71` (8 rows, one per lane source note `lanes[lane].midiNote`), `:97-211` (draw: GM drum name + source note + destination cell), `:213-238` (`showNoteMenu` COptionMenu of 46 GM drums → `sendNoteMap`), reset `:251-256` (`noteMap.reset` → `sendNoteMap`), close `:244-249` | `webui/ui.js:275-327` (`buildNoteMapModal`: **all 128 note rows**, −/+ steppers, `✦` remap marker, Reset, Close), `:298-309` (`setNoteMap {note, output}`), `:295-297` (`resetNoteMap`); bridge `plugin/source/webui/web_ui_view.cpp:580-594` (`resetNoteMap`, `setNoteMap` → `sendNoteMap`) | `divergent` | Both edit the same `noteMap` and call the identical bridge actions (`setNoteMap` / `resetNoteMap` → `sendNoteMap`), but expose materially different surfaces. Native shows **only the 8 lane source notes** (`note_map_view.cpp:63-71`) with GM drum-name labels and a **46-entry GM drum-name dropdown** picker (`:216-221`) for each destination. WebUI shows **all 128 raw note rows** (`ui.js:282`) with ±1 steppers and no GM drum names. Native-only: lane association + named GM-drum picker. WebUI-only: full 128-note remap coverage (native can only remap the 8 active lane notes). Same data + same actions, divergent scope and affordances → divergent. | <!-- [file-line-ok]: audit snapshot; refs pinned to the pre-S05 commit (native plugin/source/ui/* since deleted) -->

### Lane D — chrome (header, scene, chain, export) (T05)

| Native View | Native Path (file:line) | WebUI Path (file:line) | Parity Verdict | Evidence / Notes |
|---|---|---|---|---|
| `header_view` | `plugin/source/ui/header_view.cpp:50-129` (draw: POLY title, preset box, MAP button, version), `:136-193` (`onMouseDown`: MAP toggle + preset `COptionMenu` with **category submenus** `:156-171`), `applyPreset` `:195-277`, Init reset `:299-383`, note-map toggle `:279-297` | Preset dropdown `webui/ui.js:329-463` (`buildPresetMenu` with **category filter chips** `:344-381`, `applyPreset` action `:403-408`, Init entry `:388-391`), preset name `renderChrome` `:1130`, MAP → note-map modal `:324-327`,`buildNoteMapModal :275`; bridge `plugin/source/webui/web_ui_view.cpp:380-388` (`applyPreset`) | `divergent` | Both surfaces apply factory presets (incl. Init) and open the note map from the header. Same capability, materially different affordances: native uses a `COptionMenu` with **nested category submenus** (`header_view.cpp:156-171`); the WebUI uses a flat dropdown with a **category filter-chip row** (`ui.js:344-381`). Native-only cosmetic extras: the `POLY` title and plugin **version string** (`header_view.cpp:118-122`) have no WebUI header equivalent. No preset/note-map capability gap → divergent. | <!-- [file-line-ok]: audit snapshot; refs pinned to the pre-S05 commit (native plugin/source/ui/* since deleted) -->
| `scene_bar_view` | `plugin/source/ui/scene_bar_view.cpp:77-94` (A/B/Morph buttons), `:96-128` (morph slider), `:130-145` (CHAIN button), `onMouseDown :150-178` (scene → `kSceneSelect`, morph drag → `kSceneMorph`, chain toggle), morph drag `:180-201` | Scene buttons `webui/ui.js:59-61` (`selectScene` action), morph slider `:64-86` (`scene.morph` edit), chain button `:89`,`:187-191`; `renderChrome` scene state `:1133-1135`, morph fill + Morph-only visibility `:1144-1145`; bridge `plugin/source/webui/web_ui_view.cpp:356-377` (`selectScene` → `kSceneSelect`) | `parity` | Full behavioral + data match: A/B/Morph scene select (`kSceneSelect`), a horizontal morph slider bound to `kSceneMorph`, and a CHAIN toggle button. Both gate the morph slider to Morph mode (native `morphActive`, `scene_bar_view.cpp:97`; WebUI `renderChrome` display toggle, `ui.js:1144`). Same params, same affordances → parity. | <!-- [file-line-ok]: audit snapshot; refs pinned to the pre-S05 commit (native plugin/source/ui/* since deleted) -->
| `chain_popover_view` | `plugin/source/ui/chain_popover_view.cpp:140-153` (Enable toggle → `kChainEnabled`), `:156-165` (Mode 1-Shot/Loop/Ping → `kChainMode`), `:194-252` (per-entry Scene A/B/Morph + Bars −/+, `kChainEntryScene`/`kChainEntryBars`), `:255-275` (Add/Remove → `kChainEntryCount`), `onMouseDown :280-357` | `buildChainPopover` `webui/ui.js:96-181`: Enable `:106`,`:124-133`; Mode `:107`,`:134-140`; per-entry Scene `:113`,`:141-148`; Bars −/+ `:116-118`,`:150-166`; Add `:171-173` / Remove `:169`; bridge `plugin/source/webui/web_ui_view.cpp:550-575` (`chainAddEntry`/`chainRemoveEntry`), param setters `:614-632` (`chain.enabled`/`mode`/`entry.N.scene`/`entry.N.bars`) | `parity` | The WebUI reproduces every native chain control end-to-end through the real bridge: Enable toggle, three modes (1-Shot/Loop/Ping), per-entry Scene A/B/Morph and Bars 1–32 −/+ steppers, and Add/Remove entry, writing the identical `kChain*` params. Same data model, same affordances → parity. | <!-- [file-line-ok]: audit snapshot; refs pinned to the pre-S05 commit (native plugin/source/ui/* since deleted) -->
| `export_controls_view` | `plugin/source/ui/export_controls_view.cpp:154-265` (draw: Export button, Bars −/+ `kCaptureLength`, live event-count meter + progress bar), Bars ± + drag-start `onMouseDown :267-293`, **drag-and-drop MIDI** `:295-344` (`doDrag` writes a temp `.mid`), Save dialog + prefetch `:83-145`, `RequestMidiExport` `:75-81` | Export button `webui/ui.js:198-226` (`exportSaveAs` action, gated `capState===3`, result toast), capture header cluster `:228-270`: Bars cycle `{4,8,16,32}` via `setCaptureBars` `:240-249`, **Arm/Reset** via `armCapture`/`resetCapture` `:251-255`, `capReady` `:269`; bridge `plugin/source/webui/web_ui_view.cpp:503-535` (`armCapture`/`resetCapture`/`exportSaveAs`), `requestMidiExport :908` | `divergent` | Both export the captured MIDI window to a `.mid` file via a native Save-As dialog (native `openSaveDialog` `export_controls_view.cpp:83-115`; WebUI `exportSaveAs` → `web_ui_view.cpp:513-535`). Materially different models around it: native offers **continuous Bars 1–32** (`kCaptureLength`, `:277-290`), a **live event-count meter + fill bar** (`:215-262`), and **drag-and-drop MIDI export** (`doDrag`, `:325-344`) with background prefetch; the WebUI drives export through an explicit **arm→capture→complete state machine** (`armCapture`/`resetCapture`) with a 4-value bars cycle `{4,8,16,32}` and Save-As gated on `capState===3`, plus the M051 Cloth capture-timeline as the visual receipt — but **no drag-export and no event-count meter**. Native-only: drag-and-drop `.mid` + 1–32 bars + event meter. WebUI-only: arm/reset capture state machine + Cloth timeline. Same export capability, divergent affordances → divergent. | <!-- [file-line-ok]: audit snapshot; refs pinned to the pre-S05 commit (native plugin/source/ui/* since deleted) -->

## WebUI-only capabilities

Capabilities present in `webui/ui.js` with **no** native equivalent. Populated during the lane audits (T02–T05) and finalized in T06. Each entry carries the `webui-only` verdict and `file:line` evidence in `webui/ui.js`.

| Capability | WebUI Path (file:line) | Native equivalent | Verdict | Notes |
|---|---|---|---|---|
| Per-step live emission overlay on the desk ladder (Lane A grid) | `webui/ui.js:1188-1223` | none | `webui-only` | M045 desk emission overlay marks the ladder step that most recently emitted a note per lane (kind-diffed for cheap redraws). Native `lane_grid_view` draws only the base pattern + a single orbit-dot phase indicator; it has no per-step live-emission marker. | <!-- [file-line-ok]: audit snapshot; refs pinned to the pre-S05 commit (native plugin/source/ui/* since deleted) -->

## Coverage checklist

All 15 native views must resolve to at least one non-`_pending_` row before S01 closes:

- [x] `cell_editor_view`
- [x] `lane_grid_view`
- [x] `timeline_step_editor_view`
- [x] `micro_timing_editor_view`
- [x] `lane_edit_view`
- [x] `cross_rhythm_view`
- [x] `phase_alignment_view`
- [x] `phrase_edit_view`
- [x] `envelope_curve_view`
- [x] `velocity_view`
- [x] `note_map_view`
- [x] `header_view`
- [x] `scene_bar_view`
- [x] `chain_popover_view`
- [x] `export_controls_view`
