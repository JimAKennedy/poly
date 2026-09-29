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

// M003/S01, OS15: the per-lane channel default already does the right thing
// with a drum instrument that listens on all channels, so the chapter leads
// with that and keeps per-lane channel routing as the advanced case.
test('chapter 17 opens its routing with one instrument on all channels, before any per-lane channel setup', () => {
  const start = CH17.indexOf('### Start Here: One Drum Instrument');
  const advanced = CH17.indexOf('### Advanced: One Lane, One Channel');
  assert.notEqual(start, -1, 'chapter 17 has no "Start Here: One Drum Instrument" section');
  assert.notEqual(advanced, -1, 'chapter 17 has no "Advanced: One Lane, One Channel" section');
  assert.ok(start < advanced, 'the one-instrument section must come before the per-lane channel section');
  const section = CH17.slice(start, advanced);
  assert.match(section, /all (MIDI )?channels/i, 'the one-instrument section must say the instrument listens on all channels');
  assert.doesNotMatch(section, /set every lane.*channel 1/i, 'the one-instrument section must not tell the reader to set lanes to Channel 1');
});

// M003/S03, OS17: Logic is declined for the first release. The SDK's AUv2
// wrapper cannot produce the MIDI-FX unit Logic routes generated MIDI from,
// so the guide must not list Logic as a host that loads Poly, and both the
// README and the guide must say it is not supported.
const GUIDE = readFileSync(resolve(REPO, 'site/src/content/docs/guide-using-poly.mdx'), 'utf8');

test('the guide does not list Logic as a host that loads Poly', () => {
  const load = GUIDE.slice(GUIDE.indexOf('### Load Poly'));
  const list = load.slice(0, load.indexOf('\n\n', load.indexOf('\n- ')));
  assert.doesNotMatch(list, /^- \*\*Logic Pro:\*\* Track >/m, 'the Load Poly list still gives Logic loading steps');
  assert.doesNotMatch(GUIDE, /^\*\*Logic Pro:\*\*$/m, 'a Logic Pro routing block remains in the guide');
});

test('README and guide say Logic is not supported', () => {
  for (const [name, text] of [['README.md', README], ['guide-using-poly.mdx', GUIDE]]) {
    assert.match(text, /Logic Pro[^.\n]*not supported/i, `${name} does not say Logic Pro is not supported`);
  }
});

// M004/S04, OS41: the install section describes the zip a release actually
// contains — a poly_plugin.vst3 bundle — and not a Poly.vst3 or a
// Poly.component that no release has ever shipped.
test("the guide's install section names the bundle a release contains", () => {
  const install = GUIDE.slice(GUIDE.indexOf('### Install'), GUIDE.indexOf('### Load Poly'));
  assert.ok(install.includes('poly_plugin.vst3'), 'the install section does not name poly_plugin.vst3 (OS41)');
  assert.doesNotMatch(GUIDE, /Poly\.vst3\b/, 'the guide still names Poly.vst3, a bundle no release contains (OS41)');
  assert.doesNotMatch(GUIDE, /Poly\.component\b/, 'the guide still names Poly.component, a bundle no release contains (OS41)');
  assert.doesNotMatch(GUIDE, /ships in two formats/i, 'the guide still says Poly ships in two formats (OS41)');
});
