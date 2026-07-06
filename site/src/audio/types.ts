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
}

export interface SampleLoader {
  loadNotes(midiNotes: number[]): Promise<Map<number, AudioBuffer>>;
}
