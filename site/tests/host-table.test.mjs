// host-table.test.mjs — the README and chapter 17 carry one host table, and
// neither claims a host nobody has measured.
//
// open-source-launch M003/S01, OS14. README.md said Poly "should work with any
// VST3-compatible host" and named no version; nothing had been loaded anywhere
// but Cubase. Both files now carry the same five-column table — Cubase,
// supported, per platform, with the routing that was actually used — and this
// test holds the two copies identical so they cannot drift apart, forbids the
// unmeasured claim, and (OS17, M003/S03) forbids listing Logic as a host that
// loads Poly. Run: `npm --prefix site test` (site-unit) or
// `node --test site/tests/host-table.test.mjs`.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const REPO = resolve(dirname(fileURLToPath(import.meta.url)), '..', '..');
const README = readFileSync(resolve(REPO, 'README.md'), 'utf8');
const CH17 = readFileSync(resolve(REPO, 'site/src/content/docs/17-midi-routing-note-map.mdx'), 'utf8');

const HEADER = '| Host | Version | Platform | Status | Routing |';

// The rows of the first markdown table whose header row is HEADER, each row
// trimmed; null when the file has no such table.
function hostTable(text) {
  const lines = text.split('\n');
  const start = lines.findIndex((l) => l.trim() === HEADER);
  if (start === -1) return null;
  const rows = [];
  for (let i = start + 2; i < lines.length && lines[i].trim().startsWith('|'); i++) {
    rows.push(lines[i].trim());
  }
  return rows;
}

const cells = (row) => row.split('|').slice(1, -1).map((c) => c.trim());

test('README.md and chapter 17 carry the same host table', () => {
  const readme = hostTable(README);
  const ch17 = hostTable(CH17);
  assert.ok(readme, 'README.md has no host table');
  assert.ok(ch17, 'chapter 17 has no host table');
  assert.deepEqual(readme, ch17, 'the two host tables differ — they must be the same rows in the same order');
});

test('the table names Cubase as supported on both platforms, with a version', () => {
  const rows = (hostTable(README) ?? []).map(cells);
  for (const platform of ['macOS', 'Windows']) {
    const row = rows.find((r) => r[0] === 'Cubase' && r[2] === platform);
    assert.ok(row, `no Cubase row for ${platform}`);
    assert.equal(row[3], 'supported', `Cubase on ${platform} must be supported`);
    assert.ok(row[1].length > 0, `Cubase on ${platform} carries no version`);
  }
});

test('every Status is supported or untested', () => {
  for (const row of (hostTable(README) ?? []).map(cells)) {
    assert.ok(['supported', 'untested'].includes(row[3]), `row ${row[0]} has status "${row[3]}"`);
  }
});

test('neither file claims any VST3 host works', () => {
  for (const [name, text] of [['README.md', README], ['chapter 17', CH17]]) {
    assert.doesNotMatch(text, /should work with any VST3/i, `${name} claims Poly should work with any VST3 host`);
    assert.doesNotMatch(text, /any VST3-compatible host/i, `${name} claims any VST3-compatible host`);
  }
});
