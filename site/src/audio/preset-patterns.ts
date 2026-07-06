import type { LaneMeta, MidiEvent, Pattern } from './types.js';
import { euclid, lcm, rotArr } from './groove-math.ts';

interface LaneSpec {
  label: string;
  role: string;
  color?: string;
  note: number;
  // Tick base is the 16th note: stepLen 4 = quarter, 2 = eighth, 1 = sixteenth.
  steps: number;
  stepLen: number;
  hits: number;
  velocity: number;
  rot?: number;
  // Steps per 4-beat bar; matches poly_engine driftRate. floor(barPos * drift)
  // adds to the cycle step index at each raw step, so the pattern rotates over
  // time and preview loop length extends to hold a full drift rotation.
  driftStepsPerBar?: number;
}

interface PatternSpec {
  bpm: number;
  lanes: LaneSpec[];
}

const TICKS_PER_BEAT = 4;
const TICKS_PER_BAR = TICKS_PER_BEAT * 4;
const MAX_DRIFT_BARS = 256;

// Smallest positive N such that floor(N * |drift|) is a positive multiple of
// steps. That's the number of 4-beat bars needed for drift to return the lane
// to its original rotation. Capped so an ill-chosen drift can't unroll forever.
function driftPeriodBars(steps: number, drift: number): number {
  if (drift === 0) return 1;
  const abs = Math.abs(drift);
  for (let n = 1; n <= MAX_DRIFT_BARS; n++) {
    const shift = Math.floor(n * abs);
    if (shift > 0 && shift % steps === 0) return n;
  }
  return MAX_DRIFT_BARS;
}

function laneEvents(
  laneIdx: number,
  lane: LaneSpec,
  totalTicks: number,
): MidiEvent[] {
  const hitsArr = euclid(lane.hits, lane.steps);
  const staticRot = lane.rot ?? 0;
  const drift = lane.driftStepsPerBar ?? 0;
  const steps = lane.steps;
  const events: MidiEvent[] = [];
  for (let tick = 0; tick < totalTicks; tick += lane.stepLen) {
    const rawStep = (tick / lane.stepLen) % steps;
    const driftSteps =
      drift === 0 ? 0 : Math.floor((tick / TICKS_PER_BAR) * drift);
    const idx = (((rawStep + staticRot + driftSteps) % steps) + steps) % steps;
    if (hitsArr[idx]) {
      events.push({
        beat: tick / TICKS_PER_BEAT,
        note: lane.note,
        velocity: lane.velocity,
        lane: laneIdx,
      });
    }
  }
  return events;
}

function toLaneMeta(specs: LaneSpec[]): LaneMeta[] {
  return specs.map((l) => ({
    label: l.label,
    role: l.role,
    note: l.note,
    color: l.color,
  }));
}

function buildFromSpec(spec: PatternSpec): Pattern {
  // Base composite: LCM of each lane's own step-cycle length in ticks.
  const baseCompositeTicks = spec.lanes.reduce(
    (acc, l) => lcm(acc, l.steps * l.stepLen),
    1,
  );
  // Drift composite: LCM of each drifting lane's rotation-return period, so
  // the composite loop lands on driftSteps=0 for every lane simultaneously.
  const driftCompositeTicks = spec.lanes.reduce((acc, l) => {
    const drift = l.driftStepsPerBar ?? 0;
    if (drift === 0) return acc;
    return lcm(acc, driftPeriodBars(l.steps, drift) * TICKS_PER_BAR);
  }, 1);
  const totalTicks = lcm(baseCompositeTicks, driftCompositeTicks);
  const events = spec.lanes
    .flatMap((l, i) => laneEvents(i, l, totalTicks))
    .sort((a, b) => a.beat - b.beat);
  return {
    bpm: spec.bpm,
    loopBeats: totalTicks / TICKS_PER_BEAT,
    events,
    lanes: toLaneMeta(spec.lanes),
  };
}

// Lane hue palette — distinct per role class so preview lane chips read cleanly.
const HUE = {
  kick: '#e07a3f',
  snare: '#d3a12b',
  hat: '#7fb37a',
  perc: '#8fb0d6',
  ghost: '#6a5c8c',
  bell: '#c4915d',
  clave: '#e0c26e',
} as const;

// Tick-length shortcuts in 16th-note units.
const QTR = 4;
const EIGHTH = 2;
const SIXTEENTH = 1;

