import { readFileSync } from 'node:fs';

import { test, expect } from '@playwright/test';

import { parseProbeJsonl, probeHasNoteOn } from './lib/probe-assert';
import { EXPECTED_HIT_FILE, PROBE_OUTPUT } from './lib/toggle-contract';

// M042 S09 — post-quit probe assertion for the L4-web e2e.
//
// Runs AFTER Cubase quits: poly_midi_probe flushes its JSONL only on deactivate
// (setActive(false) in tools/midi_probe/source/probe_processor.cpp), so the
// probe file doesn't exist until quit. This spec reads the expected {pitch, ppq}
// that toggle-step.spec.ts recorded while Cubase was up and asserts the flushed
// probe contains that note-on — i.e. the toggled step actually fired through the
// real plugin in Cubase.
//
// No browser is used here; it's a pure file-read assertion under the Playwright
// runner so the whole e2e stays one toolchain. On the dev machine it only needs
// to parse — the probe/expected files exist only after a runner-gated run.

interface ExpectedHit {
  pitch: number;
  ppq: number;
  ppqTolerance?: number;
}

test.describe('L4-web: probe reflects the toggled step (post-quit)', () => {
  test('probe JSONL contains the toggled kick note-on', () => {
    const expected = JSON.parse(
      readFileSync(EXPECTED_HIT_FILE, 'utf-8'),
    ) as ExpectedHit;
    const jsonl = readFileSync(PROBE_OUTPUT, 'utf-8');
    const notes = parseProbeJsonl(jsonl, PROBE_OUTPUT);
    expect(
      probeHasNoteOn(notes, expected),
      `probe should contain a note-on for pitch ${expected.pitch} at ppq ` +
        `${expected.ppq} — the toggled step should have fired`,
    ).toBe(true);
  });
});
