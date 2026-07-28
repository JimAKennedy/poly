'use strict';
/**
 * Poly web UI — host interface contract.
 *
 * The UI (ui.js) talks ONLY to `window.PolyHost`, which implements this
 * surface. Two implementations exist:
 *   - mock-host.js   : browser-standalone; owns a groove model + WebAudio
 *                      preview voices. Used for development, docs embeds,
 *                      and Playwright CI.
 *   - plugin-host.js : bridges to the native plugin over the webview
 *                      message channel. The C++ side owns all truth.
 *
 * See bridge-schema.md for the wire format used by plugin-host.
 *
 * interface PolyHost {
 *   schemaVersion: 1
 *
 *   // --- state (pull + subscribe) ---
 *   getState(): State                    // synchronous snapshot
 *   onState(cb: (State) => void): void   // full-state pushes (preset load,
 *                                        // scene switch, setComponentState)
 *
 *   // --- edits ---
 *   // Continuous parameters carry automation gestures:
 *   edit(paramId: string, value: number, gesture: 'begin'|'perform'|'end')
 *   // Structural edits that have no single-parameter representation:
 *   action(name: string, payload: object): void
 *   //   'toggleStep'      {lane, step}
 *   //   'setEuclid'       {lane, steps?, hits?, rotation?}
 *   //   'setCells'        {lane, cells: number[] | null}
 *   //   'setLaneName'     {lane, name}    (1..15 chars; empty/oversized dropped)
 *   //   'setFixedStep'    {lane, step, on}
 *   //   'setMicroTiming'  {lane, step, ms}
 *   //   'setEnvelope'     {lane, index, envelope | null}
 *   //   'selectScene'     {scene: 'A'|'B'}
 *   //   'applyPreset'     {index: -1|0..N-1}  (-1 = Init, 0..N-1 = factory preset,
 *   //                                          N = state.presets.length;
 *   //                                          source: site/src/generated/presets.json)
 *   //   'togglePlay'      {}            (mock only; native transport is host-owned)
 *   //   'exportRequest'   {}
 *   //   'armCapture'      {}            (M051 S08: arm the capture state machine)
 *   //   'resetCapture'    {}            (M051 S08: reset to idle from any state)
 *   //   'setCaptureBars'  {bars}        (M051 S08: bar window length, bound to
 *   //                                    kCaptureLength; {4,8,16,32})
 *
 *   // --- capabilities ---
 *   capabilities: {
 *     canExport: boolean,          // true only in plugin mode (MIDI/SMF export)
 *   }
 *
 *   // --- feedback (~30-60 Hz visual frame; never authoritative) ---
 *   onFrame(cb: (Frame) => void): void
 * }
 *
 * State = {
 *   preset: string, seed: number, tempo: number,
 *   scene: 'A'|'B'|'Morph', morph: number,
 *   macros: { complexity, density, syncopation, swing, tension, humanize },
 *   lanes: Lane[],           // length = active lanes
 *   presets: PresetInfo[]     // [{name, description}] — factory presets from
 *                             //   site/src/generated/presets.json.
 *                             //   applyPreset index runs 0..presets.length-1.
 * }
 * Lane = {
 *   name, role, note, ch, steps, stepLen, vel, prob, spread, ghost, push,
 *   hits, rot, timeline, fixed: number[]|null, pattern: number[],
 *   cells: number[]|null, mt: number[], envs: Env[], hue
 * }
 * Env = { target, period, depth, on }
 * Frame = {
 *   t8: number, playing: bool, convLeft: number,
 *   lanes: [{ ph: 0..1, step: int }],
 *   // M051 S08 — capture state machine (native: UISnapshot atomics):
 *   capState?: 0|1|2|3,   // idle | armed | capturing | complete
 *   capBars?: number,     // bar-window length N (default 8)
 *   capProg?: number      // capture progress in bars, 0..N (float)
 * }
 */
window.POLY_SCHEMA_VERSION = 1;
