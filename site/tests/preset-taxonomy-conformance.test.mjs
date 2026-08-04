// M071 S04 T03 (P4.5): lock docs/preset-taxonomy.md to engine truth.
//
// The taxonomy doc carries two hand-authored tables that must agree with the
// generated presets.json exactly: the per-preset category mapping and the
// per-category counts. presets.json is emitted from kInfos[]/kFactoryPresetCategories
// in engine/src/presets.cpp (schemaVersion 3), so this test is the doc-time guard
// that a preset's documented category can no longer drift from the engine.
//
// Failure visibility: each assertion names the preset (by index+name) or the
// category and the exact doc-vs-engine divergence.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const PRESETS_PATH = join(HERE, '..', 'src', 'generated', 'presets.json');
const TAXONOMY_PATH = join(HERE, '..', '..', 'docs', 'preset-taxonomy.md');

async function loadPresets() {
  return JSON.parse(await readFile(PRESETS_PATH, 'utf8'));
}

// Parse the "Preset-to-category mapping" table: rows are `| N | Name | Category |`.
function parseMappingRows(md) {
  const rows = [];
  const lines = md.split('\n');
  const rowRe = /^\|\s*(\d+)\s*\|\s*([^|]+?)\s*\|\s*([^|]+?)\s*\|\s*$/;
  let inMapping = false;
  for (const line of lines) {
    if (line.startsWith('## Preset-to-category mapping')) {
      inMapping = true;
      continue;
    }
    if (inMapping && line.startsWith('## ')) break; // next section
    if (!inMapping) continue;
    const m = line.match(rowRe);
    if (!m) continue;
    if (m[2].trim() === 'Preset name') continue; // header row
    rows.push({ index: parseInt(m[1], 10), name: m[2].trim(), category: m[3].trim() });
  }
  return rows;
}

// Parse the "The 10 categories" table: rows are `| N | Category | Count | ... |`.
function parseCategoryCountRows(md) {
  const rows = [];
  const lines = md.split('\n');
  const rowRe = /^\|\s*(\d+)\s*\|\s*([^|]+?)\s*\|\s*(\d+)\s*\|/;
  let inSection = false;
  for (const line of lines) {
    if (line.startsWith('## The 10 categories')) {
      inSection = true;
      continue;
    }
    if (inSection && line.startsWith('## ')) break;
    if (!inSection) continue;
    const m = line.match(rowRe);
    if (!m) continue;
    rows.push({ category: m[2].trim(), count: parseInt(m[3], 10) });
  }
  return rows;
}

test('preset-taxonomy.md mapping matches presets.json exactly (index, name, category)', async () => {
  const presets = await loadPresets();
  const md = await readFile(TAXONOMY_PATH, 'utf8');
  const rows = parseMappingRows(md);

  assert.equal(
    rows.length,
    presets.presets.length,
    `mapping table has ${rows.length} rows, presets.json has ${presets.presets.length} presets`,
  );

  const failures = [];
  for (const row of rows) {
    const p = presets.presets[row.index];
    if (!p) {
      failures.push(`row #${row.index} "${row.name}": no preset at that index in presets.json`);
      continue;
    }
    if (p.name !== row.name) {
      failures.push(
        `index ${row.index}: doc name "${row.name}" != presets.json name "${p.name}"`,
      );
    }
    if (p.category !== row.category) {
      failures.push(
        `preset ${row.index} "${p.name}": doc category "${row.category}" != presets.json category "${p.category}"`,
      );
    }
  }
  assert.equal(failures.length, 0, `\n${failures.join('\n')}`);
});

test('preset-taxonomy.md category counts match presets.json category tallies', async () => {
  const presets = await loadPresets();
  const md = await readFile(TAXONOMY_PATH, 'utf8');
  const countRows = parseCategoryCountRows(md);

  // Tally categories straight from presets.json.
  const actual = new Map();
  for (const p of presets.presets) {
    actual.set(p.category, (actual.get(p.category) || 0) + 1);
  }

  const failures = [];
  for (const row of countRows) {
    const got = actual.get(row.category);
    if (got === undefined) {
      failures.push(`doc category "${row.category}" is not a category in presets.json`);
      continue;
    }
    if (got !== row.count) {
      failures.push(
        `category "${row.category}": doc count ${row.count} != presets.json tally ${got}`,
      );
    }
  }
  // Every presets.json category must appear in the doc count table.
  for (const cat of actual.keys()) {
    if (!countRows.some((r) => r.category === cat)) {
      failures.push(`presets.json category "${cat}" is missing from the doc's category table`);
    }
  }
  assert.equal(failures.length, 0, `\n${failures.join('\n')}`);
});
