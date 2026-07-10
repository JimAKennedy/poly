// Engine-driven dump helper. Renders a preset over a fixed quarter-note window
// using a per-call scratch engine context, and returns SMF bytes + a
// post-applyPreset params snapshot. Called from PolyPreviewCard's ?dump=1 path
// so the Play dump is produced by the same C++ code that produced the Try It
// dump — they're byte-identical by construction, not by careful mirroring.
//
// Chunking (chunkQuarters = 4) mirrors webui/wasm-host.js::fireDumpTryIt so
// the engine advances through identical block boundaries on both surfaces —
// any bookkeeping the engine does per-render call sees identical inputs.

import type { EngineModule, EngineEvent } from './engine-host.ts';
import { readEngineEvents, EVENT_STRIDE } from './engine-host.ts';
import { writeSMF, TICKS_PER_QUARTER, type NormalizedEvent } from './smf-writer.ts';

const CHUNK_QUARTERS = 4;
const SAMPLE_RATE = 44100;
const BLOCK_SIZE = 512;
const MAX_ITERATIONS = 64;

const LaneFieldInt = {
  MidiNote: 0,
  MidiChannel: 1,
  HitCount: 2,
  Rotation: 3,
  BaseVelocity: 4,
  GhostFloor: 5,
  Active: 6,
  Subdivision: 7,
  CycleSteps: 8,
  KotekanSource: 9,
  Timeline: 10,
  EnvelopeCount: 11,
  CellCount: 12,
} as const;

const LaneFieldFloat = {
  Probability: 0,
  VelocitySpread: 1,
  HumanizeMs: 2,
  SwingAmount: 3,
  NoteDuration: 4,
  PhraseLength: 5,
  PhraseGap: 6,
  PhraseOffset: 7,
  MutationRate: 8,
  DriftRate: 9,
  TimingOffsetMs: 10,
  SyncopationOffset: 11,
  TempoMultiplier: 12,
  EmphasisProb: 13,
} as const;

export interface EngineParamsLane {
  laneIndex: number;
  noteNumber: number;
  roleLabel: string;
  role: string;
  cycleSteps: number;
  subdivision: number;
  stepLen: number;
  hits: number;
  rotation: number;
  velocity: number;
  probability: number;
  velocitySpread: number;
  humanizeMs: number;
  swing: number;
  noteDuration: number;
  driftRate: number;
  timingOffsetMs: number;
  mutationRate: number;
  phraseLength: number;
  phraseGap: number;
  phraseOffset: number;
  tempoMultiplier: number;
  emphasisProb: number;
  ghostFloor: number;
  active: boolean;
  kotekanSource: number;
  timeline: boolean;
}

export interface EngineParams {
  source: 'site-play' | 'try-it-engine';
  displayName: string;
  engineName: string;
  bpm: number;
  seed: number;
  macros: {
    complexity: number;
    density: number;
    syncopation: number;
    swing: number;
    tension: number;
    humanize: number;
  };
  lanes: EngineParamsLane[];
}

// Render `dumpBeats` quarter notes for `presetIndex` at `bpm` and return
// engine-emitted note-on/note-off events converted to NormalizedEvent[].
// Uses a fresh scratch context so the caller's shared playback engine
// (if any) is untouched.
export function renderEngineEvents(
  Module: EngineModule,
  presetIndex: number,
  bpm: number,
  dumpBeats: number,
  mutedLanes?: Iterable<number>,
): NormalizedEvent[] {
  const ctx = Module._poly_create();
  const events: NormalizedEvent[] = [];
  try {
    Module._poly_load_preset(ctx, presetIndex);
    // M043 S14 T03: fold UI mute state into the scratch so the dump reflects
    // what the user hears on Play, not the preset's untouched defaults. The
    // engine render loop skips inactive lanes (engine.cpp:359), so a
    // mute-then-dump produces zero note-ons for the muted lane's MIDI note.
    if (mutedLanes) {
      for (const laneIdx of mutedLanes) {
        Module._poly_edit_lane_int(ctx, laneIdx, LaneFieldInt.Active, 0);
      }
    }
    let ppq = 0.0;
    let iter = 0;
    while (ppq < dumpBeats && iter < MAX_ITERATIONS) {
      const end = Math.min(ppq + CHUNK_QUARTERS, dumpBeats);
      const jumped = iter === 0 ? 1 : 0;
      const count = Module._poly_render(
        ctx,
        ppq,
        end,
        bpm,
        SAMPLE_RATE,
        BLOCK_SIZE,
        1, // playing
        0, // looping
        0.0,
        0.0,
        jumped,
      );
      if (count > 0) appendNoteEvents(events, Module, ctx, count);
      ppq = end;
      iter++;
    }
  } finally {
    Module._poly_destroy(ctx);
  }
  return events;
}

function appendNoteEvents(
  out: NormalizedEvent[],
  Module: EngineModule,
  ctx: number,
  count: number,
): void {
  const evts = readEngineEvents(Module, ctx, count);
  for (const e of evts) {
    // Engine emits NoteEvent.velocity in [0, 1] (see engine/include/poly/types.h:38).
    // The C++ SMF writer multiplies by 127 before clamping (smf_writer.cpp:93);
    // mirror that here so JS-produced dumps match C++-produced dumps byte-for-byte.
    const onTick = Math.round(e.ppq * TICKS_PER_QUARTER);
    const offTick = Math.round((e.ppq + e.duration) * TICKS_PER_QUARTER);
    const note = clamp(e.note | 0, 0, 127);
    const vel = clamp(Math.round(e.velocity * 127), 0, 127);
    const ch = clamp(e.channel | 0, 0, 15);
    out.push({ tick: onTick, note, velocity: vel, channel: ch, isOn: true });
    out.push({ tick: offTick, note, velocity: 0, channel: ch, isOn: false });
  }
}

