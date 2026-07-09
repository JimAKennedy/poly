// Engine-driven card scheduler. Feeds `_poly_render` in lookahead windows on a
// fresh scratch context and plays sample voices at engine-emitted times.
// Same Scheduler interface the card exposes to probes.
//
// Design choice: this scheduler uses its OWN scratch context, allocated on
// start() and destroyed on stop(). We don't share the engine-host shared ctx
// because playback state (RNG, phrase counters, drift) would leak between
// Play clicks. A per-playback ctx costs one _poly_create() call — well under
// a millisecond — and keeps every Play deterministic against its preset seed.

import type { EngineModule, EngineEvent } from './engine-host.ts';
import { readEngineEvents } from './engine-host.ts';
import type {
  MidiEvent,
  SampleLoader,
  Scheduler,
  SchedulerContext,
  SchedulerDumpMode,
} from './types.ts';

const CHUNK_QUARTERS = 1; // 1 quarter per _poly_render call — well under the
                          // engine's kMaxEventsPerBlock=256 cap for dense presets.
const DEFAULT_LOOK_AHEAD_MS = 140;
const DEFAULT_TICK_MS = 30;
const SAMPLE_RATE = 44100;
const BLOCK_SIZE = 512;

export interface EngineSchedulerOptions {
  Module: EngineModule;
  presetIndex: number;
  bpm: number;
  context: SchedulerContext;
  loader: SampleLoader;
  laneNotes: number[];
  destination?: AudioNode;
  lookAheadMs?: number;
  tickMs?: number;
  onNoteScheduled?: (fireTime: number, event: MidiEvent) => void;
  dumpMode?: SchedulerDumpMode;
}

export function createEngineScheduler(
  options: EngineSchedulerOptions,
): Scheduler {
  const {
    Module,
    presetIndex,
    bpm,
    context,
    loader,
    laneNotes,
    destination,
    lookAheadMs = DEFAULT_LOOK_AHEAD_MS,
    tickMs = DEFAULT_TICK_MS,
    onNoteScheduled,
  } = options;

  const output: AudioNode = destination ?? context.destination;
  const secPerBeat = 60 / bpm;

  let engineCtx = 0;
  let buffers = new Map<number, AudioBuffer>();
  const liveSources = new Set<AudioBufferSourceNode>();
  const sourcesByLane = new Map<number, Set<AudioBufferSourceNode>>();
  const mutedLanes = new Set<number>();

  let running = false;
  let startTime = 0;
  let nextPpq = 0.0;
  let renderIter = 0;
  let nodesStarted = 0;
  let tickTimer: ReturnType<typeof setTimeout> | null = null;

  function trackSource(laneIdx: number, source: AudioBufferSourceNode): void {
    if (laneIdx < 0) return;
    let set = sourcesByLane.get(laneIdx);
    if (!set) {
      set = new Set();
      sourcesByLane.set(laneIdx, set);
    }
    set.add(source);
  }

  function untrackSource(laneIdx: number, source: AudioBufferSourceNode): void {
    if (laneIdx < 0) return;
    const set = sourcesByLane.get(laneIdx);
    if (set) set.delete(source);
  }

  function playVoice(fireTime: number, evt: EngineEvent): void {
    const noteInt = evt.note | 0;
    const laneIdx = evt.laneIndex | 0;
    if (mutedLanes.has(laneIdx)) return;
    const buf = buffers.get(noteInt);
    if (!buf) {
      // Preload contract violation. Fail loudly — silently swallowing means
      // silence on stage, which is worse than a stack trace during dev.
      throw new Error(
        `engine-scheduler: no buffer loaded for MIDI note ${noteInt}`,
      );
    }
    const source = context.createBufferSource();
    source.buffer = buf;
    const gain = context.createGain();
    // Engine velocity is [0, 1] float. Match wasm-host.js voice staging
    // (unified with the site card): raw velocity, no per-role trim.
    gain.gain.value = Math.max(0, Math.min(1, evt.velocity));
    source.connect(gain);
    gain.connect(output);
    const t = Math.max(context.currentTime + 0.001, fireTime);
    source.start(t);
    source.onended = () => {
      liveSources.delete(source);
      untrackSource(laneIdx, source);
    };
    liveSources.add(source);
    trackSource(laneIdx, source);
    nodesStarted += 1;
    if (onNoteScheduled) {
      // Present the event as a MidiEvent for the audio probe. `beat` is the
      // engine's ppq position — used by the S10 gate to compute observed BPM.
      const event: MidiEvent = {
        beat: evt.ppq,
        note: noteInt,
        velocity: Math.max(0, Math.min(127, Math.round(evt.velocity * 127))),
        durationBeats: evt.duration,
        lane: laneIdx,
      };
      onNoteScheduled(fireTime, event);
    }
  }

  function stopSourcesForLane(laneIdx: number): void {
    const set = sourcesByLane.get(laneIdx);
    if (!set) return;
    for (const src of set) {
      try {
        src.stop();
      } catch {
        // already stopped
      }
      liveSources.delete(src);
    }
    set.clear();
  }

  function renderChunk(): void {
    if (engineCtx === 0) return;
    const end = nextPpq + CHUNK_QUARTERS;
    const jumped = renderIter === 0 ? 1 : 0;
    const count = Module._poly_render(
      engineCtx,
      nextPpq,
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
    if (count > 0) {
      const events = readEngineEvents(Module, engineCtx, count);
      for (const e of events) {
        const fireTime = startTime + e.ppq * secPerBeat;
        playVoice(fireTime, e);
      }
    }
    nextPpq = end;
    renderIter += 1;
  }

  function pump(): void {
    if (!running) return;
    const deadlinePpq =
      (context.currentTime - startTime + lookAheadMs / 1000) / secPerBeat;
    // Bound iterations so a wild deadline can't stall the tick.
    let safety = 0;
    while (running && nextPpq < deadlinePpq && safety++ < 256) {
      renderChunk();
    }
    tickTimer = setTimeout(pump, tickMs);
  }

  return {
    async start() {
      if (running) return;
      buffers = await loader.loadNotes(laneNotes);
      engineCtx = Module._poly_create();
      Module._poly_load_preset(engineCtx, presetIndex);
      running = true;
      startTime = context.currentTime + 0.05;
      nextPpq = 0.0;
      renderIter = 0;
      pump();
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
          // ignore
        }
      }
      liveSources.clear();
      sourcesByLane.clear();
      if (engineCtx !== 0) {
        Module._poly_destroy(engineCtx);
        engineCtx = 0;
      }
    },
    setLaneMuted(laneIndex, muted) {
      if (muted) {
        mutedLanes.add(laneIndex);
        stopSourcesForLane(laneIndex);
      } else {
        mutedLanes.delete(laneIndex);
      }
    },
    get currentTime() {
      return context.currentTime;
    },
    get nodesStarted() {
      return nodesStarted;
    },
    get mutedLanes() {
      return mutedLanes;
    },
  };
}
