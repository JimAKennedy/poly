import { test, expect } from '@playwright/test';

import {
  ProbeParseError,
  parseProbeJsonl,
  noteOns,
  probeHasNoteOn,
  stepToPpq,
} from './probe-assert';

// Unit tests for the pure probe-assert helpers (M042 S09 T02). No browser, no
// Cubase — these run on the dev machine against synthetic probe JSONL. The
// runner-gated half (CDP attach + real transport) is in toggle-step.spec.ts.

// A synthetic probe capture: kick (36) on beats 0 and 2, plus a note-off that
// must be filtered by noteOns().
const PROBE_WITH_KICK =
  '{"type":"noteOn","ppq":0.000000,"pitch":36,"velocity":0.750000,"channel":0}\n' +
  '{"type":"noteOff","ppq":0.500000,"pitch":36,"velocity":0.000000,"channel":0}\n' +
  '{"type":"noteOn","ppq":2.000000,"pitch":36,"velocity":0.700000,"channel":0}\n';

// The same capture with the beat-1 (ppq=1.0) kick step toggled OFF — i.e. it is
// simply absent. Used to prove probeHasNoteOn distinguishes present vs. absent.
const PROBE_WITHOUT_STEP = PROBE_WITH_KICK;

test.describe('parseProbeJsonl', () => {
  test('parses note-ons and note-offs, blank lines skipped', () => {
    const notes = parseProbeJsonl(PROBE_WITH_KICK + '\n');
    expect(notes.length).toBe(3);
    expect(notes[0].type).toBe('noteOn');
    expect(notes[1].type).toBe('noteOff');
    expect(notes[0].pitch).toBe(36);
  });

  test('invalid JSON throws ProbeParseError', () => {
    expect(() => parseProbeJsonl('{not json}\n')).toThrow(ProbeParseError);
  });

  test('missing field throws ProbeParseError', () => {
    expect(() =>
      parseProbeJsonl('{"type":"noteOn","ppq":0.0,"pitch":36,"channel":0}\n'),
    ).toThrow(ProbeParseError);
  });

  test('unexpected type throws ProbeParseError', () => {
    expect(() =>
      parseProbeJsonl(
        '{"type":"controlChange","ppq":0.0,"pitch":1,"velocity":0.5,"channel":0}\n',
      ),
    ).toThrow(ProbeParseError);
  });
});

test.describe('noteOns', () => {
  test('filters out note-offs', () => {
    const ons = noteOns(parseProbeJsonl(PROBE_WITH_KICK));
    expect(ons.length).toBe(2);
    expect(ons.every((n) => n.type === 'noteOn')).toBe(true);
  });
});

test.describe('probeHasNoteOn', () => {
  test('finds a present note-on at the expected pitch and ppq', () => {
    const notes = parseProbeJsonl(PROBE_WITH_KICK);
    expect(probeHasNoteOn(notes, { pitch: 36, ppq: 2.0 })).toBe(true);
  });

  test('returns false for an absent step (toggled OFF)', () => {
    const notes = parseProbeJsonl(PROBE_WITHOUT_STEP);
    // No kick at ppq 1.0 in the synthetic capture.
    expect(probeHasNoteOn(notes, { pitch: 36, ppq: 1.0 })).toBe(false);
  });

  test('respects ppq tolerance', () => {
    const notes = parseProbeJsonl(PROBE_WITH_KICK);
    // 2.0002 is within the default 5e-4 tolerance of the 2.0 onset.
    expect(probeHasNoteOn(notes, { pitch: 36, ppq: 2.0002 })).toBe(true);
    // 2.01 is well beyond it.
    expect(probeHasNoteOn(notes, { pitch: 36, ppq: 2.01 })).toBe(false);
  });

  test('wrong pitch does not match', () => {
    const notes = parseProbeJsonl(PROBE_WITH_KICK);
    expect(probeHasNoteOn(notes, { pitch: 38, ppq: 0.0 })).toBe(false);
  });
});

test.describe('stepToPpq', () => {
  test('maps step index to ppq for a 16-step bar in 4/4', () => {
    // 16 steps/bar, 4 beats/bar => each step is 0.25 ppq.
    expect(stepToPpq(0, 16)).toBe(0);
    expect(stepToPpq(4, 16)).toBe(1.0);
    expect(stepToPpq(8, 16)).toBe(2.0);
  });

  test('non-positive stepsPerBar throws', () => {
    expect(() => stepToPpq(1, 0)).toThrow(ProbeParseError);
  });
});
