import { test, expect } from '@playwright/test';

import { parseProbeJsonl } from './probe-assert';
import {
  TransportContractError,
  applyMutation,
  findDivergences,
  requireRepeatedVisits,
  splitPasses,
} from './transport-contract';

// M004 S03 (DAW03) unit tests. No Cubase: these prove the comparison decides
// both ways against synthetic probe captures, which is the half of the slice
// that can be proved without a DAW. The runner-gated half is
// transport-motion.spec.ts.

const on = (ppq: number, pitch: number) =>
  `{"type":"noteOn","ppq":${ppq.toFixed(6)},"pitch":${pitch},"velocity":0.700000,"channel":0}`;

// Two passes over the same bar: the transport played 0..2, located back, played
// 0..2 again. Identical pitches at identical PPQ -- the property holding.
// Pitches are deliberately NON-PERIODIC across the bar. With 36,38,36 a shift
// of one bar maps 36 onto 36 and the mutation aliases into looking correct --
// which is how the first version of the red-path case passed for the wrong
// reason.
const LOCATED_BACK = [
  on(0, 36), on(1, 38), on(2, 42),
  on(0, 36), on(1, 38), on(2, 42),
].join('\n') + '\n';

// The same session if phase were accumulated: the second pass is shifted late.
const ACCUMULATED = [
  on(0, 36), on(1, 38), on(2, 42),
  on(0, 38), on(1, 42), on(2, 36),
].join('\n') + '\n';

const LINEAR_ONLY = [on(0, 36), on(1, 38), on(2, 36), on(3, 42)].join('\n') + '\n';

test.describe('splitPasses', () => {
  test('cuts the stream where PPQ goes backwards', () => {
    const passes = splitPasses(parseProbeJsonl(LOCATED_BACK));
    expect(passes.length).toBe(2);
    expect(passes[0].startPpq).toBe(0);
    expect(passes[0].endPpq).toBe(2);
    expect(passes[1].startPpq).toBe(0);
  });

  test('a purely forward capture is one pass', () => {
    expect(splitPasses(parseProbeJsonl(LINEAR_ONLY)).length).toBe(1);
  });

  test('note-ons sharing a PPQ do not split a pass', () => {
    const together = [on(0, 36), on(0, 42), on(1, 38)].join('\n') + '\n';
    expect(splitPasses(parseProbeJsonl(together)).length).toBe(1);
  });
});

test.describe('findDivergences', () => {
  test('identical passes over the same range diverge nowhere', () => {
    expect(findDivergences(splitPasses(parseProbeJsonl(LOCATED_BACK)))).toEqual([]);
  });

  // A note shifted off its position leaves that position silent. The
  // comparison must report the empty side, not skip it.
  test('a position that goes silent in a later pass is reported', () => {
    const silent = [on(0, 36), on(1, 38), on(2, 42), on(0, 36), on(2, 42)].join('\n') + '\n';
    const d = findDivergences(splitPasses(parseProbeJsonl(silent)));
    expect(d.length).toBe(1);
    expect(d[0].ppq).toBe(1);
    expect(d[0].firstPitches).toEqual([38]);
    expect(d[0].laterPitches).toEqual([]);
  });

  test('a pass emitting different pitches at the same PPQ is reported', () => {
    const d = findDivergences(splitPasses(parseProbeJsonl(ACCUMULATED)));
    expect(d.length).toBe(3);
    expect(d[0].ppq).toBe(0);
    expect(d[0].firstPitches).toEqual([36]);
    expect(d[0].laterPitches).toEqual([38]);
  });

  // The honest short-pass case: a cycle that wraps early covers less ground.
  // That is not a divergence, and calling it one would fail a correct run.
  test('a later pass covering a shorter range is not a divergence', () => {
    const shortSecond =
      [on(0, 36), on(1, 38), on(2, 42), on(0, 36)].join('\n') + '\n';
    expect(findDivergences(splitPasses(parseProbeJsonl(shortSecond)))).toEqual([]);
  });

  test('a single pass has nothing to compare', () => {
    expect(findDivergences(splitPasses(parseProbeJsonl(LINEAR_ONLY)))).toEqual([]);
  });
});

test.describe('requireRepeatedVisits', () => {
  // The vacuity guard. A run where the transport never jumped yields no
  // divergences, and reporting that as a pass would be a green result from a
  // run that never exercised the property.
  test('a linear-only capture is an error, not a pass', () => {
    expect(() => requireRepeatedVisits(splitPasses(parseProbeJsonl(LINEAR_ONLY)))).toThrow(
      TransportContractError,
    );
  });

  test('passes that do not overlap in PPQ are an error', () => {
    const disjoint = [on(10, 36), on(11, 38), on(0, 36), on(1, 38)].join('\n') + '\n';
    const passes = splitPasses(parseProbeJsonl(disjoint));
    expect(passes.length).toBe(2);
    expect(() => requireRepeatedVisits(passes)).toThrow(/no PPQ appears in more than one/);
  });

  test('a located-back capture satisfies it', () => {
    expect(() => requireRepeatedVisits(splitPasses(parseProbeJsonl(LOCATED_BACK)))).not.toThrow();
  });
});

test.describe('applyMutation', () => {
  test('without the knob the capture is untouched', () => {
    const passes = splitPasses(parseProbeJsonl(LOCATED_BACK));
    expect(findDivergences(applyMutation(passes, undefined))).toEqual([]);
    expect(findDivergences(applyMutation(passes, 's02-malformed-preset'))).toEqual([]);
  });

  // The red path: a correct capture, shifted by the drift an accumulator would
  // have introduced, must now diverge.
  test('the knob makes a correct capture diverge', () => {
    const passes = splitPasses(parseProbeJsonl(LOCATED_BACK));
    const mutated = applyMutation(passes, 's03-accumulated-phase');
    const d = findDivergences(mutated);
    expect(d.length).toBeGreaterThan(0);
  });
});
