import type {
  Fetcher,
  LoaderOptions,
  Manifest,
  SampleEntry,
  SampleLoader,
} from './types.js';

const DEFAULT_BASE_URL = '/samples/';

// region:pick-entry-for-note
// Exported so PolyPreviewCard and the sample-equivalence gate can compute the
// expected file for a given (note, preferredRole) without duplicating this
// algorithm. wasm-host.js mirrors this exact 2-step lookup so both surfaces
// pick the same file for the same lane. Any change here MUST be mirrored in
// webui/wasm-host.js::pickEntryForNote.
export function pickEntryForNote(
  manifest: Manifest,
  note: number,
  preferredRole?: string,
): SampleEntry | undefined {
  if (preferredRole) {
    const roleMatch = manifest.samples.find(
      (entry) =>
        entry.role === preferredRole && entry.midiNotes.includes(note),
    );
    if (roleMatch) return roleMatch;
  }
  return manifest.samples.find((entry) => entry.midiNotes.includes(note));
}
// endregion:pick-entry-for-note

function joinUrl(baseUrl: string, file: string): string {
  const trimmedBase = baseUrl.endsWith('/') ? baseUrl : `${baseUrl}/`;
  const trimmedFile = file.startsWith('/') ? file.slice(1) : file;
  return `${trimmedBase}${trimmedFile}`;
}

async function decodeFile(
  file: string,
  url: string,
  fetcher: Fetcher,
  context: LoaderOptions['context'],
  onDecoded: LoaderOptions['onDecoded'],
): Promise<AudioBuffer> {
  let bytes: ArrayBuffer;
  try {
    bytes = await fetcher(url);
  } catch (cause) {
    const message = cause instanceof Error ? cause.message : String(cause);
    throw new Error(`fetch failed for ${file}: ${message}`);
  }
  try {
    const buffer = await context.decodeAudioData(bytes);
    if (onDecoded) onDecoded(file);
    return buffer;
  } catch (cause) {
    const message = cause instanceof Error ? cause.message : String(cause);
    throw new Error(`decode failed for ${file}: ${message}`);
  }
}

export function createSampleLoader(options: LoaderOptions): SampleLoader {
  const {
    manifest,
    context,
    fetcher,
    baseUrl = DEFAULT_BASE_URL,
    onDecoded,
    preferredRoles,
  } = options;
  const bufferCache = new Map<string, Promise<AudioBuffer>>();

  function getBuffer(entry: SampleEntry): Promise<AudioBuffer> {
    const cached = bufferCache.get(entry.file);
    if (cached) return cached;
    const url = joinUrl(baseUrl, entry.file);
    const promise = decodeFile(entry.file, url, fetcher, context, onDecoded);
    bufferCache.set(entry.file, promise);
    promise.catch(() => {
      bufferCache.delete(entry.file);
    });
    return promise;
  }

  return {
    async loadNotes(midiNotes) {
      const results = new Map<number, AudioBuffer>();
      const unique = Array.from(new Set(midiNotes));
      await Promise.all(
        unique.map(async (note) => {
          const entry = pickEntryForNote(
            manifest,
            note,
            preferredRoles?.get(note),
          );
          if (!entry) {
            throw new Error(`no sample for MIDI note ${note}`);
          }
          const buffer = await getBuffer(entry);
          results.set(note, buffer);
        }),
      );
      return results;
    },
  };
}
