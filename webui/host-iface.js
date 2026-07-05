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
 *   //   'setFixedStep'    {lane, step, on}
 *   //   'setMicroTiming'  {lane, step, ms}
 *   //   'setEnvelope'     {lane, index, envelope | null}
 *   //   'selectScene'     {scene: 'A'|'B'}
 *   //   'applyPreset'     {index: -1|0..13}  (-1 = Init, 0-13 = factory preset)
 *   //   'togglePlay'      {}            (mock only; native transport is host-owned)
 *   //   'exportRequest'   {}
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
 *   presets: PresetInfo[]     // [{name, description}] — 14 factory presets
 * }
 * Lane = {
 *   name, role, note, ch, steps, stepLen, vel, prob, spread, ghost, push,
 *   hits, rot, timeline, fixed: number[]|null, pattern: number[],
 *   cells: number[]|null, mt: number[], envs: Env[], hue
 * }
 * Env = { target, period, depth, on }
 * Frame = {
 *   t8: number, playing: bool, convLeft: number,
 *   lanes: [{ ph: 0..1, step: int }]
 * }
 */
window.POLY_SCHEMA_VERSION = 1;
