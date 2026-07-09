import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile, readdir } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

import {
  listChapterAliases,
  listPresetNames,
  resolvePreset,
  resolvePresetName,
} from '../src/audio/preset-patterns.ts';

const HERE = dirname(fileURLToPath(import.meta.url));
const MANIFEST_PATH = join(HERE, '..', 'public', 'samples', 'manifest.json');
const CHAPTERS_DIR = join(HERE, '..', 'src', 'content', 'docs');
const PREVIEW_CARD_RE = /<PolyPreviewCard\s+preset="([^"]+)"/g;

async function chapterCardPresetNames() {
  const files = (await readdir(CHAPTERS_DIR)).filter((f) => f.endsWith('.mdx'));
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

test('resolvePreset returns null for unknown preset', () => {
  assert.equal(resolvePreset('Not A Real Preset'), null);
  assert.equal(resolvePresetName('Not A Real Preset'), null);
});

test('every chapter PolyPreviewCard preset resolves', async () => {
  const { files, names } = await chapterCardPresetNames();
  assert.ok(files.length >= 15, `expected >= 15 chapter files, got ${files.length}`);
  assert.ok(names.size >= 15, `expected >= 15 unique chapter presets, got ${names.size}`);

  const unresolved = [];
  for (const name of names) {
    if (!resolvePreset(name)) unresolved.push(name);
  }
  assert.deepEqual(
    unresolved,
    [],
    `chapter presets that do not resolve: ${JSON.stringify(unresolved)}`,
  );
});

test('any chapter alias points at a registered factory preset', () => {
  // After M043 S11 T06 the CHAPTER_ALIASES map is empty — every chapter preset
  // resolves to its native engine preset. This test still runs to guard against
  // a future alias that points nowhere. Empty aliases is legitimate; a broken
  // alias is not.
  const registered = new Set(listPresetNames());
  assert.ok(registered.size > 0);
  const missing = [];
  for (const alias of listChapterAliases()) {
    if (!resolvePreset(alias)) missing.push(alias);
  }
  assert.deepEqual(missing, [], `aliases without a target: ${JSON.stringify(missing)}`);
});

test('every registered preset returns a well-formed ResolvedPreset', () => {
  const names = listPresetNames();
  assert.ok(names.length >= 15, `expected >= 15 presets, got ${names.length}`);

  for (const name of names) {
    const resolved = resolvePreset(name);
    assert.ok(resolved, `expected a ResolvedPreset for ${name}`);
    assert.ok(resolved.bpm > 0, `${name}: bpm must be positive`);
    assert.ok(resolved.lanes.length > 0, `${name}: must define lanes`);
    assert.ok(resolved.notesInBar > 0, `${name}: notesInBar must be positive`);
    assert.equal(
      resolved.laneRoleMeta.length,
      resolved.lanes.length,
      `${name}: laneRoleMeta length must match lanes length`,
    );
    for (const lane of resolved.lanes) {
      assert.ok(lane.label && lane.label.length > 0, `${name}: lane missing label`);
      assert.ok(lane.role && lane.role.length > 0, `${name}: lane missing role`);
      assert.ok(
        lane.note >= 0 && lane.note <= 127,
        `${name}: lane note ${lane.note} out of MIDI range`,
      );
    }
  }
});

test('every preset a chapter card can reach uses only manifest-covered notes', async () => {
  // The playable set = whatever MDX cards can trigger. This gate catches drift
  // where a new chapter or removed alias would leave the card fail-loud on a
  // note the sample manifest does not yet cover.
  const covered = await coveredNotes();
  const { names: cardNames } = await chapterCardPresetNames();
  const uncovered = [];
  for (const name of cardNames) {
    const resolved = resolvePreset(name);
    assert.ok(resolved, `card preset "${name}" does not resolve`);
    for (const lane of resolved.lanes) {
      if (!covered.has(lane.note)) {
        uncovered.push({ preset: name, note: lane.note });
        break;
      }
    }
  }
  assert.deepEqual(
    uncovered,
    [],
    `card presets referencing notes not in manifest: ${JSON.stringify(uncovered)}`,
  );
});
