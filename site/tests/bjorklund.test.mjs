import { test } from 'node:test';
import assert from 'node:assert/strict';

import { bjorklund, rotateRight } from '../src/audio/bjorklund.ts';
// Imported through the audio/playback surface to prove it re-exports the very
// same generator (no internal fork). See preset-patterns.ts.
import { bjorklund as presetPatternsBjorklund } from '../src/audio/preset-patterns.ts';

// Canon table — byte-for-byte the same spellings the engine pins in
// tests/euclidean_tests.cpp (ToussaintCanonSpellings). Each onset-index set is
// expanded to a boolean-per-step array so a regression on either side surfaces
// loudly. Rotation-0 forms are Toussaint's maximally-even distributions; the
// named rotations pin famous world rhythms to their conventional phase using
// the same right-shift convention as engine/src/euclidean.cpp.

// Build a length-n boolean pattern from a set of onset indices.
function fromOnsets(n, onsets) {
  const set = new Set(onsets);
  return Array.from({ length: n }, (_, i) => set.has(i));
}

test('bjorklund pins the engine rotation-0 canon (E(3,8) … E(7,12))', () => {
  assert.deepEqual(bjorklund(8, 3), fromOnsets(8, [0, 3, 6]), 'E(3,8) tresillo');
  assert.deepEqual(bjorklund(8, 5), fromOnsets(8, [0, 2, 3, 5, 6]), 'E(5,8) cinquillo');
  assert.deepEqual(bjorklund(16, 5), fromOnsets(16, [0, 3, 6, 9, 12]), 'E(5,16)');
  assert.deepEqual(bjorklund(7, 3), fromOnsets(7, [0, 2, 4]), 'E(3,7)');
  assert.deepEqual(bjorklund(9, 4), fromOnsets(9, [0, 2, 4, 6]), 'E(4,9)');
  assert.deepEqual(
    bjorklund(12, 7),
    fromOnsets(12, [0, 2, 3, 5, 7, 8, 10]),
    'E(7,12)',
  );
});

test('rotateRight pins the engine named-rotation canon', () => {
  // bossa-nova E(5,16) r10, rupak E(3,7) r3, Ewe bell E(7,12) r9 — identical to
  // the engine's onsetSet() expectations in ToussaintCanonSpellings.
  assert.deepEqual(
    rotateRight(bjorklund(16, 5), 10),
    fromOnsets(16, [0, 3, 6, 10, 13]),
    'bossa-nova E(5,16) r10',
  );
  assert.deepEqual(
    rotateRight(bjorklund(7, 3), 3),
    fromOnsets(7, [0, 3, 5]),
    'rupak E(3,7) r3',
  );
  assert.deepEqual(
    rotateRight(bjorklund(12, 7), 9),
    fromOnsets(12, [0, 2, 4, 5, 7, 9, 11]),
    'Ewe bell E(7,12) r9',
  );
});

test('rotateRight by n (and 0) is identity; preserves onset count', () => {
  const base = bjorklund(8, 3);
  assert.deepEqual(rotateRight(base, 0), base, 'r0 is identity');
  assert.deepEqual(rotateRight(base, 8), base, 'rotation by n returns original');
  for (let r = 0; r < 8; r++) {
    const rotated = rotateRight(base, r);
    const hits = rotated.filter(Boolean).length;
    assert.equal(hits, 3, `rotation ${r} preserves hit count`);
  }
});

// --- Boundary / degenerate inputs (mirror the engine's guards) ---
test('bjorklund saturates, empties, and clamps degenerate (k, n)', () => {
  assert.deepEqual(bjorklund(4, 4), [true, true, true, true], 'hits == steps → all onsets');
  assert.deepEqual(bjorklund(4, 10), [true, true, true, true], 'hits > steps → all onsets');
  assert.deepEqual(bjorklund(8, 0), Array(8).fill(false), 'zero hits → all rests');
  assert.deepEqual(bjorklund(8, -2), Array(8).fill(false), 'negative hits → all rests');
});

// --- Single-source-of-truth guard ---
test('preset-patterns re-exports the identical generator (no fork)', () => {
  assert.equal(
    presetPatternsBjorklund,
    bjorklund,
    'preset-patterns.ts must re-export the same bjorklund function object',
  );
});
