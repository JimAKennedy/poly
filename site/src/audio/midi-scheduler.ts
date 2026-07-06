import type {
  MidiEvent,
  Pattern,
  Scheduler,
  SchedulerOptions,
} from './types.js';

const DEFAULT_LOOK_AHEAD_MS = 100;
const DEFAULT_SCHEDULE_TICK_MS = 25;

function uniqueNotes(events: MidiEvent[]): number[] {
  return Array.from(new Set(events.map((e) => e.note)));
}

function sortByBeat(events: MidiEvent[]): MidiEvent[] {
  return [...events].sort((a, b) => a.beat - b.beat);
}

export function createScheduler(options: SchedulerOptions): Scheduler {
  const {
    context,
    loader,
    pattern,
    lookAheadMs = DEFAULT_LOOK_AHEAD_MS,
    scheduleTickMs = DEFAULT_SCHEDULE_TICK_MS,
    batchDurationSec,
  } = options;

  const events = sortByBeat(pattern.events);
  const beatSec = 60 / pattern.bpm;
  const liveSources = new Set<AudioBufferSourceNode>();
  let buffers = new Map<number, AudioBuffer>();
  let running = false;
  let startTime = 0;
  let nextEventIdx = 0;
  let currentLoop = 0;
  let nodesStarted = 0;
  let tickTimer: ReturnType<typeof setTimeout> | null = null;

  function scheduleUpTo(untilTime: number): void {
    if (events.length === 0) return;
    // Bound loop iterations to avoid runaway if bpm/loopBeats is bogus.
    const maxIterations = Math.max(
      1024,
      Math.ceil((untilTime - startTime) / beatSec) * events.length + events.length,
    );
    let iterations = 0;
    while (iterations++ < maxIterations) {
      const event = events[nextEventIdx];
      const absBeat = event.beat + currentLoop * pattern.loopBeats;
      const fireTime = startTime + absBeat * beatSec;
      if (fireTime > untilTime) return;
      const buf = buffers.get(event.note);
      if (buf) {
        const source = context.createBufferSource();
        source.buffer = buf;
        const gain = context.createGain();
        gain.gain.value = Math.max(0, Math.min(1, event.velocity / 127));
        source.connect(gain);
        gain.connect(context.destination);
        source.start(fireTime);
        source.onended = () => {
          liveSources.delete(source);
        };
        liveSources.add(source);
        nodesStarted += 1;
      }
      nextEventIdx += 1;
      if (nextEventIdx >= events.length) {
        nextEventIdx = 0;
        currentLoop += 1;
      }
    }
  }

  function tick(): void {
    if (!running) return;
    scheduleUpTo(context.currentTime + lookAheadMs / 1000);
    tickTimer = setTimeout(tick, scheduleTickMs);
  }

  return {
    async start() {
      if (running) return;
      buffers = await loader.loadNotes(uniqueNotes(events));
      running = true;
      startTime = context.currentTime;
      nextEventIdx = 0;
      currentLoop = 0;
      if (typeof batchDurationSec === 'number') {
        scheduleUpTo(startTime + batchDurationSec);
      } else {
        tick();
      }
    },
    stop() {
      running = false;
      if (tickTimer !== null) {
        clearTimeout(tickTimer);
        tickTimer = null;
      }
      for (const src of liveSources) {
        try {
          src.stop();
        } catch {
          // already stopped or not yet started; ignore
        }
      }
      liveSources.clear();
    },
    get currentTime() {
      return context.currentTime;
    },
    get nodesStarted() {
      return nodesStarted;
    },
  };
}

export type { Pattern, MidiEvent, Scheduler, SchedulerOptions };