function clamp(v: number, lo: number, hi: number): number {
  return v < lo ? lo : v > hi ? hi : v;
}

export function dumpPresetAsSmf(
  Module: EngineModule,
  presetIndex: number,
  bpm: number,
  dumpBeats: number,
  mutedLanes?: Iterable<number>,
): Uint8Array {
  const events = renderEngineEvents(Module, presetIndex, bpm, dumpBeats, mutedLanes);
  return writeSMF(events, bpm);
}

// Snapshot the post-applyPreset engine state as a params.json payload. Called
// on the Play surface for parity with wasm-host.js::buildTryItParams. The
// laneMeta (roleLabel / role) comes from presets.json — passed in because the
// engine doesn't expose it directly.
export function buildEngineParams(
  Module: EngineModule,
  presetIndex: number,
  bpm: number,
  source: 'site-play' | 'try-it-engine',
  displayName: string,
  engineName: string,
  laneMeta: Array<{ roleLabel: string; role: string }>,
  mutedLanes?: Iterable<number>,
): EngineParams {
  const ctx = Module._poly_create();
  try {
    Module._poly_load_preset(ctx, presetIndex);
    if (mutedLanes) {
      for (const laneIdx of mutedLanes) {
        Module._poly_edit_lane_int(ctx, laneIdx, LaneFieldInt.Active, 0);
      }
    }
    const laneCount = Module._poly_active_lane_count(ctx);
    const lanes: EngineParamsLane[] = [];
    for (let i = 0; i < laneCount; i++) {
      const meta = laneMeta[i] ?? { roleLabel: '', role: '' };
      const subdivision = Module._poly_lane_int(ctx, i, LaneFieldInt.Subdivision);
      const stepLen = Math.round(8 / Math.max(1, subdivision));
      lanes.push({
        laneIndex: i,
        noteNumber: Module._poly_lane_int(ctx, i, LaneFieldInt.MidiNote),
        roleLabel: meta.roleLabel,
        role: meta.role,
        cycleSteps: Module._poly_lane_int(ctx, i, LaneFieldInt.CycleSteps),
        subdivision,
        stepLen,
        hits: Module._poly_lane_int(ctx, i, LaneFieldInt.HitCount),
        rotation: Module._poly_lane_int(ctx, i, LaneFieldInt.Rotation),
        velocity: Module._poly_lane_int(ctx, i, LaneFieldInt.BaseVelocity),
        probability: Module._poly_lane_float(ctx, i, LaneFieldFloat.Probability),
        velocitySpread: Module._poly_lane_float(ctx, i, LaneFieldFloat.VelocitySpread),
        humanizeMs: Module._poly_lane_float(ctx, i, LaneFieldFloat.HumanizeMs),
        swing: Module._poly_lane_float(ctx, i, LaneFieldFloat.SwingAmount),
        noteDuration: Module._poly_lane_float(ctx, i, LaneFieldFloat.NoteDuration),
        driftRate: Module._poly_lane_float(ctx, i, LaneFieldFloat.DriftRate),
        timingOffsetMs: Module._poly_lane_float(ctx, i, LaneFieldFloat.TimingOffsetMs),
        mutationRate: Module._poly_lane_float(ctx, i, LaneFieldFloat.MutationRate),
        phraseLength: Module._poly_lane_float(ctx, i, LaneFieldFloat.PhraseLength),
        phraseGap: Module._poly_lane_float(ctx, i, LaneFieldFloat.PhraseGap),
        phraseOffset: Module._poly_lane_float(ctx, i, LaneFieldFloat.PhraseOffset),
        tempoMultiplier: Module._poly_lane_float(ctx, i, LaneFieldFloat.TempoMultiplier),
        emphasisProb: Module._poly_lane_float(ctx, i, LaneFieldFloat.EmphasisProb),
        ghostFloor: Module._poly_lane_int(ctx, i, LaneFieldInt.GhostFloor),
        active: Module._poly_lane_int(ctx, i, LaneFieldInt.Active) !== 0,
        kotekanSource: Module._poly_lane_int(ctx, i, LaneFieldInt.KotekanSource),
        timeline: Module._poly_lane_int(ctx, i, LaneFieldInt.Timeline) !== 0,
      });
    }
    return {
      source,
      displayName,
      engineName,
      bpm,
      seed: Number(Module._poly_seed(ctx)),
      macros: {
        complexity: Module._poly_macro_value(ctx, 0),
        density: Module._poly_macro_value(ctx, 1),
        syncopation: Module._poly_macro_value(ctx, 2),
        swing: Module._poly_macro_value(ctx, 3),
        tension: Module._poly_macro_value(ctx, 4),
        humanize: Module._poly_macro_value(ctx, 5),
      },
      lanes,
    };
  } finally {
    Module._poly_destroy(ctx);
  }
}

// Exported so tests can reference the same magic numbers used inside the
// dump loop when constructing expectations.
export const ENGINE_DUMP_CONSTANTS = {
  CHUNK_QUARTERS,
  SAMPLE_RATE,
  BLOCK_SIZE,
  EVENT_STRIDE,
} as const;
