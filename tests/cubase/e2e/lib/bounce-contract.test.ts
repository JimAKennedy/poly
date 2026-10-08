import { test, expect } from '@playwright/test';

import {
  BounceContractError,
  MUTATION_SHIFT_PPQ,
  applyMutation,
  bounceFindings,
  requireBothPasses,
} from './bounce-contract';
import { parseProbeJsonl } from './probe-assert';
import { splitPasses, type Pass } from './transport-contract';

// M004 S06 (DAW06) unit tests. No Cubase: the comparison is proved here in
// both directions against synthetic captures. offline-bounce.spec.ts is the
// runner-gated half.

const on = (ppq: number, pitch: number) =>
  `{"type":"noteOn","ppq":${ppq.toFixed(6)},"pitch":${pitch},"velocity":0.700000,"channel":0}`;

/** A pass of `bars` 4/4 bars: kick each beat, hat each 8th. */
function pass(bars: number, shift: (ppq: number, pitch: number) => number = (p) => p): string[] {
  const lines: string[] = [];
  for (let ppq = 0; ppq < bars * 4; ppq += 0.5) {
    if (Number.isInteger(ppq)) lines.push(on(shift(ppq, 36), 36));
    lines.push(on(shift(ppq, 42), 42));
  }
  // A real capture arrives in time order; a shifted note must not read as a
  // backward jump, which splitPasses would take for a locate.
  const at = (l: string) => Number(/"ppq":([0-9.]+)/.exec(l)![1]);
  return lines.sort((a, b) => at(a) - at(b));
}

function capture(...passes: string[][]): Pass[] {
  return splitPasses(parseProbeJsonl(passes.flat().join('\n') + '\n', 'synthetic'));
}

test.describe('bounceFindings', () => {
  test('a bounce that matches realtime over the passage has no findings', () => {
    // Realtime: four bars and a tail; offline: the eight-bar locator range.
    const [rt, off] = capture([...pass(4), on(16, 36)], pass(8));
    expect(bounceFindings(rt, off)).toEqual([]);
  });

  test('a note moved by a block edge is caught and named', () => {
    // 0.021 ppq: one 512-sample block at 48 kHz, 120 BPM.
    const [rt, off] = capture(pass(4), pass(8, (p, pitch) => (p === 6 && pitch === 36 ? 6.021 : p)));
    expect(bounceFindings(rt, off).map((f) => f.ppq)).toEqual([6, 6.021]);
  });

  test('a note dropped offline is caught', () => {
    const offline = pass(8).filter((l) => !l.includes('"ppq":9.000000,"pitch":36'));
    const [rt, off] = capture(pass(4), offline);
    expect(bounceFindings(rt, off)).toEqual([{ ppq: 9, realtime: [36, 42], offline: [42] }]);
  });
});

test.describe('requireBothPasses', () => {
  test('rejects a session where the export never reached the probe', () => {
    expect(() => requireBothPasses(capture(pass(4)))).toThrow(BounceContractError);
  });

  test('rejects a pass that stops short of the passage', () => {
    expect(() => requireBothPasses(capture(pass(4), pass(2)))).toThrow(/offline pass ends/);
  });

  test('accepts realtime plus offline', () => {
    expect(() => requireBothPasses(capture(pass(4), pass(8)))).not.toThrow();
  });
});

test.describe('applyMutation (s06-perturb-capture)', () => {
  test('is inert unless named', () => {
    const [, off] = capture(pass(4), pass(8));
    expect(applyMutation(off, undefined)).toBe(off);
  });

  test('turns an honest bounce red at the first position', () => {
    const [rt, off] = capture(pass(4), pass(8));
    const findings = bounceFindings(rt, applyMutation(off, 's06-perturb-capture'));
    expect(findings.map((f) => f.ppq)).toEqual([0, MUTATION_SHIFT_PPQ]);
  });
});
