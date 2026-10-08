import { test, expect } from '@playwright/test';

import { asymmetry } from './instance-contract';
import { parseProbeJsonl } from './probe-assert';
import { splitPasses, type Pass } from './transport-contract';

// M004 S05 (DAW05) unit tests. No Cubase: proves the asymmetry check decides
// both ways. multi-instance.spec.ts is the runner-gated half.

const on = (ppq: number, pitch: number) =>
  `{"type":"noteOn","ppq":${ppq.toFixed(6)},"pitch":${pitch},"velocity":0.700000,"channel":0}`;

/** One pass, in time order as a real capture arrives -- unsorted would read as a locate. */
function pass(notes: Array<[number, number]>): Pass {
  const sorted = [...notes].sort((x, y) => x[0] - y[0]);
  return splitPasses(parseProbeJsonl(sorted.map(([ppq, p]) => on(ppq, p)).join('\n') + '\n', 'synthetic'))[0];
}

const defaultPatch = pass([[0, 36], [0, 42], [1, 38], [1, 42], [2, 36], [2, 42]]);
const reichPhasing = pass([[0, 76], [0.5, 42], [1, 76], [2, 78]]);

test('two instances on different patches each have notes of their own', () => {
  const a = asymmetry(defaultPatch, reichPhasing);
  expect(a.onlyFirst).toBeGreaterThan(0);
  expect(a.onlySecond).toBeGreaterThan(0);
});

test('shared state -- identical captures -- has nothing of its own on either side', () => {
  expect(asymmetry(defaultPatch, defaultPatch)).toEqual({ onlyFirst: 0, onlySecond: 0 });
});

test('a capture holding both instances notes is caught by the other side being empty', () => {
  // One probe sees its own Poly plus the other's: a strict superset.
  const both = pass([[0, 36], [0, 42], [1, 38], [1, 42], [2, 36], [2, 42], [0, 76], [1, 76], [2, 78], [0.5, 42]]);
  const a = asymmetry(both, reichPhasing);
  expect(a.onlyFirst).toBeGreaterThan(0);
  expect(a.onlySecond).toBe(0);
});

test('a shared pitch at the same position is not counted as either side own', () => {
  const a = asymmetry(pass([[0, 42]]), pass([[0, 42], [1, 76]]));
  expect(a).toEqual({ onlyFirst: 0, onlySecond: 1 });
});
