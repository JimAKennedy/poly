// PresetTable rendering-conformance harness (M071 S04 T02).
//
// appendix-presets.mdx must contain ZERO hand-written parameter tables — every
// per-lane table and every Macros line is rendered by PresetTable.astro from
// ../src/generated/presets.json via ../src/lib/preset-table-data.mjs (D026/D4).
// This harness proves two things:
//
//   1. Fabrication is gone: the MDX has no <PolyPatch>, no `| Lane |` markdown
//      table, and no literal `**Macros:**` line — the only parameter source is
//      the <PresetTable preset="..."> tags.
//   2. Every cell the module renders equals the corresponding raw field in
//      presets.json, RE-DERIVED here independently of the module. A divergence
//      names the preset, the column, and both values so a reintroduced fake
//      value points at itself.
//
// The re-derivation below is deliberately a separate authorship of the same
// mapping: if someone edits preset-table-data.mjs to display a value that is not
// engine truth, these assertions fail.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { dirname, join, relative } from 'node:path';
import { fileURLToPath } from 'node:url';

import presetsData from '../src/generated/presets.json' with { type: 'json' };
import { bjorklund, rotate } from '../src/lib/euclidean-claims.mjs';
import { buildPresetTable, findPreset, formatNumber } from '../src/lib/preset-table-data.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = join(HERE, '..', '..');
const MDX_PATH = join(HERE, '..', 'src', 'content', 'docs', 'appendix-presets.mdx');
const MDX = readFileSync(MDX_PATH, 'utf8');
const MDX_REL = relative(REPO, MDX_PATH);

// Preset names referenced by <PresetTable preset="..."> in document order.
function referencedPresetNames(src) {
  const names = [];
  const re = /<PresetTable\s+preset="([^"]+)"/g;
  let m;
  while ((m = re.exec(src)) !== null) names.push(m[1]);
  return names;
}

const REFERENCED = referencedPresetNames(MDX);

// Independent re-derivation of a single rendered cell straight from the raw
// presets.json lane record — the conformance oracle.
function capitalize(label) {
  return label ? label.charAt(0).toUpperCase() + label.slice(1) : label;
}

function expectedCell(header, lane, index) {
  switch (header) {
    case 'Lane':
      return String(index + 1);
    case 'Role':
      return capitalize(lane.roleLabel);
    case 'Steps':
      return String(lane.cycleSteps);
    case 'Hits':
      return String(lane.hits);
    case 'Rotation':
      return String(lane.rotation);
    case 'Subdivision':
      return `1/${lane.subdivision}`;
    case 'Note':
      return String(lane.noteNumber);
    case 'Velocity':
      return String(lane.velocity);
    case 'Ghost':
      return String(lane.ghostFloor);
    case 'Swing':
      return formatNumber(lane.swingAmount);
    case 'Mutation':
      return formatNumber(lane.mutationRate);
    case 'Drift':
      return formatNumber(lane.driftRate);
    case 'Timing':
      return `${formatNumber(lane.timingOffsetMs)}ms`;
    case 'Humanize':
      return `${formatNumber(lane.humanizeMs)}ms`;
    case 'Micro-timing':
      return lane.hasMicroTiming ? 'yes' : '—';
    case 'Kotekan':
      return lane.kotekanSourceLane === -1 ? 'off' : `L${lane.kotekanSourceLane + 1}`;
    case 'Phrase Len':
      return String(lane.phraseLength);
    case 'Gap':
      return String(lane.phraseGap);
    case 'Offset':
      return String(lane.phraseOffset);
    case 'Cells':
      return lane.cellCount > 0 ? `[${lane.cellSizes.join('+')}]` : '—';
    case 'Timeline':
      return lane.timeline ? `fixed(${lane.fixedPatternLength})` : '—';
    default:
      throw new Error(`unknown PresetTable column header "${header}"`);
  }
}

// Conditional columns and the raw-field presence predicate that must force them
// to appear — so no lane data is silently hidden by a dropped column.
const CONDITIONAL_PRESENCE = [
  ['Swing', (l) => l.swingAmount !== 0],
  ['Mutation', (l) => l.mutationRate !== 0],
  ['Drift', (l) => l.driftRate !== 0],
  ['Timing', (l) => l.timingOffsetMs !== 0],
  ['Humanize', (l) => l.humanizeMs !== 0],
  ['Micro-timing', (l) => l.hasMicroTiming],
  ['Kotekan', (l) => l.kotekanSourceLane !== -1],
  ['Phrase Len', (l) => l.phraseLength !== 0],
  ['Cells', (l) => l.cellCount > 0],
  ['Timeline', (l) => l.timeline],
];

// --- Fabrication guards ----------------------------------------------------

test('appendix-presets.mdx contains no hand-written parameter tables', () => {
  const fail = [];
  if (/<PolyPatch/.test(MDX)) fail.push('still imports/uses <PolyPatch> (hand table container)');
  const lines = MDX.split('\n');
  lines.forEach((line, i) => {
    if (/^\s*\|\s*Lane\s*\|/.test(line)) fail.push(`${MDX_REL}:${i + 1} hand-written "| Lane |" markdown table header`);
    if (/^\s*\*\*Macros:\*\*/.test(line)) fail.push(`${MDX_REL}:${i + 1} hand-written "**Macros:**" line (must render from PresetTable)`);
  });
  assert.equal(fail.length, 0, `\n  ${fail.join('\n  ')}`);
});

