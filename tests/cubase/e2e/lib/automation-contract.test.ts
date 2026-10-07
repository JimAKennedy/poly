import { test, expect } from '@playwright/test';

import {
  AutomationContractError,
  CHANGE_PPQ,
  automationFindings,
  requireComparable,
} from './automation-contract';
import { parseProbeJsonl } from './probe-assert';
import { splitPasses, type Pass } from './transport-contract';

// M004 S07 (DAW07) unit tests. No Cubase: the comparison is the logic that
// decides pass or fail, so it is proved here against synthetic captures in both
// directions. host-automation.spec.ts is the runner-gated half.

const on = (ppq: number, pitch: number) =>
  `{"type":"noteOn","ppq":${ppq.toFixed(6)},"pitch":${pitch},"velocity":0.700000,"channel":0}`;

/** One pass of the fixture's shape: a kick (36) every beat, a snare (38) on 1 and 3. */
function pass(kickUntil: number, opts: { snareAt?: number[]; tail?: boolean } = {}): string[] {
  const lines: string[] = [];
  for (let ppq = 0; ppq < 16; ppq++) {
    if (ppq < kickUntil) lines.push(on(ppq, 36));
    if ((opts.snareAt ?? [1, 3, 5, 7, 9, 11, 13, 15]).includes(ppq)) lines.push(on(ppq, 38));
  }
  // The driver plays half a second past the passage, so a note can land on 16.
  if (opts.tail ?? true) lines.push(on(16, 36));
  return lines;
}

function capture(...passes: string[][]): Pass[] {
  const text = passes.flat().join('\n') + '\n';
  return splitPasses(parseProbeJsonl(text, 'synthetic'));
}

test.describe('automationFindings', () => {
  test('a lane that switches the kick off at the change matches', () => {
    const passes = capture(pass(16), pass(CHANGE_PPQ), pass(CHANGE_PPQ));
    expect(automationFindings(passes[0], passes[2])).toEqual([]);
  });

  test('applied too early is caught, which "did anything change" would pass', () => {
    // The kick stops a beat before the change: the output DID change, after the
    // change it matches, and only the before-half of the comparison sees it.
    const passes = capture(pass(16), pass(CHANGE_PPQ - 1), pass(CHANGE_PPQ - 1));
    const findings = automationFindings(passes[0], passes[2]);
    expect(findings.map((f) => f.ppq)).toEqual([CHANGE_PPQ - 1]);
    expect(findings[0].why).toBe('lane silenced before the change');
  });

  test('a flat lane is caught: the kick never stops', () => {
    // The s07-flatten-lane red path: the lane exists and never changes.
    const passes = capture(pass(16), pass(16), pass(16));
    const findings = automationFindings(passes[0], passes[2]);
    expect(findings.length).toBe(16 - CHANGE_PPQ);
    expect(findings.every((f) => f.why === 'lane still sounding after the change')).toBe(true);
  });

  test('a change that leaks into another lane is caught', () => {
    const snareMissing = [1, 3, 5, 7, 9, 13, 15]; // ppq 11 dropped
    const passes = capture(pass(16), pass(CHANGE_PPQ), pass(CHANGE_PPQ, { snareAt: snareMissing }));
    const findings = automationFindings(passes[0], passes[2]);
    expect(findings.map((f) => [f.ppq, f.why])).toEqual([
      [11, 'a pitch other than the automated lane changed'],
    ]);
  });

  test('the tail past the passage is not compared', () => {
    const passes = capture(pass(16, { tail: true }), pass(CHANGE_PPQ), pass(CHANGE_PPQ, { tail: false }));
    expect(automationFindings(passes[0], passes[2])).toEqual([]);
  });
});

test.describe('requireComparable', () => {
  test('rejects a capture missing a pass', () => {
    expect(() => requireComparable(capture(pass(16), pass(CHANGE_PPQ)))).toThrow(AutomationContractError);
  });

  test('rejects a baseline whose lane was never sounding after the change', () => {
    const passes = capture(pass(CHANGE_PPQ, { tail: false }), pass(CHANGE_PPQ), pass(CHANGE_PPQ));
    expect(() => requireComparable(passes)).toThrow(/indistinguishable/);
  });

  test('accepts the three-pass shape', () => {
    expect(() => requireComparable(capture(pass(16), pass(CHANGE_PPQ), pass(CHANGE_PPQ)))).not.toThrow();
  });
});
