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
| `action` | `name, payload` (see host-iface.js action list) | structural edits with no single-param representation: route to the existing message paths or dedicated setters; each mutation is followed by a fresh `state` push. Actions include `applyPreset {index}` (-1=Init, 0-13=factory) |

`paramId` strings map to `ParamIDs` via a table in the shell (e.g.
`"macro.density"` → `kMacroDensity`, `"lane.3.hits"` →
`laneCoreParam(3, kCoreHits)`), so the JS side never hardcodes numeric VST
param IDs.

## C++ → JS

| type | payload | cadence |
|---|---|---|
| `state` | `state: State` (full snapshot — see host-iface.js) | on `ready`, preset load, scene switch, `setComponentState`, and after every `action` |
| `frame` | `frame: {t8, playing, convLeft, lanes:[{ph, step}]}` | ~30 Hz message-thread timer while the editor is open; sourced from the same values the read-only feedback params carry today |

## State mapping (C++ side)

`State` is produced from the *selected scene's* `GrooveState` after
`sanitizeGrooveState` (engine truth, not controller cache). Fields the v1 UI
does not render (mutation, drift, phrase, kotekan, accent masks, note map,
chain) are still included so the UI can grow without schema churn; the UI
ignores what it doesn't know.

`State.presets` is an array of `{name, description}` for the 14 factory
presets (index 0-13), always included. `State.preset` is the name of the
currently active preset (empty string if no preset / custom edits).

## Invariants

1. C++ owns truth. The UI never assumes an action succeeded until the
   following `state` push confirms it.
2. Feedback frames are advisory visuals; they must never carry editable
   state.
3. Both sides tolerate version skew: check `v`, degrade gracefully.
4. No remote content, no eval of pushed data beyond `JSON.parse`.
