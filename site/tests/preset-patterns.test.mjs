import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile, readdir } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

import {
  getPatternForPreset,
  listPresetNames,
  listChapterAliases,
} from '../src/audio/preset-patterns.ts';
import { euclid, lcm, gcd, rotArr } from '../src/audio/groove-math.ts';

const HERE = dirname(fileURLToPath(import.meta.url));
const MANIFEST_PATH = join(HERE, '..', 'public', 'samples', 'manifest.json');
const CHAPTERS_DIR = join(HERE, '..', 'src', 'content', 'docs');
const PREVIEW_CARD_RE = /<PolyPreviewCard\s+preset="([^"]+)"/g;

async function chapterCardPresetNames() {
  const files = (await readdir(CHAPTERS_DIR)).filter((f) => /^\d\d-.*\.mdx$/.test(f));
  const names = new Set();
  for (const f of files) {
    const src = await readFile(join(CHAPTERS_DIR, f), 'utf8');
    for (const m of src.matchAll(PREVIEW_CARD_RE)) names.add(m[1]);
  }
  return { files, names };
}

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
  // Drift on lane 2 (+2 steps/bar) unrolls the composite to 6 bars = 24 beats.
  assert.equal(pattern.loopBeats, 24);
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

test('Reich Phase Process drift: lane 1 rotates over bars, lane 0 anchor holds', () => {
  const pattern = getPatternForPreset('Reich Phase Process');
  // Lanes 0 (anchor) and 1 (drifter) share note 76; only the lane index
  // distinguishes them at the event level. Collect offsets inside a window.
  const beatsIn = (lane, start, end) =>
    pattern.events
      .filter((e) => e.lane === lane && e.beat >= start && e.beat < end)
      .map((e) => e.beat - start)
      .sort((a, b) => a - b);
  // Anchor's own cycle is 6 beats (12 steps at 1/8), not 4 — with no drift,
  // every 6-beat window replays the same offsets.
  assert.deepEqual(beatsIn(0, 0, 6), beatsIn(0, 6, 12));
  assert.deepEqual(beatsIn(0, 0, 6), beatsIn(0, 18, 24));
  // Drifter shares that 6-beat cycle. With +2 steps/bar the effective step
  // index differs from the anchor's, so its 6-beat windows must diverge.
  const cycle0 = beatsIn(1, 0, 6);
  const cycle1 = beatsIn(1, 6, 12);
  assert.notDeepEqual(
    cycle0,
    cycle1,
    `drifter cycle 0 and cycle 1 should differ; got ${JSON.stringify(cycle0)} vs ${JSON.stringify(cycle1)}`,
  );
  // And it must not be locked to the anchor either — the whole point of drift.
  assert.notDeepEqual(
    beatsIn(0, 0, 24),
    beatsIn(1, 0, 24),
    'drifter events must diverge from anchor across the loop',
  );
});

test('getPatternForPreset is deterministic — same call, same output', () => {
  const a = getPatternForPreset('Reich Phase Process');
  const b = getPatternForPreset('Reich Phase Process');
  assert.equal(a.events.length, b.events.length);
  for (let i = 0; i < a.events.length; i++) {
    assert.deepEqual(a.events[i], b.events[i]);
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

test('every chapter PolyPreviewCard preset resolves to a pattern', async () => {
  const { files, names } = await chapterCardPresetNames();
  assert.ok(files.length >= 15, `expected >= 15 chapter files, got ${files.length}`);
  assert.ok(names.size >= 15, `expected >= 15 unique chapter presets, got ${names.size}`);

  const aliases = new Set(listChapterAliases());
  const direct = new Set(listPresetNames());
  const unresolved = [];
  for (const name of names) {
    const pattern = getPatternForPreset(name);
    if (!pattern) unresolved.push(name);
    // Aliased chapters must not collide with a direct-registered name.
    if (aliases.has(name) && direct.has(name)) {
      assert.fail(`${name} is both a direct preset and an alias — pick one`);
    }
  }
  assert.deepEqual(
    unresolved,
    [],
    `chapter presets that do not resolve to a pattern: ${JSON.stringify(unresolved)}`,
  );
});

test('every chapter alias points at a registered factory preset', () => {
  const registered = new Set(listPresetNames());
  const missing = [];
  for (const alias of listChapterAliases()) {
    const pattern = getPatternForPreset(alias);
    if (!pattern) missing.push(alias);
  }
  assert.deepEqual(missing, [], `aliases without a target pattern: ${JSON.stringify(missing)}`);
  // Sanity: at least one alias exists — otherwise chapter play buttons will all no-op.
  assert.ok(listChapterAliases().length > 0, 'expected chapter aliases to be populated');
  assert.ok(registered.size > 0);
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
