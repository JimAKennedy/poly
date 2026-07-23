# Poly Web UI ↔ Native Bridge Schema (v1)

Wire format between `plugin-host.js` and the native webview shell
(`plugin/source/webui/`, built with `-DPOLY_WEB_UI=ON`). Every message is a
single JSON object with a `v` (schema version) field. Unknown fields must be
ignored; unknown message types must be logged and dropped, never crash.

## Transport

- **JS → C++**: the shell binds a global `polyHostCall(jsonString)`.
- **C++ → JS**: the shell evaluates `window.polyHostPush(jsonString)`.
- The shell defines `window.__POLY_EMBEDDED__ = true` before page load.

## JS → C++

| type | payload | native handling |
|---|---|---|
| `ready` | — | push initial `state`; start the frame timer |
| `edit` | `paramId, value (normalized 0..1), gesture ('begin'\|'perform'\|'end')` | map to `beginEdit`/`performEdit`/`endEdit` on the controller — automation-correct |
| `action` | `name, payload` (see host-iface.js action list) | structural edits with no single-param representation: route to the existing message paths or dedicated setters; each mutation is followed by a fresh `state` push. Actions include `applyPreset {index}` (-1=Init, 0..N-1=factory where N = `state.presets.length`; source: `site/src/generated/presets.json`) |

`paramId` strings map to `ParamIDs` via a table in the shell (e.g.
`"macro.density"` → `kMacroDensity`, `"lane.3.hits"` →
`laneCoreParam(3, kCoreHits)`), so the JS side never hardcodes numeric VST
param IDs.

### Action reference

<!-- Regenerate via `node scripts/generate-bridge-schema-doc.mjs`. Do not hand-edit content between markers. -->

<!-- BEGIN GENERATED: actions -->
| Action | Payload type | Required fields | Notes |
|---|---|---|---|
| `toggleStep` | `payloadToggleStep` | `lane`, `step` |  |
| `setEuclid` | `payloadSetEuclid` | `lane` |  |
| `setCells` | `payloadSetCells` | `lane`, `cells` |  |
| `setFixedStep` | `payloadSetFixedStep` | `lane`, `step`, `on` |  |
| `setMicroTiming` | `payloadSetMicroTiming` | `lane`, `step`, `ms` |  |
| `setEnvelope` | `payloadSetEnvelope` | `lane`, `index` |  |
| `selectScene` | `payloadSelectScene` | `scene` |  |
| `applyPreset` | `payloadApplyPreset` | `index` | index=-1 loads Init (all lanes cleared). index>=0 selects a factory preset from the runtime inventory (site/src/generated/presets.json). No hard maximum here: bounds are enforced by the native side against the actual inventory. |
| `togglePlay` | `payloadEmpty` | — | Empty payload for: togglePlay, exportRequest, chainAddEntry, resetNoteMap |
| `exportRequest` | `payloadEmpty` | — | Empty payload for: togglePlay, exportRequest, chainAddEntry, resetNoteMap |
| `chainAddEntry` | `payloadEmpty` | — | Empty payload for: togglePlay, exportRequest, chainAddEntry, resetNoteMap |
| `chainRemoveEntry` | `payloadChainRemoveEntry` | `index` |  |
| `resetNoteMap` | `payloadEmpty` | — | Empty payload for: togglePlay, exportRequest, chainAddEntry, resetNoteMap |
| `setNoteMap` | `payloadSetNoteMap` | `note`, `output` |  |
| `setAccent` | `payloadSetAccent` | `lane`, `step`, `value` |  |
<!-- END GENERATED: actions -->

## C++ → JS

| type | payload | cadence |
|---|---|---|
| `state` | `state: State` (full snapshot — see host-iface.js) | on `ready`, preset load, scene switch, `setComponentState`, and after every `action` |
| `frame` | `frame: {t8, playing, convLeft, lanes:[{ph, step}]}` | ~30 Hz message-thread timer while the editor is open; sourced from the same values the read-only feedback params carry today |

### Message reference

<!-- Regenerate via `node scripts/generate-bridge-schema-doc.mjs`. Do not hand-edit content between markers. -->

<!-- BEGIN GENERATED: messages -->
| Message type | Direction | Fields | Required |
|---|---|---|---|
| `msgReady` | JS → C++ | `type`: `"ready"`<br>`v`: integer | `type`, `v` |
| `msgEdit` | JS → C++ | `type`: `"edit"`<br>`v`: integer<br>`paramId`: string<br>`value`: number<br>`gesture`: `begin`\|`perform`\|`end` | `type`, `v`, `paramId`, `value`, `gesture` |
| `msgAction` | JS → C++ | `type`: `"action"`<br>`v`: integer<br>`name`: `toggleStep`\|`setEuclid`\|`setCells`\|`setFixedStep`\|`setMicroTiming`\|`setEnvelope`\|`selectScene`\|`applyPreset`\|`togglePlay`\|`exportRequest`\|`chainAddEntry`\|`chainRemoveEntry`\|`resetNoteMap`\|`setNoteMap`\|`setAccent`<br>`payload`: object | `type`, `v`, `name`, `payload` |
| `msgState` | C++ → JS | `type`: `"state"`<br>`state`: `state` | `type`, `state` |
| `msgFrame` | C++ → JS | `type`: `"frame"`<br>`frame`: `frame` | `type`, `frame` |
<!-- END GENERATED: messages -->

## State mapping (C++ side)

`State` is produced from the *selected scene's* `GrooveState` after
`sanitizeGrooveState` (engine truth, not controller cache). Fields the v1 UI
does not render (mutation, drift, phrase, kotekan, accent masks, note map,
chain) are still included so the UI can grow without schema churn; the UI
ignores what it doesn't know.

`State.presets` is an array of `{name, description}` for the factory presets
from `site/src/generated/presets.json`. `applyPreset` accepts index `-1`
(Init) or `0..presets.length-1`.
`State.preset` is the name of the currently active preset (empty string if
no preset / custom edits).

## Invariants

1. C++ owns truth. The UI never assumes an action succeeded until the
   following `state` push confirms it.
2. Feedback frames are advisory visuals; they must never carry editable
   state.
3. Both sides tolerate version skew: check `v`, degrade gracefully.
4. No remote content, no eval of pushed data beyond `JSON.parse`.
