export type SpdxLicense =
  | 'CC0-1.0'
  | 'CC-BY-4.0'
  | 'CC-BY-3.0'
  | 'Artistic-2.0'
  | 'MIT';

export interface SampleEntry {
  file: string;
  spdx: SpdxLicense;
  source: string;
  midiNotes: number[];
  attribution?: string;
  sourceUrl?: string;
  role?: string;
  tags?: string[];
}

export interface Manifest {
  version: 1;
  samples: SampleEntry[];
}

export interface AudioContextLike {
  decodeAudioData(data: ArrayBuffer): Promise<AudioBuffer>;
}

export type Fetcher = (url: string) => Promise<ArrayBuffer>;

export interface LoaderOptions {
  manifest: Manifest;
  context: AudioContextLike;
  fetcher: Fetcher;
  baseUrl?: string;
  onDecoded?: (file: string) => void;
  // Per-note role hint used to disambiguate collisions in the shared sample
  // manifest — e.g. note 36 has both `cajon` and `kick` entries, note 43 has
  // both `darbuka` and `tom`. When the requested role matches an entry's
  // `role`, that entry wins over the first-found fallback. Site-only concern;
  // the plugin resolves samples independently.
  preferredRoles?: ReadonlyMap<number, string>;
}

export interface SampleLoader {
  loadNotes(midiNotes: number[]): Promise<Map<number, AudioBuffer>>;
}

export interface MidiEvent {
  beat: number;
  note: number;
  velocity: number;
  durationBeats?: number;
  lane?: number;
}

export interface LaneMeta {
  label: string;
  role: string;
  note: number;
  color?: string;
}

export interface Pattern {
  bpm: number;
  loopBeats: number;
  events: MidiEvent[];
  lanes?: LaneMeta[];
}

export interface SchedulerContext {
  readonly currentTime: number;
  readonly destination: AudioNode;
  createBufferSource(): AudioBufferSourceNode;
  createGain(): GainNode;
}

export interface SchedulerOptions {
  context: SchedulerContext;
  loader: SampleLoader;
  pattern: Pattern;
  lookAheadMs?: number;
  scheduleTickMs?: number;
  batchDurationSec?: number;
  onNoteScheduled?: (fireTime: number, event: MidiEvent) => void;
  // Optional override for the per-voice gain's destination. Defaults to
  // `context.destination`. Used by the site card runtime to interpose a
  // shared voice bus + AnalyserNode tap for the S11 T06 cross-surface RMS
  // parity check without changing scheduler internals.
  destination?: AudioNode;
}

export interface Scheduler {
  start(): Promise<void>;
  stop(): void;
  setLaneMuted(laneIndex: number, muted: boolean): void;
  readonly currentTime: number;
  readonly nodesStarted: number;
  readonly mutedLanes: ReadonlySet<number>;
}
