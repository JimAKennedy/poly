import { readFileSync } from 'node:fs';

import { test, expect } from '@playwright/test';

import { parseProbeJsonl, noteOns } from './lib/probe-assert';
import { PROBE_OUTPUT } from './lib/toggle-contract';
import {
  applyMutation,
  findDivergences,
  requireRepeatedVisits,
  splitPasses,
} from './lib/transport-contract';

// M004 S03 (DAW03) — the transport moved, and what Poly emitted did not.
//
// Runs AFTER Cubase quits, like assert-probe.spec.ts: poly_midi_probe rewrites
// its JSONL from the accumulated event list, so the file holds every note the
// session emitted — including both sides of the driver's mid-playback locate
// (`play_scenario.py --locate-after`).
//
// The property: what Poly emits at a PPQ is a function of that PPQ, not of how
// the transport arrived there. Golden tests enforce determinism for linear
// playback only, and linear playback is the one case where a derivation and an
// accumulator agree — they diverge the moment the transport jumps. An
// accumulating lane replays the located range shifted by however long the
// transport had been rolling.
//
// No browser: a file-read assertion under the Playwright runner so the whole
// e2e stays one toolchain. On a dev machine it only needs to typecheck — the
// probe file exists only after a runner-gated run.

test.describe('L4: transport motion leaves PPQ-derived output unchanged (post-quit)', () => {
  test('the located-back pass matches the first pass at every shared position', () => {
    const jsonl = readFileSync(PROBE_OUTPUT, 'utf-8');
    const notes = noteOns(parseProbeJsonl(jsonl, PROBE_OUTPUT));
    expect(notes.length, `probe ${PROBE_OUTPUT} contains no note-ons`).toBeGreaterThan(0);

    const passes = applyMutation(splitPasses(notes), process.env.POLY_E2E_MUTATE);

    // Fails loudly if the transport never jumped. A run with one forward pass
    // has no repeated position, therefore no divergence, and would report green
    // while proving nothing — the vacuous-predicate failure this programme has
    // found more than once.
    requireRepeatedVisits(passes);

    const divergences = findDivergences(passes);
    const detail = divergences
      .map(
        (d) =>
          `  ppq ${d.ppq}: pass ${d.firstPass} emitted [${d.firstPitches.join(',')}], ` +
          `pass ${d.laterPass} emitted [${d.laterPitches.join(',')}]`,
      )
      .join('\n');
    expect(
      divergences,
      `${divergences.length} position(s) differ between transport passes — phase is not ` +
        `being derived from absolute PPQ:\n${detail}`,
    ).toEqual([]);
  });
});
