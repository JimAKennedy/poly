// central-africa M003/S02, CA28. Chapter 2b prints each Central African formula
// as a box-notation grid in a fence titled with the factory preset that plays
// it. This test reads every such grid and compares it, position by position,
// with that preset's lanes in the engine-generated presets.json, and checks
// that each `(count)` row is the per-position sum of its grid. A grid that
// drifts from the preset — or a preset that drifts from the page — fails here.
//
// Grid format (see docs/plans/central-africa/M003-S02-plan.md):
//   ```text title="<preset name>"
//   <label> [x . . | . x . | ...]
//   (count) [1 . 2 | ...]
//   ```
// Part rows map to the preset's lanes in order. A label holding "/" names one
// lane per "/"-separated name: several lanes playing one part.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

import { bjorklund, rotateRight } from '../src/audio/bjorklund.ts';
import { resolvePresetName } from '../src/audio/preset-patterns.ts';

const HERE = dirname(fileURLToPath(import.meta.url));
const PAGE = join(HERE, '..', 'src', 'content', 'docs', '02b-central-africa.mdx');
const PRESETS = JSON.parse(
  readFileSync(join(HERE, '..', 'src', 'generated', 'presets.json'), 'utf8'),
).presets;

const REQUIRED = [
  'Aka: Ngbolu',
  'Aka: Mokondi',
  'Aka: Yombe',
  'Aka: Monzoli',
  'Aka: Mombenzele',
  'Aka: Zoboko',
  'Aka: Bondo',
  'Aka: Ngbolu Drums',
  'Banda-Dakpa: Gaza',
  'Zande: Kponingbo',
  'Banda-Linda: Aga-torumo',
];

function gridsOf(source) {
  const fence = /^```text title="([^"]+)"\n([\s\S]*?)^```$/gm;
  const grids = [];
  for (const m of source.matchAll(fence)) {
    const rows = m[2]
      .split('\n')
      .filter((l) => l.trim())
      .map((line) => {
        const at = line.indexOf('[');
        const end = line.indexOf(']', at);
        assert.ok(at > 0 && end > at, `grid "${m[1]}": row is not "label [ ... ]": ${line}`);
        const tokens = line
          .slice(at + 1, end)
          .split(/\s+/)
          .filter((t) => t && t !== '|');
        return { label: line.slice(0, at).trim(), tokens };
      });
    grids.push({ title: m[1], rows });
  }
  return grids;
}

// One lane's onsets over `length` positions, tiled from its own cycle.
function laneOnsets(lane, length) {
  let cycle;
  if (lane.timeline) {
    const fixed = new Set(lane.onsets);
    const period = lane.fixedPatternLength || lane.cycleSteps;
    cycle = Array.from({ length: lane.cycleSteps }, (_, i) => fixed.has(i % period));
  } else {
    cycle = rotateRight(bjorklund(lane.cycleSteps, lane.hits), lane.rotation);
  }
  return Array.from({ length }, (_, i) => cycle[i % cycle.length]);
}

const show = (bits) => bits.map((b) => (b ? 'x' : '.')).join('');

const grids = gridsOf(readFileSync(PAGE, 'utf8'));

test('CA28: chapter 2b prints a grid for every required formula', () => {
  for (const name of REQUIRED) {
    assert.ok(
      grids.some((g) => g.title === name),
      `chapter 2b has no grid titled "${name}"`,
    );
  }
});

for (const grid of grids) {
  test(`CA28: the "${grid.title}" grid is what its preset plays`, () => {
    const engineName = resolvePresetName(grid.title);
    assert.ok(engineName, `"${grid.title}" resolves to no factory preset`);
    const preset = PRESETS.find((p) => p.name === engineName);
    assert.ok(preset, `presets.json has no preset named "${engineName}"`);

    const count = grid.rows.find((r) => r.label === '(count)');
    assert.ok(count, `"${grid.title}" has no (count) row`);
    const parts = grid.rows.filter((r) => r !== count);
    const length = count.tokens.length;

    const lanes = preset.lanes.slice(0, preset.activeLaneCount);
    let next = 0;
    for (const part of parts) {
      assert.equal(part.tokens.length, length, `"${grid.title}" ${part.label}: not ${length} positions`);
      for (const t of part.tokens) assert.match(t, /^[x.]$/, `"${grid.title}" ${part.label}: token "${t}"`);
      const marks = part.tokens.map((t) => t === 'x');
      for (const name of part.label.split('/')) {
        const lane = lanes[next++];
        assert.ok(lane, `"${grid.title}": row ${name} has no lane left to compare with`);
        const played = laneOnsets(lane, length);
        played.forEach((on, i) =>
          assert.equal(
            on,
            marks[i],
            `"${grid.title}" ${name} (lane ${next}), position ${i + 1}: page ${marks[i] ? 'x' : '.'}, ` +
              `preset ${on ? 'x' : '.'}\n  page   ${show(marks)}\n  preset ${show(played)}`,
          ),
        );
      }
    }
    assert.equal(next, lanes.length, `"${grid.title}": ${lanes.length} lanes, but the grid names ${next}`);

    count.tokens.forEach((t, i) => {
      assert.match(t, /^[.\d]$/, `"${grid.title}" (count): token "${t}"`);
      const sum = parts.reduce((n, p) => n + (p.tokens[i] === 'x' ? 1 : 0), 0);
      assert.equal(t === '.' ? 0 : Number(t), sum, `"${grid.title}" (count) position ${i + 1}: printed ${t}, grid sums to ${sum}`);
    });
  });
}
