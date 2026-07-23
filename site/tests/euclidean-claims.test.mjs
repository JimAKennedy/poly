import { test } from 'node:test';
import assert from 'node:assert/strict';

import {
  bjorklund,
  parsePattern,
  onsetCount,
  gapSequence,
  rotate,
  findEuclideanRotation,
  findAnyEuclideanFit,
  parseGrouping,
} from '../src/lib/euclidean-claims.mjs';

// Known-truth vectors from Toussaint (2004) and the 2026-07-16 review D1/D2/D3
// corrections. Any regression here is an anti-D1-style bug.

test('bjorklund E(3,8) tresillo', () => {
  const p = bjorklund(8, 3);
  assert.deepEqual(p, [true, false, false, true, false, false, true, false]);
  assert.deepEqual(gapSequence(p), [3, 3, 2]);
});

test('bjorklund E(5,16) canonical is 3-3-3-3-4 (D2 fixture)', () => {
  // The appendix's bossa nova row prints the same rhythm at a different rotation
  // (gaps 3-3-4-3-3, starting at onset index 0 of that rotation). Both are
  // legitimate Euclidean patterns — findEuclideanRotation() covers the equivalence.
  const p = bjorklund(16, 5);
  assert.deepEqual(gapSequence(p), [3, 3, 3, 3, 4]);
  // Sanity: the appendix pattern IS a rotation of canonical E(5,16).
  const bossa = parsePattern('x . . x . . x . . . x . . x . .');
  assert.notEqual(findEuclideanRotation(bossa, 5, 16), null);
  // And its gap sequence is the 3-3-4-3-3 the appendix claims.
  assert.deepEqual(gapSequence(bossa), [3, 3, 4, 3, 3]);
});

test('bjorklund E(4,9) daichovo (D5 fixture — canonical 2-2-2-3)', () => {
  const p = bjorklund(9, 4);
  assert.deepEqual(gapSequence(p), [2, 2, 2, 3]);
});

test('parsePattern accepts x-dot form with any whitespace', () => {
  assert.deepEqual(parsePattern('x . x .'), [true, false, true, false]);
  assert.deepEqual(parsePattern('xx..'), [true, true, false, false]);
  assert.deepEqual(parsePattern('x x x . x x . x . x x .'), [
    true, true, true, false, true, true, false, true, false, true, true, false,
  ]);
});

test('parsePattern rejects unknown characters', () => {
  assert.throws(() => parsePattern('x . y .'), /unexpected character 'y'/);
});

test('onsetCount tresillo', () => {
  assert.equal(onsetCount(parsePattern('x . . x . . x .')), 3);
});

test('gapSequence wraps from last onset back to first', () => {
  // "x . x . x" — onsets at 0, 2, 4 in length 5. Gaps: 2, 2, 1.
  assert.deepEqual(gapSequence(parsePattern('x . x . x')), [2, 2, 1]);
});

test('rotate shifts pattern right (positive) or left (negative)', () => {
  const p = [true, false, false, false];
  assert.deepEqual(rotate(p, 1), [false, true, false, false]);
  assert.deepEqual(rotate(p, -1), [false, false, false, true]);
  assert.deepEqual(rotate(p, 0), p);
});

test('findEuclideanRotation identifies rotated tresillo', () => {
  const canonical = bjorklund(8, 3); // rotation 0
  assert.equal(findEuclideanRotation(canonical, 3, 8), 0);
  assert.equal(findEuclideanRotation(rotate(canonical, 2), 3, 8), 2);
});

test('findEuclideanRotation returns null for non-Euclidean pattern', () => {
  // Son clave (gaps 3-3-4-2-4) — the D1 fixture. Not Euclidean at any rotation.
  const sonClave = parsePattern('x . . x . . x . . . x . x . . .');
  assert.equal(onsetCount(sonClave), 5);
  assert.equal(sonClave.length, 16);
  assert.equal(findEuclideanRotation(sonClave, 5, 16), null);
});

test('findAnyEuclideanFit returns null for Reich Clapping Music (D3 fixture)', () => {
  // Reich pattern: x x x . x x . x . x x . — 8 onsets, 12 steps.
  // Opens with three consecutive onsets, which Bjorklund never produces.
  const reich = parsePattern('x x x . x x . x . x x .');
  assert.equal(onsetCount(reich), 8);
  assert.equal(reich.length, 12);
  assert.equal(findAnyEuclideanFit(reich), null);
});

test('parseGrouping "3+3+4+3+3" sums to 16', () => {
  const g = parseGrouping('3+3+4+3+3');
  assert.deepEqual(g, [3, 3, 4, 3, 3]);
  assert.equal(g.reduce((a, b) => a + b, 0), 16);
});

test('parseGrouping tolerates whitespace and rejects non-numerics', () => {
  assert.deepEqual(parseGrouping('2 + 2 + 3'), [2, 2, 3]);
  assert.throws(() => parseGrouping('2+abc+3'), /non-numeric/);
});
