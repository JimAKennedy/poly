// The reference appendices catch up with the Central Africa chapter
// (central-africa M005/S01, rows CA35–CA37 of docs/plans/central-africa/ledger.md).
//
// Chapter 2b relies on three things the rest of the guide did not carry: a
// 24-step Euclidean family for the dì.kétɔ̀ mould, five steps to a beat, and
// a Chapter 1 that does not present co-prime cycles as what traditions do.
// Each is locked here. Structural cases derive their expectation from
// bjorklund.ts, presets.json and Chapter 2b's own tables rather than
// transcribing it.
//
// Wired into scripts/check-doc-conformance.sh and named in REQUIRED in
// doc-conformance-wiring.test.mjs.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

import presetsData from '../src/generated/presets.json' with { type: 'json' };
import euclideanAppendix from '../src/data/euclidean-appendix.json' with { type: 'json' };
import { bjorklund, rotateRight } from '../src/audio/bjorklund.ts';
import { registerClaimTests } from './helpers/prose-claims.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const DOCS = join(HERE, '..', 'src', 'content', 'docs');
const EUCLIDEAN = 'appendix-euclidean-reference.mdx';
const CENTRAL_AFRICA = '02b-central-africa.mdx';

const loadSource = (file) => readFile(join(DOCS, file), 'utf8');

const presets = Array.isArray(presetsData) ? presetsData : presetsData.presets;

const render = (pattern) => pattern.map((on) => (on ? 'x' : '.')).join(' ');

function gaps(pattern) {
  const onsets = pattern.flatMap((on, i) => (on ? [i] : []));
  return onsets.map((at, i) => {
    const next = onsets[(i + 1) % onsets.length];
    return ((next - at + pattern.length - 1) % pattern.length) + 1;
  });
}

function section(n) {
  return euclideanAppendix.sections.find((s) => s.n === n);
}

// The rows between one generated region's markers, in order.
function generatedRows(src, region) {
  const begin = src.indexOf(`{/* BEGIN GENERATED: ${region} */}`);
  const end = src.indexOf(`{/* END GENERATED: ${region} */}`);
  assert.ok(begin !== -1 && end > begin, `region ${region} is missing from ${EUCLIDEAN}`);
  return src
    .slice(begin, end)
    .split('\n')
    .map((line) => line.match(/^\|\s*E\((\d+),(\d+)\)\s*\|\s*`([^`]+)`\s*\|\s*([^|]+?)\s*\|/))
    .filter(Boolean)
    .map((m) => ({ k: Number(m[1]), n: Number(m[2]), pattern: m[3], grouping: m[4] }));
}

// Rotation cells of Chapter 2b's mould table: `| formula | N pulsations | R |`.
async function mouldRotations() {
  const src = await loadSource(CENTRAL_AFRICA);
  const start = src.indexOf('## The mould');
  assert.ok(start !== -1, `${CENTRAL_AFRICA} has no "## The mould" section`);
  const body = src.slice(start, src.indexOf('\n## ', start + 1));
  return [...body.matchAll(/^\|[^|]+\|\s*\d+ pulsations?\s*\|\s*(\d+)\s*\|/gm)].map((m) =>
    Number(m[1]),
  );
}

const CLAIMS = [
  {
    id: 'CA35-link',
    file: EUCLIDEAN,
    rule:
      'ledger CA35. The 24-step section sends the reader to the mould section of ' +
      'Chapter 2b, where the five rotations are taught',
    present: ['/02b-central-africa/#the-mould'],
  },
];

registerClaimTests({ test, assert, claims: CLAIMS, loadSource });

test('CA35: the fixture has a 24-step section whose rows are E(13,24) at the five mould rotations', async () => {
  const s24 = section(24);
  assert.ok(s24, 'euclidean-appendix.json has no n = 24 section');
  for (const row of s24.rows) assert.equal(row.k, 13, `24-step row has k = ${row.k}, not 13`);
  const fixture = new Set(s24.rows.map((r) => r.rotation ?? 0));
  const mould = new Set(await mouldRotations());
  assert.equal(mould.size, 5, `Chapter 2b's mould table gives ${mould.size} rotations, not 5`);
  assert.deepEqual([...fixture].sort((a, b) => a - b), [...mould].sort((a, b) => a - b));
});

test("CA35: each mould rotation is what the presets' dì.kétɔ̀ lanes play", () => {
  const rotations = new Set((section(24)?.rows ?? []).map((r) => r.rotation ?? 0));
  const lanes = presets
    .filter((p) => p.category === 'Central African')
    .flatMap((p) =>
      p.lanes.filter((l) => l.cycleSteps === 24 && l.hits === 13).map((l) => ({ p, l })),
    );
  assert.ok(lanes.length >= 6, `expected >= 6 dì.kétɔ̀ lanes, found ${lanes.length}`);
  for (const { p, l } of lanes) {
    assert.ok(
      rotations.has(l.rotation),
      `${p.name} plays E(13,24) at rotation ${l.rotation}, which the 24-step section does not show`,
    );
  }
});

test('CA35: every generated 24-step row is bjorklund at its rotation', async () => {
  const fixtureRows = section(24)?.rows ?? [];
  const rows = generatedRows(await loadSource(EUCLIDEAN), 'euclidean-24');
  assert.equal(rows.length, fixtureRows.length, 'generated rows do not match the fixture');
  rows.forEach((row, i) => {
    const expected = rotateRight(bjorklund(24, 13), fixtureRows[i].rotation ?? 0);
    assert.equal(row.k, 13);
    assert.equal(row.n, 24);
    assert.equal(row.pattern, render(expected), `row ${i + 1} pattern`);
    assert.equal(row.grouping, gaps(expected).join('+'), `row ${i + 1} grouping`);
  });
});

test("CA35: the 12-step section names bòndò's dì.kétɔ̀ as E(7,12) rotation 0", async () => {
  const bondo = section(12).rows.find((r) => r.k === 7 && /bòndò/.test(r.association));
  assert.ok(bondo, 'the 12-step fixture section has no bòndò E(7,12) row');
  assert.equal(bondo.rotation ?? 0, 0);
  const rows = generatedRows(await loadSource(EUCLIDEAN), 'euclidean-12');
  const r0 = render(rotateRight(bjorklund(12, 7), 0));
  assert.ok(
    rows.some((r) => r.k === 7 && r.pattern === r0),
    `the generated 12-step region has no E(7,12) row at rotation 0 (${r0})`,
  );
  const preset = presets.find((p) => p.name === 'Aka: Bondo');
  const lane = preset.lanes.find((l) => l.cycleSteps === 12 && l.hits === 7);
  assert.equal(lane.rotation, 0, 'Aka: Bondo no longer plays its dì.kétɔ̀ at rotation 0');
});

test('CA35: the fixture note describes the gankogui at the rotation the table prints', () => {
  const gankogui = section(12).rows.find((r) => r.k === 7 && /gankogui/.test(r.association));
  const rotation = gankogui.rotation ?? 0;
  assert.ok(
    euclideanAppendix.$note.includes(`gankogui bell at rotation ${rotation}`),
    `$note does not say the gankogui bell is at rotation ${rotation}`,
  );
  if (rotation !== 1) assert.ok(!euclideanAppendix.$note.includes('gankogui bell at rotation 1'));
});
