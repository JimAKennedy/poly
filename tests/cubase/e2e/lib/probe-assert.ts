// Pure probe-JSONL helpers for the S09 Playwright-over-CDP e2e (M042).
//
// The Playwright spec toggles a step in Poly's editor inside Cubase, plays the
// transport, and must then assert the poly_midi_probe JSONL reflects the change.
// The parsing + "does the probe contain the expected note" logic is kept here,
// free of Playwright and Cubase, so it can be unit-tested on the dev machine
// against synthetic JSONL (the runner-gated half is only the CDP attach +
// transport, not this arithmetic).
//
// Probe JSONL schema (source of truth: tools/midi_probe/source/probe_processor.cpp
// writeJsonl): one JSON object per line, keys
//   type ("noteOn" | "noteOff"), ppq (number), pitch (number),
//   velocity (number, VST3-normalized 0..1), channel (number).

export interface ProbeNote {
  type: 'noteOn' | 'noteOff';
  ppq: number;
  pitch: number;
  velocity: number;
  channel: number;
}

export class ProbeParseError extends Error {}

/**
 * Parse probe JSONL text into ProbeNote[]. Blank lines are skipped; any
 * non-blank line that is not valid JSON or is missing a field throws
 * ProbeParseError (a parse failure is distinct from a content assertion).
 */
export function parseProbeJsonl(text: string, source = 'probe'): ProbeNote[] {
  const notes: ProbeNote[] = [];
  const lines = text.split(/\r?\n/);
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i].trim();
    if (line === '') continue;
    let obj: unknown;
    try {
      obj = JSON.parse(line);
    } catch (err) {
      throw new ProbeParseError(`${source}:${i + 1}: invalid JSON: ${String(err)}`);
    }
    const rec = obj as Record<string, unknown>;
    const type = rec.type;
    if (type !== 'noteOn' && type !== 'noteOff') {
      throw new ProbeParseError(
        `${source}:${i + 1}: unexpected type ${JSON.stringify(type)}`,
      );
    }
    for (const key of ['ppq', 'pitch', 'velocity', 'channel'] as const) {
      if (typeof rec[key] !== 'number') {
        throw new ProbeParseError(`${source}:${i + 1}: missing/invalid field ${key}`);
      }
    }
    notes.push({
      type,
      ppq: rec.ppq as number,
      pitch: rec.pitch as number,
      velocity: rec.velocity as number,
      channel: rec.channel as number,
    });
  }
  return notes;
}

/** Keep only note-ons — the golden/assert surface compares onsets. */
export function noteOns(notes: ProbeNote[]): ProbeNote[] {
  return notes.filter((n) => n.type === 'noteOn');
}

export interface ExpectedHit {
  pitch: number;
  /** Musical position of the toggled step, in quarter-note (ppq) units. */
  ppq: number;
  /** Match tolerance on ppq; both sides derive from the same transport. */
  ppqTolerance?: number;
}

const DEFAULT_PPQ_TOLERANCE = 5e-4;

/**
 * Does the probe contain a note-on at the expected pitch within ppqTolerance of
 * the expected ppq? This is the "the toggled step fired" assertion: toggling a
 * kick step ON must add exactly such a note-on; toggling it OFF must remove it.
 */
export function probeHasNoteOn(notes: ProbeNote[], expected: ExpectedHit): boolean {
  const tol = expected.ppqTolerance ?? DEFAULT_PPQ_TOLERANCE;
  return noteOns(notes).some(
    (n) => n.pitch === expected.pitch && Math.abs(n.ppq - expected.ppq) <= tol,
  );
}

/**
 * Map a step index in a fixed-pattern (timeline) grid to its musical position.
 * The grid divides each bar into `stepsPerBar` equal steps; step 0 is bar-start.
 * Returns ppq (quarter-note units): stepIndex * (beatsPerBar / stepsPerBar).
 */
export function stepToPpq(
  stepIndex: number,
  stepsPerBar: number,
  beatsPerBar = 4,
): number {
  if (stepsPerBar <= 0) {
    throw new ProbeParseError(`stepsPerBar must be positive, got ${stepsPerBar}`);
  }
  return stepIndex * (beatsPerBar / stepsPerBar);
}
