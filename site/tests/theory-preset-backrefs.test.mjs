// first-release M002/S02, FR25. M002/S01 deleted fourteen pointers from shipping
// pages into the deep dives, five of them into #what-breaks-the-idiom sections.
// The rules survive in the deferred bundle; the link between a preset and the
// rule it knowingly breaks did not survive anywhere.
//
// The deep dives now record it from the other side, which is the direction that
// lasts: they are the pages that state the rules, and the presets are shipping
// and stable, so a reference from a rule to a preset cannot rot while the bundle
// is unpublished — unless the preset is renamed or removed, which is exactly
// what this guard is for.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile, readdir } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const HERE = dirname(fileURLToPath(import.meta.url));
const THEORY = join(HERE, '..', 'src', 'content', 'theory');
const PRESETS = join(HERE, '..', 'src', 'generated', 'presets.json');

const HEADING = '## Presets that bend these rules';

async function presetNames() {
  const raw = JSON.parse(await readFile(PRESETS, 'utf8'));
  const list = Array.isArray(raw) ? raw : raw.presets;
  const names = new Set((list ?? []).map((p) => p.name));
  if (names.size === 0) {
    throw new Error(
      `no preset names read from ${PRESETS} — has the emitter's shape changed? ` +
        'An empty set would let every assertion below pass while checking nothing.',
    );
  }
  return names;
}

// Bold spans inside the section are the preset references: **Kotekan Interlock**.
function referencedPresets(section) {
  return [...section.matchAll(/\*\*([A-Z][^*]{2,40})\*\*/g)]
    .map((m) => m[1].trim())
    .filter((n) => !/^Rule \d+$/.test(n));
}

test('every preset a deep dive names exists in presets.json', async () => {
  const names = await presetNames();
  const files = (await readdir(THEORY)).filter((f) => f.endsWith('.mdx'));

  const unknown = [];
  let sections = 0;
  for (const f of files) {
    const src = await readFile(join(THEORY, f), 'utf8');
    const i = src.indexOf(HEADING);
    if (i < 0) continue;
    sections += 1;
    for (const ref of referencedPresets(src.slice(i))) {
      if (!names.has(ref)) unknown.push(`${f}: ${ref}`);
    }
  }

  assert.ok(
    sections > 0,
    `no deep dive carries a "${HEADING}" section — FR25 recorded which presets ` +
      'bend which rules, and a guard that finds none is asserting nothing',
  );
  assert.deepEqual(
    unknown,
    [],
    'named by a deep dive but absent from presets.json — a renamed or removed ' +
      'preset leaves the rule pointing at nothing: ' + unknown.join(', '),
  );
});
