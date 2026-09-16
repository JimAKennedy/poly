import { readFileSync } from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

// M004 S02 (DAW02) — the expectation the in-Cubase preset sweep compares
// against, read from the generated preset data rather than restated here.
//
// 31 of the 45 factory presets have never been selected inside a DAW.
// kWebPresetLaneNames was once initialised sparsely — 14 rows against a
// kFactoryPresetCount-sized extent — and a null entry there crashed Cubase on
// preset change. The null guard at the applyPreset call site is the only thing
// between that table and the same crash, and nothing exercises it per index.

const HERE = path.dirname(fileURLToPath(import.meta.url));
const PRESETS_JSON = path.resolve(HERE, '../../../../site/src/generated/presets.json');

export interface PresetExpectation {
  index: number;
  name: string;
  laneCount: number;
  noteNumbers: number[];
}

export class PresetContractError extends Error {}

/**
 * Read the generated preset data into one expectation per preset.
 *
 * Throws rather than returning a partial list: a contract built from a file
 * that silently changed shape is worse than no contract, because the sweep
 * would then compare against nothing and report success.
 */
export function loadPresetExpectations(file: string = PRESETS_JSON): PresetExpectation[] {
  let raw: string;
  try {
    raw = readFileSync(file, 'utf8');
  } catch (err) {
    throw new PresetContractError(`cannot read ${file}: ${(err as Error).message}`);
  }

  let parsed: unknown;
  try {
    parsed = JSON.parse(raw);
  } catch (err) {
    throw new PresetContractError(`${file} is not valid JSON: ${(err as Error).message}`);
  }

  const doc = parsed as { schemaVersion?: unknown; presets?: unknown };
  if (typeof doc.schemaVersion !== 'number')
    throw new PresetContractError('presets.json carries no numeric schemaVersion');
  if (!Array.isArray(doc.presets) || doc.presets.length === 0)
    throw new PresetContractError('presets.json carries no presets array');

  return doc.presets.map((entry, i) => {
    const p = entry as { name?: unknown; lanes?: unknown };
    if (typeof p.name !== 'string' || p.name.length === 0)
      throw new PresetContractError(`preset ${i} has no name`);
    if (!Array.isArray(p.lanes) || p.lanes.length === 0)
      throw new PresetContractError(`preset ${i} (${p.name}) has no lanes`);

    const noteNumbers = p.lanes.map((laneEntry, laneIndex) => {
      const lane = laneEntry as { noteNumber?: unknown };
      if (typeof lane.noteNumber !== 'number')
        throw new PresetContractError(`preset ${i} (${p.name}) lane ${laneIndex} has no noteNumber`);
      return lane.noteNumber;
    });

    return { index: i, name: p.name, laneCount: p.lanes.length, noteNumbers };
  });
}

export interface ObservedPreset {
  index: number;
  laneCount: number;
  noteNumbers: number[];
}

/**
 * Compare what Cubase showed against what the contract expects.
 *
 * Returns every mismatch rather than the first. A sweep that costs a Cubase
 * launch to produce should report everything it found, not stop at index 3 and
 * leave the remaining 41 unknown.
 */
export function comparePresets(
  expected: PresetExpectation[],
  observed: ObservedPreset[],
): string[] {
  const problems: string[] = [];
  const byIndex = new Map(observed.map((o) => [o.index, o]));

  for (const want of expected) {
    const got = byIndex.get(want.index);
    if (!got) {
      problems.push(`preset ${want.index} (${want.name}) was never observed`);
      continue;
    }
    if (got.laneCount !== want.laneCount) {
      problems.push(
        `preset ${want.index} (${want.name}): ${got.laneCount} lanes in the host, ` +
          `${want.laneCount} in presets.json`,
      );
      continue;
    }
    const wantNotes = want.noteNumbers.join(',');
    const gotNotes = got.noteNumbers.join(',');
    if (wantNotes !== gotNotes) {
      problems.push(
        `preset ${want.index} (${want.name}): notes [${gotNotes}] in the host, ` +
          `[${wantNotes}] in presets.json`,
      );
    }
  }
  return problems;
}

/**
 * The red path, per M004-decisions.md: POLY_E2E_MUTATE=s02-malformed-preset
 * makes the contract claim a wrong lane count for one index, so the comparison
 * must fail naming it.
 *
 * The perturbation is applied to the EXPECTATION, never to the plugin. A knob
 * that changes what Poly does under an environment variable is a knob that can
 * be set in production.
 */
export function applyMutation(
  expected: PresetExpectation[],
  mutate: string | undefined,
): PresetExpectation[] {
  if (mutate !== 's02-malformed-preset') return expected;
  if (expected.length === 0) return expected;
  const target = Math.min(7, expected.length - 1);
  return expected.map((p) =>
    p.index === target ? { ...p, laneCount: p.laneCount + 1 } : p,
  );
}