// All appendix-preset parameter tables (steps/hits/rotation/subdivision/note/velocity)
// mirror site/src/content/docs/appendix-presets.mdx. Notes not covered by the sample
// manifest are remapped to the nearest covered percussion voice.
//   47 (Low Tom)         -> 45 (Low Floor Tom)
//   58 (Vibraslap)       -> 56 (Cowbell)  -- Reich Pattern B uses same voice as A
//   62 (Mute Hi Conga)   -> 76 (Hi Woodblock)  -- Kotekan Polos
//   64 (Low Conga)       -> 77 (Low Woodblock) -- Kotekan Sangsih
// Additive-cell rhythms ([2+2+3], [4+2+2]) are approximated as uniform Euclidean
// patterns over the total cell length (7 for aksak, 8 for adi tala).
const PATTERN_SPECS: Record<string, PatternSpec> = {
  // Preview for chapter 8 (minimalism). Lane 2 drifts against the anchor at
  // +2 steps/bar — an accelerated rate so a full 12-step rotation completes
  // every 6 bars (14.4 s at 100 BPM) and the composite loop passes through
  // all six phase relationships. The plugin uses +0.25 steps/bar as the
  // aesthetic reference (~2 minutes for a full rotation); the preview trades
  // faithfulness to that rate for something audible in a short listen.
  'Reich Phase Process': {
    bpm: 100,
    lanes: [
      { label: 'Anchor pulse',   role: 'woodblock', color: HUE.kick,  note: 76, steps: 12, stepLen: EIGHTH, hits: 5, velocity: 95 },
      { label: 'Drifting pulse', role: 'woodblock', color: HUE.snare, note: 76, steps: 12, stepLen: EIGHTH, hits: 5, velocity: 90, driftStepsPerBar: 2 },
      { label: 'Shimmer',        role: 'hat',       color: HUE.hat,   note: 42, steps: 4,  stepLen: QTR,    hits: 4, velocity: 65 },
    ],
  },

  'Factory: Four on the Floor': {
    bpm: 120,
    lanes: [
      { label: 'Kick',   role: 'kick',  color: HUE.kick,  note: 36, steps: 4,  stepLen: QTR,       hits: 4, velocity: 110 },
      { label: 'Snare',  role: 'snare', color: HUE.snare, note: 38, steps: 4,  stepLen: QTR,       hits: 2, velocity: 95, rot: 1 },
      { label: 'Hi-hat', role: 'hat',   color: HUE.hat,   note: 42, steps: 16, stepLen: SIXTEENTH, hits: 8, velocity: 70 },
      { label: 'Ghost',  role: 'hat',   color: HUE.ghost, note: 42, steps: 16, stepLen: SIXTEENTH, hits: 7, velocity: 45, rot: 3 },
    ],
  },

  'Factory: Polymetric Drift': {
    bpm: 110,
    lanes: [
      { label: 'Kick pulse',  role: 'kick', color: HUE.kick,  note: 36, steps: 3,  stepLen: QTR,    hits: 2, velocity: 105 },
      { label: 'Rim pattern', role: 'rim',  color: HUE.snare, note: 37, steps: 5,  stepLen: EIGHTH, hits: 3, velocity: 80,  rot: 1 },
      { label: 'Tom accent',  role: 'tom',  color: HUE.perc,  note: 45, steps: 7,  stepLen: EIGHTH, hits: 4, velocity: 85 },
      { label: 'Hat shimmer', role: 'hat',  color: HUE.hat,   note: 42, steps: 11, stepLen: EIGHTH, hits: 7, velocity: 60,  rot: 2 },
    ],
  },

  'Factory: Sparse Pulse': {
    bpm: 90,
    lanes: [
      { label: 'Kick',  role: 'kick',   color: HUE.kick,  note: 36, steps: 8,  stepLen: EIGHTH,    hits: 2, velocity: 95 },
      { label: 'Hat',   role: 'hat',    color: HUE.hat,   note: 42, steps: 8,  stepLen: EIGHTH,    hits: 3, velocity: 50, rot: 2 },
      { label: 'Ghost', role: 'shaker', color: HUE.ghost, note: 70, steps: 16, stepLen: SIXTEENTH, hits: 5, velocity: 35, rot: 1 },
    ],
  },

  'Factory: Breakbeat': {
    bpm: 140,
    lanes: [
      { label: 'Kick',  role: 'kick',  color: HUE.kick,  note: 36, steps: 16, stepLen: SIXTEENTH, hits: 3, velocity: 110 },
      { label: 'Snare', role: 'snare', color: HUE.snare, note: 38, steps: 8,  stepLen: EIGHTH,    hits: 2, velocity: 100, rot: 1 },
      { label: 'Hat',   role: 'hat',   color: HUE.hat,   note: 42, steps: 16, stepLen: SIXTEENTH, hits: 9, velocity: 65,  rot: 2 },
      { label: 'Ghost', role: 'tom',   color: HUE.ghost, note: 45, steps: 16, stepLen: SIXTEENTH, hits: 5, velocity: 45,  rot: 4 },
    ],
  },

  'Factory: Latin Feel': {
    bpm: 100,
    lanes: [
      { label: 'Clave',   role: 'clave',   color: HUE.clave, note: 75, steps: 8,  stepLen: EIGHTH,    hits: 3,  velocity: 100 },
      { label: 'Conga',   role: 'conga',   color: HUE.perc,  note: 63, steps: 16, stepLen: SIXTEENTH, hits: 7,  velocity: 80, rot: 2 },
      { label: 'Shaker',  role: 'shaker',  color: HUE.hat,   note: 70, steps: 16, stepLen: SIXTEENTH, hits: 11, velocity: 50 },
      { label: 'Cowbell', role: 'cowbell', color: HUE.bell,  note: 56, steps: 8,  stepLen: EIGHTH,    hits: 4,  velocity: 75, rot: 1 },
    ],
  },

  'Factory: Afro-House Phrases': {
    bpm: 122,
    lanes: [
      { label: 'Kick',   role: 'kick',    color: HUE.kick,  note: 36, steps: 4,  stepLen: QTR,       hits: 4, velocity: 110 },
      { label: 'Perc A', role: 'conga',   color: HUE.perc,  note: 63, steps: 12, stepLen: EIGHTH,    hits: 5, velocity: 80 },
      { label: 'Perc B', role: 'cowbell', color: HUE.bell,  note: 56, steps: 8,  stepLen: EIGHTH,    hits: 3, velocity: 75, rot: 1 },
      { label: 'Shaker', role: 'shaker',  color: HUE.hat,   note: 70, steps: 16, stepLen: SIXTEENTH, hits: 9, velocity: 50, rot: 2 },
      { label: 'Ghost',  role: 'hat',     color: HUE.ghost, note: 42, steps: 12, stepLen: EIGHTH,    hits: 7, velocity: 45, rot: 3 },
    ],
  },

  // Reich phasing needs identical voices on Pattern A and Pattern B; note 58
  // (Vibraslap) is not in the manifest so both lanes use 56 (Cowbell) and a
  // static rotation stands in for drift.
  'Factory: Reich Phasing': {
    bpm: 108,
    lanes: [
      { label: 'Pattern A',    role: 'cowbell', color: HUE.kick,  note: 56, steps: 12, stepLen: EIGHTH, hits: 5, velocity: 85 },
      { label: 'Pattern B',    role: 'cowbell', color: HUE.snare, note: 56, steps: 12, stepLen: EIGHTH, hits: 5, velocity: 80, rot: 2 },
      { label: 'Anchor pulse', role: 'kick',    color: HUE.bell,  note: 36, steps: 4,  stepLen: QTR,    hits: 2, velocity: 95 },
    ],
  },

  'Factory: Kotekan Interlock': {
    bpm: 120,
    lanes: [
      { label: 'Polos',      role: 'woodblock', color: HUE.perc,  note: 76, steps: 8,  stepLen: EIGHTH,    hits: 5, velocity: 85 },
      { label: 'Sangsih',    role: 'woodblock', color: HUE.snare, note: 77, steps: 8,  stepLen: EIGHTH,    hits: 3, velocity: 80, rot: 1 },
      { label: 'Gong cycle', role: 'kick',      color: HUE.kick,  note: 36, steps: 4,  stepLen: QTR,       hits: 3, velocity: 100 },
      { label: 'Shimmer',    role: 'shaker',    color: HUE.hat,   note: 70, steps: 16, stepLen: SIXTEENTH, hits: 9, velocity: 50, rot: 2 },
    ],
  },

  'Factory: Pocket Groove': {
    bpm: 92,
    lanes: [
      { label: 'Kick',  role: 'kick',  color: HUE.kick,  note: 36, steps: 8,  stepLen: EIGHTH,    hits: 3, velocity: 105 },
      { label: 'Snare', role: 'snare', color: HUE.snare, note: 38, steps: 8,  stepLen: EIGHTH,    hits: 2, velocity: 95, rot: 1 },
      { label: 'Hat',   role: 'hat',   color: HUE.hat,   note: 42, steps: 16, stepLen: SIXTEENTH, hits: 9, velocity: 60 },
      { label: 'Ghost', role: 'tom',   color: HUE.ghost, note: 45, steps: 16, stepLen: SIXTEENTH, hits: 7, velocity: 40, rot: 3 },
    ],
  },

  'Factory: Afrobeat 12/8': {
    bpm: 96,
    lanes: [
      { label: 'Bell timeline', role: 'cowbell', color: HUE.bell,  note: 56, steps: 12, stepLen: EIGHTH, hits: 7,  velocity: 90 },
      { label: 'Kick',          role: 'kick',    color: HUE.kick,  note: 36, steps: 4,  stepLen: QTR,    hits: 4,  velocity: 112 },
      { label: 'Snare',         role: 'snare',   color: HUE.snare, note: 38, steps: 3,  stepLen: EIGHTH, hits: 2,  velocity: 95 },
      { label: 'Shaker',        role: 'shaker',  color: HUE.hat,   note: 70, steps: 12, stepLen: EIGHTH, hits: 12, velocity: 55 },
      { label: 'Conga',         role: 'conga',   color: HUE.perc,  note: 63, steps: 5,  stepLen: EIGHTH, hits: 3,  velocity: 65 },
    ],
  },

  // Balkan aksak — 7/8 [2+2+3] rendered as steps=7 at 1/8. The uniform-step
  // engine can't do additive cells; total cell length preserves the 7/8 feel.
  'Factory: Balkan Aksak': {
    bpm: 140,
    lanes: [
      { label: 'Davul',   role: 'kick',      color: HUE.kick,  note: 36, steps: 7, stepLen: EIGHTH, hits: 2, velocity: 110 },
      { label: 'Rim',     role: 'rim',       color: HUE.snare, note: 37, steps: 7, stepLen: EIGHTH, hits: 1, velocity: 95 },
      { label: 'Zurna',   role: 'woodblock', color: HUE.bell,  note: 76, steps: 7, stepLen: EIGHTH, hits: 4, velocity: 80 },
      { label: 'Darbuka', role: 'tom',       color: HUE.perc,  note: 43, steps: 7, stepLen: EIGHTH, hits: 3, velocity: 60 },
    ],
  },

  'Factory: Bossa Nova': {
    bpm: 130,
    lanes: [
      { label: 'Surdo',            role: 'kick',      color: HUE.kick,  note: 36, steps: 4,  stepLen: QTR,       hits: 2,  velocity: 100 },
      { label: 'Tamborim (clave)', role: 'woodblock', color: HUE.clave, note: 76, steps: 16, stepLen: SIXTEENTH, hits: 5,  velocity: 85 },
      { label: 'Agogo',            role: 'agogo',     color: HUE.bell,  note: 67, steps: 8,  stepLen: EIGHTH,    hits: 5,  velocity: 70 },
      { label: 'Pandeiro',         role: 'shaker',    color: HUE.hat,   note: 70, steps: 16, stepLen: SIXTEENTH, hits: 12, velocity: 50 },
    ],
  },

  // Adi tala — 8-beat [4+2+2] rendered as steps=8 at 1/8.
  'Factory: Carnatic Tala': {
    bpm: 100,
    lanes: [
      { label: 'Mridangam bass',   role: 'kick', color: HUE.kick,  note: 36, steps: 8, stepLen: EIGHTH, hits: 3, velocity: 105 },
      { label: 'Mridangam treble', role: 'tom',  color: HUE.perc,  note: 43, steps: 8, stepLen: EIGHTH, hits: 5, velocity: 90 },
      { label: 'Ghatam',           role: 'hat',  color: HUE.hat,   note: 42, steps: 8, stepLen: EIGHTH, hits: 5, velocity: 65, rot: 2 },
      { label: 'Kanjira',          role: 'clap', color: HUE.ghost, note: 39, steps: 5, stepLen: EIGHTH, hits: 3, velocity: 50 },
    ],
  },

  'Factory: IDM Glitch': {
    bpm: 100,
    lanes: [
      { label: 'Kick',   role: 'kick',    color: HUE.kick,  note: 36, steps: 4,  stepLen: SIXTEENTH, hits: 3, velocity: 115 },
      { label: 'Snare',  role: 'snare',   color: HUE.snare, note: 38, steps: 5,  stepLen: SIXTEENTH, hits: 2, velocity: 100 },
      { label: 'Hat',    role: 'hat',     color: HUE.hat,   note: 42, steps: 9,  stepLen: SIXTEENTH, hits: 6, velocity: 70 },
      { label: 'Perc',   role: 'tom',     color: HUE.perc,  note: 45, steps: 7,  stepLen: SIXTEENTH, hits: 3, velocity: 55 },
      { label: 'Glitch', role: 'cowbell', color: HUE.ghost, note: 56, steps: 11, stepLen: SIXTEENTH, hits: 4, velocity: 75 },
    ],
  },
};

export function getPatternForPreset(name: string): Pattern | null {
  const spec = PATTERN_SPECS[name];
  return spec ? buildFromSpec(spec) : null;
}

export function listPresetNames(): string[] {
  return Object.keys(PATTERN_SPECS);
}
