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
 *   //   'fitMidi'         {lane, bytes: number[]}  (M035 S02: drop a .mid onto
 *   //                                    a lane -> reverse-Euclid import. bytes
 *   //                                    is the raw file content as 0..255
 *   //                                    values; the host parses+fits+applies
 *   //                                    (mock JS / wasm poly_import_midi /
 *   //                                    native fitMidi). A non-MIDI / unparse-
 *   //                                    able / degenerate drop leaves the lane
 *   //                                    untouched.)
 *   //   'revertImport'    {lane}          (M035 S03: back out of a fitMidi import
 *   //                                    — restore the lane to its exact pre-import
 *   //                                    params from the snapshot armed by the
 *   //                                    matching fitMidi (mock JS deep-copy / wasm
 *   //                                    poly_revert_import / native scene snapshot).
 *   //                                    {lane}-only; no LaneConfig crosses the
 *   //                                    bridge. No armed snapshot -> warn + no-op.)
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
 *   //   'manualFill'      {}            (M034 S01: pulse the global momentary
 *   //                                    fill trigger; native edges
 *   //                                    kFillManualTrigger for one fill render
 *   //                                    pass, independent of lane.N.fillEveryN)
 *   //   'exportRequest'   {}
 *   //   'exportSaveAs'    {lane?}       (canExport-gated, plugin only: offline-render
 *   //                                    the current pattern to SMF and open the
 *   //                                    native Save-As panel. Optional {lane:N}
 *   //                                    (M032 S02) exports only lane N as one named
 *   //                                    track; absent/negative = all lanes.)
 *   //   'beginMidiDrag'   {lane?}       (G06: open native drag-source window for
 *   //                                    drag-to-DAW; canExport-gated, plugin only.
 *   //                                    M053 S11 removed the capState gate — fires
 *   //                                    in every capture state. Optional {lane:N}
 *   //                                    (M032 S02) drags only that lane.)
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
 *
 *   // --- emission stream (M073: what the engine actually played) ---
 *   // Per-lane ordered ring of classified emissions for the desk overlay +
 *   // played timeline. base/ghost/add/drop with grid ppq and post-timing-shift
 *   // onset. [] for an out-of-range lane, before the first frame, or when the
 *   // host carries no stream (degrade to positional-pattern-only).
 *   getLaneEmissions(li: number): Emission[]
 * }
 * Emission = { ppq: number, shiftedPpq: number, step: number,
 *              kind: 'base'|'ghost'|'add'|'drop' }
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
 *   cells: number[]|null, mt: number[], envs: Env[], hue,
 *   fillEveryN: number,      // M034 S01: bars between auto-fills (0 = off)
 *   seedLocked: bool         // M034 S03: lane pinned to its captured seed; a
 *                            //   global seed reroll leaves it unchanged. The
 *                            //   'lane.N.seedLock' edit (0/1) flips it; native
 *                            //   captures the current global seed into laneSeed
 *                            //   on the false->true edge.
 * }
 * Env = { target, period, depth, on }
 * Frame = {
 *   t8: number, playing: bool, convLeft: number,
 *   // M073: each lane carries its ordered emission stream (native: drained from
 *   // the UISnapshot rings; kind is the int EmissionKind mapped to a label by
 *   // plugin-host.js). Absent on legacy hosts → getLaneEmissions returns [].
 *   lanes: [{ ph: 0..1, step: int, emissions?: Emission[] }],
 *   // M051 S08 — capture state machine (native: UISnapshot atomics):
 *   capState?: 0|1|2|3,   // idle | armed | capturing | complete
 *   capBars?: number,     // bar-window length N (default 8)
 *   capProg?: number      // capture progress in bars, 0..N (float)
 * }
 */
window.POLY_SCHEMA_VERSION = 1;
