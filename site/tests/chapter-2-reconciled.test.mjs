// Chapter 2 reconciled with the Central Africa chapter (central-africa M004/S01,
// rows CA29–CA34 of docs/plans/central-africa/ledger.md).
//
// Chapter 2 spoke for "West and Central Africa", built its stacking section on
// an uncited claim about Ewe cycle lengths, asserted a downbeat, and printed a
// patch table that matched no preset. Each correction is locked here: the
// prose cases through the shared prose-claims helper, the structural ones as
// plain tests that derive their expectation rather than transcribe it.
//
// Wired into scripts/check-doc-conformance.sh and named in REQUIRED in
// doc-conformance-wiring.test.mjs.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { existsSync } from 'node:fs';
import { readFile } from 'node:fs/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

import presetsData from '../src/generated/presets.json' with { type: 'json' };
import { registerClaimTests } from './helpers/prose-claims.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const DOCS = join(HERE, '..', 'src', 'content', 'docs');
const PAGE = '02-sub-saharan-africa.mdx';

const loadSource = (file) => readFile(join(DOCS, file), 'utf8');

const CLAIMS = [
  {
    id: 'CA29',
    file: PAGE,
    rule:
      'ledger CA29. The description and opening claimed "West and Central African ' +
      'drumming" and offered the timeline/support/lead model for both. The Aka ' +
      'formulae have no timeline in the Ewe sense and their drums barely vary ' +
      '(Arom norm N10), so the chapter is scoped to West Africa',
    forbidden: ['West and Central Africa', 'West and Central African'],
    present: ['West African drumming', 'West Africa'],
  },
  {
    id: 'CA34',
    file: PAGE,
    rule: 'ledger CA34. Chapter 2 sends readers to Chapter 2b for Central African music',
    present: ['/02b-central-africa/'],
  },
  {
    id: 'CA30-tradition',
    file: PAGE,
    rule:
      'ledger CA30. Polymetric Stacking said Ewe support drums "often run at ' +
      'different cycle lengths from the bell" and cited nothing. Locke 1982 ' +
      '(fr-locke-1982), read 2026-10-18, supports a simple-ratio account only: the ' +
      'bell sets the period (p. 217), the kagan cycles 4 times and the kidi twice ' +
      'per bell pattern (p. 219), and the common cross-rhythms are 3:2 and 3:4 ' +
      '(p. 244). The section says that and no more about the tradition',
    forbidden: [
      'often run at different cycle lengths',
      'The power of the Ewe approach is that support drums',
      'Where Ewe drumming stacks many independent cycles',
    ],
    present: ['4 times per bell pattern', 'twice per bell pattern', '3:2 and 3:4', 'fr-locke-1982'],
  },
  {
    id: 'CA30-extension',
    file: PAGE,
    rule:
      'ledger CA30. With the tradition stated from Locke, 12/7/5 layering is ' +
      'presented as what Poly adds, with the Ewe Polymetric Ensemble preset as ' +
      'the Ewe-inspired example (vision decision: "honestly labelled")',
    present: ["Poly's extension", 'Ewe-inspired', 'Ewe Polymetric Ensemble'],
  },
  {
    id: 'CA31',
    file: PAGE,
    rule:
      'ledger CA31. "Pulling forward toward the next downbeat" and "structural ' +
      'downbeats" asserted a downbeat; Arom\'s R6 says there is no measure and no ' +
      'strong beat. The lock is the word itself, so a third assertion cannot ' +
      'replace the two removed',
    forbidden: ['toward the next downbeat', 'structural downbeats'],
    forbiddenRegex: [/\bdownbeats?\b/i],
  },
];

registerClaimTests({ test, assert, claims: CLAIMS, loadSource });

