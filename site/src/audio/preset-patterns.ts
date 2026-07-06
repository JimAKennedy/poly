import type { MidiEvent, Pattern } from './types.js';
import { euclid, lcm, rotArr } from './groove-math.ts';

interface LaneSpec {
  note: number;
  steps: number;
  stepLen: number;
  hits: number;
  velocity: number;
  rot?: number;
}

function laneEvents(lane: LaneSpec, compositeTicks: number): MidiEvent[] {
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
      });
    }
  }
  return events;
}

function buildReichPhaseProcess(): Pattern {
  // Mirrors mock-host case 27 (Reich Phase Process).
  // Static preview: driftRate ignored — a 3-second demo can't audibly phase
  // and we want a deterministic pattern for E2E assertions.
  const lanes: LaneSpec[] = [
    { note: 76, steps: 12, stepLen: 1, hits: 5, velocity: 90 },
    { note: 76, steps: 12, stepLen: 1, hits: 5, velocity: 85 },
    { note: 42, steps: 4, stepLen: 2, hits: 4, velocity: 70 },
  ];

  // Composite loop = LCM of lane cycs (in tick units where 1 tick = 1 eighth).
  // 12 * 1 = 12, 4 * 2 = 8, LCM(12, 8) = 24 ticks = 12 quarter-note beats.
  const compositeTicks = lanes.reduce(
    (acc, l) => lcm(acc, l.steps * l.stepLen),
    1,
  );

  const events = lanes
    .flatMap((l) => laneEvents(l, compositeTicks))
    .sort((a, b) => a.beat - b.beat);

  return {
    bpm: 100,
    loopBeats: compositeTicks / 2,
    events,
  };
}

const PATTERNS: Record<string, () => Pattern> = {
  'Reich Phase Process': buildReichPhaseProcess,
};

export function getPatternForPreset(name: string): Pattern | null {
  const build = PATTERNS[name];
  return build ? build() : null;
}
