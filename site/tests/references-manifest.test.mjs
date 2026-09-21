// M002/S01: the references manifest's declared shape.
//
// VR04. Nothing in this repo recorded whether a citation had been checked, so
// silence implied "checked" — an unexamined entry and a verified one looked
// identical. The manifest makes the absence of a verdict explicit: every anchor
// has a record, and a record that has not been assessed says so.
//
// The manifest is keyed by anchor rather than an array of records, so a
// duplicate anchor is impossible by construction rather than something a test
// has to exclude.
//
// Following presets-json-schema.test.mjs on one point that matters: a lookup
// that stops matching must throw rather than silently assert nothing. An enum
// or an anchor pattern that has been renamed is a broken check, not a passing
// one.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const HERE = dirname(fileURLToPath(import.meta.url));
const MANIFEST_PATH = join(HERE, '..', 'src', 'data', 'references.json');

const OBTAINABILITY = [
  'open-access',
  'borrowable',
  'purchasable',
  'library-only',
  'browser-only',
  'unobtainable',
  'unassessed',
];

const DESCRIPTION = ['verified', 'mismatch', 'unverified'];

const RECORD_KEYS = [
  'obtainability',
  'evidence',
  'checked',
  'description',
  'note',
  'identifier',
  'price',
  'archiveFile',
];

async function loadManifest() {
  const raw = await readFile(MANIFEST_PATH, 'utf8');
  return JSON.parse(raw);
}

test('references manifest declares schemaVersion 1 and an entries object', async () => {
  const m = await loadManifest();
  assert.equal(m.schemaVersion, 1, 'schemaVersion must be 1');
  assert.equal(
    Object.prototype.toString.call(m.entries),
    '[object Object]',
    'entries must be a plain object keyed by anchor id, not an array — keying is ' +
      'what makes a duplicate anchor impossible rather than merely detectable',
  );
});

test('every manifest record carries exactly the declared keys and types', async () => {
  const m = await loadManifest();
  const anchors = Object.keys(m.entries);
  assert.ok(anchors.length > 0, 'manifest has no records at all');

  for (const [anchor, rec] of Object.entries(m.entries)) {
    const keys = Object.keys(rec).sort();
    assert.deepEqual(
      keys,
      [...RECORD_KEYS].sort(),
      `${anchor}: record keys do not match the declared shape`,
    );
    assert.ok(
      OBTAINABILITY.includes(rec.obtainability),
      `${anchor}: obtainability ${JSON.stringify(rec.obtainability)} is not one of ` +
        OBTAINABILITY.join(', '),
    );
    assert.ok(
      DESCRIPTION.includes(rec.description),
      `${anchor}: description ${JSON.stringify(rec.description)} is not one of ` +
        DESCRIPTION.join(', '),
    );
    assert.equal(typeof rec.evidence, 'string', `${anchor}: evidence must be a string`);
    assert.equal(typeof rec.note, 'string', `${anchor}: note must be a string`);
    assert.ok(
      rec.archiveFile === null || typeof rec.archiveFile === 'string',
      `${anchor}: archiveFile must be a string or null`,
    );
    assert.ok(
      rec.identifier === null ||
        (Object.prototype.toString.call(rec.identifier) === '[object Object]' &&
          ['doi', 'isbn'].every((k) => k in rec.identifier)),
      `${anchor}: identifier must be null or an object with doi and isbn keys`,
    );
  }
});

// The three conditional requirements. Each exists because a verdict without its
// supporting field is not a decision anyone downstream can act on.
test('a purchasable verdict carries a price', async () => {
  const m = await loadManifest();
  const offenders = Object.entries(m.entries)
    .filter(([, r]) => r.obtainability === 'purchasable' && !r.price)
    .map(([a]) => a);
  assert.deepEqual(
    offenders,
    [],
    'purchasable without a price is not a decision anyone can make (VR06): ' +
      offenders.join(', '),
  );
});

test('a mismatch verdict records what the source actually is', async () => {
  const m = await loadManifest();
  const offenders = Object.entries(m.entries)
    .filter(([, r]) => r.description === 'mismatch' && r.note.trim() === '')
    .map(([a]) => a);
  assert.deepEqual(
    offenders,
    [],
    'a mismatch with an empty note forces M004 to repeat the work (VR07): ' +
      offenders.join(', '),
  );
});

test('an assessed entry carries a check date, an unassessed one does not', async () => {
  const m = await loadManifest();
  const bad = [];
  for (const [anchor, r] of Object.entries(m.entries)) {
    const assessed = r.obtainability !== 'unassessed';
    const dated = typeof r.checked === 'string' && /^\d{4}-\d{2}-\d{2}$/.test(r.checked);
    if (assessed && !dated) bad.push(`${anchor} (assessed, checked=${JSON.stringify(r.checked)})`);
    if (!assessed && r.checked !== null) bad.push(`${anchor} (unassessed, checked=${JSON.stringify(r.checked)})`);
  }
  assert.deepEqual(bad, [], 'check-date rule violated: ' + bad.join('; '));
});
