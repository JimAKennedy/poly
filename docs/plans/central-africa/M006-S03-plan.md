---
class: gated
---

# M006/S03 — Controls the browser cannot honour are disabled

**Slice:** central-africa M006/S03 (rows CA44–CA46)
**Ledger:** `docs/plans/central-africa/ledger.md`
**Decisions consumed:** `M006-decisions.md` (2026-10-18)

## Task status

- [x] 1. Note Map, Capture, Channel and Duration are disabled in the browser, with the reason
- [x] 2. Lane rename works in the browser
- [x] 3. A failed WASM load is visible

## Definition of Done

Copied verbatim from the slice:

- [x] Every control that cannot work in the browser is disabled there and
      says why, and stays enabled in the plugin
- [x] Lane rename works in the browser or is disabled there
- [x] A failed WASM load is visible to the reader rather than silently
      replaced by the mock host

## Validation

| Token | Command |
|---|---|
| `format` | `pre-commit run --all-files` |
| `webui-e2e` | `npm --prefix webui test` |
| `e2e` | `bash scripts/site-verify-local.sh` |

After every `e2e` run restore `webui/poly_engine.{js,wasm}` with
`git checkout HEAD -- webui/poly_engine.js webui/poly_engine.wasm`; this
slice changes no engine source.

## Task 1 — Note Map, Capture, Channel and Duration are disabled in the browser, with the reason

Rows: CA44.

**Files**

- Modify `webui/host-iface.js` (document the flags)
- Modify `webui/wasm-host.js`, `webui/plugin-host.js`, `webui/mock-host.js`
  (`capabilities`)
- Modify `webui/ui.js`, `webui/ui.css`
- Create `webui/tests/browser-capabilities.spec.mjs`
- Create `docs/plans/central-africa/evidence/M006-S03.md`

**Consumes:** nothing. **Produces:** the capability pattern Task 2 does not
need and Task 3 reuses for its notice styling.

**Steps**

1. Write the spec. On the WASM host (`?host=wasm`, the static-server
   pattern of `scene-isolation.spec.mjs`): `#noteMapBtn`, the capture-bars
   button, `#armBtn`, and the expanded lane's Channel and Duration sliders
   (`.slider-track[data-field="channel"]`, `[data-field="duration"]`) each
   have `aria-disabled="true"` and a non-empty `title` containing "browser".
   On the mock host (no `host` param), which stands in for the plugin in
   these specs, assert the same controls have no `aria-disabled`. And read
   `window.PolyPluginHost.capabilities` in the page (the script is loaded
   on every host) and assert all four new flags are `true`, so the plugin's
   own declaration is checked, not only the mock's.
2. Run it and watch the WASM cases fail.
3. Add to each host's `capabilities`: `canMapNotes`, `canCapture`,
   `canRouteChannels`, `canSetDuration`. `true` in `plugin-host.js` and
   `mock-host.js`, `false` in `wasm-host.js`, documented in `host-iface.js`.
4. In `ui.js`, add `function disableFor(el, reason)` that sets
   `aria-disabled="true"`, `title=reason`, a `disabled-reason` class, and
   ignores clicks and wheel events on that element. Apply it, when the
   capability is false, to Note Map ("Note Map: the browser plays its own
   sounds, so MIDI note mapping has no effect here"), the capture bars and
   Arm buttons ("Capture: recording MIDI needs the plugin in a DAW"), and the
   Channel and Duration sliders each time the expression pane renders
   ("Channel: the browser does not route MIDI channels", "Duration: the
   browser's sounds play their full length"). Style `.disabled-reason` with
   reduced opacity and `cursor: not-allowed`.
5. Run the spec and watch it pass. Mutate: set `canCapture: true` in the
   WASM host, watch the Arm case fail; restore. Record.
6. Run the three tokens. Write the evidence. Tick this task. Commit with
   `Rows: CA44`.

## Task 2 — Lane rename works in the browser

Rows: CA45.

**Files**

- Modify `webui/wasm-host.js`
- Modify `webui/tests/browser-capabilities.spec.mjs`
- Append to the evidence

**Steps**

1. Add a case on the WASM host: `PolyHost.action('setLaneName', {lane: 0,
   name: 'Bell'})`, then assert `getState().lanes[0].name === 'Bell'`;
   select scene B and back to A and assert it still reads "Bell"; and assert
   a 16-character name and an empty name are ignored (the mock host's
   1–15 rule).
2. Run it and watch it fail: the action is unknown.
3. Add `case 'setLaneName'` to `action()`: accept a string of 1–15
   characters, write it into the active scene's names array (`laneNamesA`
   or `laneNamesB`), then `activateLabelsForScene(state.scene, state.morph)`.
   Confirm `syncStateFromWasm` takes lane names from that array; if it reads
   them elsewhere, write there instead.
4. Run and watch it pass. Mutate: drop the length check, watch the
   oversized-name case fail; restore. Record.
5. Run the three tokens. Append the evidence. Tick this task. Commit with
   `Rows: CA45`.

## Task 3 — A failed WASM load is visible

Rows: CA46.

**Files**

- Modify `webui/index.html`, `webui/ui.js`, `webui/ui.css`
- Modify `webui/tests/browser-capabilities.spec.mjs`
- Append to the evidence; modify this plan and the ledger

**Steps**

1. Add a case: route `**/poly_engine.wasm` to abort, load `?host=wasm`, and
   assert a visible element `[data-wasm-fallback]` whose text says the
   engine did not load and that the editor is running a simplified preview.
   Assert no such element when the WASM loads.
2. Run it and watch it fail.
3. In `index.html`'s catch, set `window.__POLY_WASM_FAILED__ = e.message`
   before falling back. In `ui.js`, when that is set, insert at the top of the
   editor a `<div class="wasm-fallback" data-wasm-fallback role="status">`
   reading "The Poly engine did not load in this browser, so this is a
   simplified preview. It may not sound like the plugin." Style it in
   `ui.css` as a one-line warning bar.
4. Run and watch it pass. Mutate: remove the flag assignment, watch the case
   fail; restore. Record.
5. Run the three tokens. Tick the DoD boxes here and in the ledger, set
   CA44–CA46 and the slice to `done`, run `jk-standards ledger`. Append the
   evidence. Tick this task. Commit with `Rows: CA46`.

## Self-review

- **DoD.** Disabled with reason, enabled in plugin → Task 1. Rename →
  Task 2. Failed load visible → Task 3.
- **Rows.** CA44 → Task 1's spec, enabled in the mock host, whose
  capabilities are the plugin's; CA45 → Task 2 (rename works, decided at
  planning); CA46 → Task 3 (blocked `.wasm`).
- **Placeholders.** None.
- **Names.** `canMapNotes`, `canCapture`, `canRouteChannels`,
  `canSetDuration`, `disableFor`, `__POLY_WASM_FAILED__`,
  `data-wasm-fallback`, `browser-capabilities.spec.mjs`.