test('appendix-presets.mdx references the founding fourteen presets, all resolvable', () => {
  assert.equal(REFERENCED.length, 14, `expected 14 <PresetTable> references, found ${REFERENCED.length}`);
  const unresolved = REFERENCED.filter((name) => !presetsData.presets.some((p) => p.name === name));
  assert.equal(unresolved.length, 0, `unresolved preset names in ${MDX_REL}: ${unresolved.join(', ')}`);
});

// --- Per-preset value conformance ------------------------------------------

for (const name of REFERENCED) {
  test(`PresetTable "${name}" renders engine truth for every cell`, () => {
    const record = findPreset(presetsData.presets, name);
    const { headers, rows, macros } = buildPresetTable(record);

    // Completeness: any lane field with data forces its column to appear.
    for (const [header, present] of CONDITIONAL_PRESENCE) {
      if (record.lanes.some(present) && !headers.includes(header)) {
        assert.fail(`preset "${name}": lane data present for "${header}" but the column is missing — value would be hidden`);
      }
    }

    assert.equal(rows.length, record.lanes.length, `preset "${name}": ${rows.length} rendered rows vs ${record.lanes.length} lanes`);

    rows.forEach((row, laneIndex) => {
      const lane = record.lanes[laneIndex];
      headers.forEach((header, col) => {
        const expected = expectedCell(header, lane, laneIndex);
        assert.equal(
          row[col],
          expected,
          `preset "${name}" lane ${laneIndex + 1} column "${header}": rendered "${row[col]}" diverges from presets.json truth "${expected}"`,
        );
      });
    });

    // Macros line re-derived from raw macros.
    const rawMacros = record.macros;
    const expectedMacros = [
      `Density ${formatNumber(rawMacros.density)}`,
      `Complexity ${formatNumber(rawMacros.complexity)}`,
      `Syncopation ${formatNumber(rawMacros.syncopation)}`,
      `Swing ${formatNumber(rawMacros.swing)}`,
      `Tension ${formatNumber(rawMacros.tension)}`,
      `Humanize ${formatNumber(rawMacros.humanize)}`,
    ];
    assert.deepEqual(macros, expectedMacros, `preset "${name}": Macros line diverges from presets.json macros`);
  });
}

// --- findPreset failure visibility -----------------------------------------

test('findPreset throws a named error for an unresolved preset', () => {
  assert.throws(
    () => findPreset(presetsData.presets, 'No Such Preset'),
    /No Such Preset/,
    'findPreset must name the unresolved preset',
  );
});

// M005/S02 (PIPE02). Until schemaVersion 4 this question could not be asked of
// presets.json at all: it carried `timeline` and `fixedPatternLength` but not
// the pattern, so an authored clave and the Euclidean bake of the same hit
// count and cycle serialised identically. M001/S02 authored three patterns the
// guide states explicitly; this asserts the shipped data matches the prose AND
// differs from the Euclidean neighbour, because matching the prose alone would
// still pass if someone re-derived the lane from Bjorklund.
const onsetsOf = (pattern) => pattern.flatMap((on, i) => (on ? [i] : []));

test('timeline presets carry their authored pattern, not its Euclidean neighbour', () => {
  const cases = [
    { preset: 'Cuban Son Montuno', role: 'clave', onsets: [0, 3, 6, 10, 12], source: 'theory-afro-cuban.mdx' },
    { preset: 'Rumba Clave', role: 'clave', onsets: [0, 3, 7, 10, 12], source: '03-afro-cuban.mdx' },
    { preset: 'Clapping Music', role: 'clap', onsets: [0, 1, 2, 4, 5, 7, 9, 10], source: '08-minimalism.mdx' },
  ];

  for (const c of cases) {
    const preset = presetsData.presets.find((p) => p.name === c.preset);
    assert.ok(preset, `no factory preset named ${c.preset}`);
    const lane = preset.lanes.find((l) => l.timeline && l.roleLabel === c.role);
    assert.ok(lane, `${c.preset} has no timeline lane labelled ${c.role}`);

    assert.deepStrictEqual(
      lane.onsets,
      c.onsets,
      `${c.preset} ${c.role} onsets are ${JSON.stringify(lane.onsets)}, but ${c.source} states ${JSON.stringify(c.onsets)}`,
    );

    // Computed, not hand-typed: the claim is that the two differ, so both sides
    // have to be derived or the test is just two literals sitting next to each
    // other.
    const euclidean = onsetsOf(rotate(bjorklund(lane.cycleSteps, lane.hits), lane.rotation));
    assert.notDeepStrictEqual(
      lane.onsets,
      euclidean,
      `${c.preset} ${c.role} carries E(${lane.hits},${lane.cycleSteps}) rather than the authored pattern ${c.source} states`,
    );
  }
});
