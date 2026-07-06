import type { LaneMeta, MidiEvent, Pattern } from './types.js';
import { euclid, lcm, rotArr } from './groove-math.ts';

interface LaneSpec {
  label: string;
  role: string;
  color?: string;
  note: number;
  steps: number;
  stepLen: number;
  hits: number;
  velocity: number;
  rot?: number;
}

function laneEvents(
  laneIdx: number,
  lane: LaneSpec,
  compositeTicks: number,
): MidiEvent[] {
  const pattern = rotArr(euclid(lane.hits, lane.steps), lane.rot ?? 0);
  const events: MidiEvent[] = [];
  for (let tick = 0; tick < compositeTicks; tick += lane.stepLen) {
    const stepIdx = ((tick / lane.stepLen) % lane.steps + lane.steps) % lane.steps;
    if (pattern[stepIdx]) {
      // One tick = one 8th note. Convert to quarter-note beats: beat = tick / 2.
      events.push({
        beat: tick / 2,
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

function buildReichPhaseProcess(): Pattern {
  // Mirrors mock-host case 27 (Reich Phase Process), but a 3-second static
  // preview cannot audibly demonstrate driftRate. Instead, lane 2 is rotated
  // by 3 eighth-note steps so it locks against lane 1 at a distinctive
  // polyrhythmic offset — the "arrived" phase state Reich phasing produces
  // over time. Deterministic; keeps E2E assertions cheap.
  const lanes: LaneSpec[] = [
    { label: 'Anchor pulse', role: 'woodblock', color: '#e07a3f', note: 76, steps: 12, stepLen: 1, hits: 5, velocity: 95 },
    { label: 'Drifting pulse', role: 'woodblock', color: '#d3a12b', note: 76, steps: 12, stepLen: 1, hits: 5, velocity: 90, rot: 3 },
    { label: 'Shimmer', role: 'hat', color: '#7fb37a', note: 42, steps: 4, stepLen: 2, hits: 4, velocity: 65 },
  ];

  // Composite loop = LCM of lane cycs (in tick units where 1 tick = 1 eighth).
  // 12 * 1 = 12, 4 * 2 = 8, LCM(12, 8) = 24 ticks = 12 quarter-note beats.
  const compositeTicks = lanes.reduce(
    (acc, l) => lcm(acc, l.steps * l.stepLen),
    1,
  );

  const events = lanes
    .flatMap((l, i) => laneEvents(i, l, compositeTicks))
    .sort((a, b) => a.beat - b.beat);

  return {
    bpm: 100,
    loopBeats: compositeTicks / 2,
    events,
    lanes: toLaneMeta(lanes),
  };
}

const PATTERNS: Record<string, () => Pattern> = {
  'Reich Phase Process': buildReichPhaseProcess,
};

export function getPatternForPreset(name: string): Pattern | null {
  const build = PATTERNS[name];
  return build ? build() : null;
}
