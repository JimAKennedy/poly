import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

import { getPatternForPreset, listPresetNames } from '../src/audio/preset-patterns.ts';
import { euclid, lcm, gcd, rotArr } from '../src/audio/groove-math.ts';

const HERE = dirname(fileURLToPath(import.meta.url));
const MANIFEST_PATH = join(HERE, '..', 'public', 'samples', 'manifest.json');

async function coveredNotes() {
  const raw = await readFile(MANIFEST_PATH, 'utf8');
  const manifest = JSON.parse(raw);
  const set = new Set();
  for (const s of manifest.samples) for (const n of s.midiNotes) set.add(n);
  return set;
}

test('euclid(k, n) matches Bresenham reference cases', () => {
  assert.deepEqual(euclid(3, 8), [1, 0, 0, 1, 0, 0, 1, 0]);
  assert.deepEqual(euclid(5, 8), [1, 0, 1, 0, 1, 1, 0, 1]);
  assert.deepEqual(euclid(4, 4), [1, 1, 1, 1]);
  const e5_12 = euclid(5, 12);
  assert.equal(e5_12.filter((x) => x === 1).length, 5);
  assert.equal(e5_12.length, 12);
  assert.equal(e5_12[0], 1);
});

test('rotArr rotates by r positions', () => {
  assert.deepEqual(rotArr([1, 2, 3, 4], 1), [4, 1, 2, 3]);
  assert.deepEqual(rotArr([1, 2, 3, 4], -1), [2, 3, 4, 1]);
});

test('gcd/lcm math', () => {
  assert.equal(gcd(12, 8), 4);
  assert.equal(lcm(12, 8), 24);
});

test('getPatternForPreset("Reich Phase Process") returns a well-formed pattern', () => {
  const pattern = getPatternForPreset('Reich Phase Process');
  assert.ok(pattern, 'expected a Pattern object');
  assert.equal(pattern.bpm, 100);
  assert.equal(pattern.loopBeats, 12);
  assert.ok(
    pattern.events.length >= 12,
    `expected >= 12 events per loop, got ${pattern.events.length}`,
  );
  // events must be sorted by beat and within the loop range.
  for (let i = 1; i < pattern.events.length; i++) {
    assert.ok(
      pattern.events[i].beat >= pattern.events[i - 1].beat,
      'events must be sorted by beat',
    );
  }
  for (const e of pattern.events) {
    assert.ok(e.beat >= 0 && e.beat < pattern.loopBeats);
    assert.ok(e.velocity >= 1 && e.velocity <= 127);
  }
});

test('Reich Phase Process uses only notes present in manifest.json', async () => {
  const covered = await coveredNotes();
  const pattern = getPatternForPreset('Reich Phase Process');
  const usedNotes = new Set(pattern.events.map((e) => e.note));
  for (const n of usedNotes) {
    assert.ok(covered.has(n), `note ${n} referenced by preset but not in manifest`);
  }
});

test('getPatternForPreset returns null for unknown preset', () => {
  assert.equal(getPatternForPreset('Not A Real Preset'), null);
});

test('every registered preset returns a well-formed pattern', async () => {
  const covered = await coveredNotes();
  const names = listPresetNames();
  // Reich Phase Process (chapter 8 preview) + 14 Factory presets from the appendix.
  assert.ok(names.length >= 15, `expected >= 15 presets, got ${names.length}`);

  for (const name of names) {
    const pattern = getPatternForPreset(name);
    assert.ok(pattern, `expected a Pattern object for ${name}`);
    assert.ok(pattern.bpm > 0, `${name}: bpm must be positive`);
    assert.ok(pattern.loopBeats > 0, `${name}: loopBeats must be positive`);
    assert.ok(pattern.events.length > 0, `${name}: must have at least one event`);
    assert.ok(pattern.lanes && pattern.lanes.length > 0, `${name}: must define lanes`);

    for (let i = 1; i < pattern.events.length; i++) {
      assert.ok(
        pattern.events[i].beat >= pattern.events[i - 1].beat,
        `${name}: events must be sorted by beat`,
      );
    }
    for (const e of pattern.events) {
      assert.ok(
        e.beat >= 0 && e.beat < pattern.loopBeats,
        `${name}: event beat ${e.beat} outside [0, ${pattern.loopBeats})`,
      );
      assert.ok(
        e.velocity >= 1 && e.velocity <= 127,
        `${name}: MIDI velocity ${e.velocity} out of range`,
      );
      assert.ok(
        covered.has(e.note),
        `${name}: note ${e.note} referenced but not in manifest`,
      );
    }
  }
});