test('CA34: the Central Africa link resolves to a page', async () => {
  const src = await loadSource(PAGE);
  const slugs = [...src.matchAll(/\]\(\/([a-z0-9-]+)\//g)].map((m) => m[1]);
  const bridge = slugs.filter((s) => s === '02b-central-africa');
  assert.ok(bridge.length > 0, `${PAGE} carries no link to /02b-central-africa/`);
  for (const slug of bridge) {
    // Starlight routes a page by its file name, so the file is the route.
    assert.ok(existsSync(join(DOCS, `${slug}.mdx`)), `${PAGE} links /${slug}/, but no ${slug}.mdx exists`);
  }
});

// --- CA32: the patch table is the preset it names -----------------------------

const EWE_PRESET = 'Ewe Polymetric Ensemble';

// The Polymetric Stacking <PolyPatch>: its preset attribute and its markdown
// table as header-keyed rows.
function ewePatch(src) {
  const start = src.indexOf('<PolyPatch title="Ewe');
  assert.ok(start !== -1, `${PAGE} has no Ewe <PolyPatch>`);
  const block = src.slice(start, src.indexOf('</PolyPatch>', start));
  const preset = block.match(/preset="([^"]+)"/)?.[1] ?? null;
  const lines = block.split('\n').filter((l) => l.trim().startsWith('|'));
  const cells = (l) => l.trim().replace(/^\||\|$/g, '').split('|').map((c) => c.trim());
  const header = cells(lines[0]);
  const rows = lines.slice(2).map((l) => Object.fromEntries(cells(l).map((c, i) => [header[i], c])));
  return { block, preset, header, rows };
}

// The display mapping of src/lib/preset-table-data.mjs, re-derived here so a
// change there cannot quietly move what this lock compares.
const COLUMNS = {
  Steps: (l) => String(l.cycleSteps),
  Hits: (l) => String(l.hits),
  Rotation: (l) => String(l.rotation),
  Subdivision: (l) => `1/${l.subdivision}`,
  Timeline: (l) => (l.timeline ? 'on' : 'off'),
  Note: (l) => String(l.noteNumber),
  Velocity: (l) => String(l.velocity),
  Ghost: (l) => String(l.ghostFloor),
  Kotekan: (l) => (l.kotekanSourceLane === -1 ? 'off' : `L${l.kotekanSourceLane + 1}`),
};

test('CA32: the Ewe table is the preset it names', async () => {
  const src = await loadSource(PAGE);
  const { preset, header, rows } = ewePatch(src);
  assert.equal(preset, EWE_PRESET, `the Ewe <PolyPatch> names "${preset}", not the "${EWE_PRESET}" its card plays`);
  const found = presetsData.presets.find((p) => p.name === preset);
  assert.ok(found, `preset "${preset}" is not in src/generated/presets.json`);
  assert.equal(rows.length, found.lanes.length, `the table has ${rows.length} rows; "${preset}" has ${found.lanes.length} lanes`);
  for (const col of Object.keys(COLUMNS)) assert.ok(header.includes(col), `the table has no ${col} column`);
  rows.forEach((row, i) => {
    assert.equal(row.Lane, String(i + 1), `row ${i + 1} is numbered ${row.Lane}`);
    for (const [col, want] of Object.entries(COLUMNS)) {
      assert.equal(row[col], want(found.lanes[i]), `lane ${i + 1} ${col}: table ${row[col]}, preset ${want(found.lanes[i])}`);
    }
  });
});

test('CA32: the prose names the lane that carries the Kotekan setting', async () => {
  const src = await loadSource(PAGE);
  const { rows } = ewePatch(src);
  const m = src.match(/Lane (\d+) uses the Kotekan setting `(L\d+)`/);
  assert.ok(m, `${PAGE} no longer says which lane carries the Kotekan setting`);
  const carriers = rows.filter((r) => r.Kotekan !== 'off');
  assert.deepEqual(
    carriers.map((r) => [r.Lane, r.Kotekan]),
    [[m[1], m[2]]],
    `the prose says Lane ${m[1]} carries ${m[2]}; the table's Kotekan lanes are ` +
      JSON.stringify(carriers.map((r) => `${r.Lane}:${r.Kotekan}`)),
  );
});
