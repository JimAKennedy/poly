// central-africa M003/S01, CA22. Chapter 2b's rhythmic-oddity table (norm N2)
// prints each of Arom's oddity figures as a cell sequence, a length, the cut
// nearest the middle, and the onsets the cells produce. Every one of those is
// arithmetic a reader can redo, so this test redoes it: the cells must sum to
// the stated length, no running total may land on the halfway point, the
// stated split must be the cut nearest it (half-1/half+1), and the box
// notation must put an onset at the start of each cell and nowhere else.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const HERE = dirname(fileURLToPath(import.meta.url));
const PAGE = join(HERE, '..', 'src', 'content', 'docs', '02b-central-africa.mdx');

// The rows the chapter must carry: exx. 31, 33, 34, 35 and 36 (pp. 246-247)
// and the bòndò model (ex. 56, p. 255).
const REQUIRED_EXAMPLES = ['ex. 31', 'ex. 33', 'ex. 34', 'ex. 35', 'ex. 36', 'ex. 56'];

function oddityRows(source) {
  const lines = source.split('\n');
  const header = lines.findIndex((l) => l.startsWith('|') && /\|\s*Cells\s*\|/.test(l));
  assert.notEqual(header, -1, 'chapter 2b has no table with a "Cells" column');
  const columns = lines[header]
    .split('|')
    .slice(1, -1)
    .map((c) => c.trim());
  const rows = [];
  for (let i = header + 2; i < lines.length && lines[i].startsWith('|'); i++) {
    const cells = lines[i]
      .split('|')
      .slice(1, -1)
      .map((c) => c.trim());
    rows.push(Object.fromEntries(columns.map((c, j) => [c, cells[j]])));
  }
  return rows;
}

function boxOnsets(box) {
  const tokens = box
    .replace(/[`[\]|]/g, ' ')
    .split(/\s+/)
    .filter(Boolean);
  for (const t of tokens) assert.match(t, /^[x.]$/, `unexpected box token "${t}" in ${box}`);
  return tokens.map((t) => t === 'x');
}

const rows = oddityRows(readFileSync(PAGE, 'utf8'));

test('CA22: the oddity table carries every required example', () => {
  for (const ex of REQUIRED_EXAMPLES) {
    assert.ok(
      rows.some((r) => r.Example.startsWith(`${ex},`)),
      `oddity table is missing ${ex}`,
    );
  }
});

for (const row of rows) {
  test(`CA22: ${row.Example} — ${row.Cells} is an oddity figure as printed`, () => {
    const cells = row.Cells.split('+').map(Number);
    const length = Number(row.Steps);
    assert.equal(
      cells.reduce((a, b) => a + b, 0),
      length,
      `${row.Cells} does not sum to ${length}`,
    );

    const totals = [];
    let running = 0;
    for (const c of cells.slice(0, -1)) totals.push((running += c));
    const half = length / 2;
    assert.ok(!totals.includes(half), `${row.Cells} has a cut at the halfway point ${half}`);

    const nearest = totals.reduce((best, t) =>
      Math.abs(t - half) < Math.abs(best - half) ? t : best,
    );
    assert.equal(Math.abs(nearest - half), 1, `${row.Cells}: nearest cut ${nearest} is not half±1`);
    const [a, b] = row.Split.split('/').map(Number);
    assert.deepEqual(
      [a, b],
      [Math.min(nearest, length - nearest), Math.max(nearest, length - nearest)],
      `${row.Example}: stated split ${row.Split} is not the cut nearest the middle`,
    );

    const starts = new Set([0, ...totals]);
    const onsets = boxOnsets(row.Onsets);
    assert.equal(onsets.length, length, `${row.Example}: box notation is not ${length} positions`);
    onsets.forEach((on, i) =>
      assert.equal(on, starts.has(i), `${row.Example}: position ${i + 1} disagrees with the cells`),
    );
  });
}
