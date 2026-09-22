// first-release M003/S03, FR20. The shipping appendix was reduced to the entries
// its own chapters cite (M003/S02), every one of them Tier A or B and obtainable
// without an institution. Nothing else holds that state: research_provenance
// checks that a citation resolves to an anchor, citation-tier checks a declared
// tier exists, and the manifest completeness test checks every anchor has a
// record. None of them fails when an uncited, Tier C or library-only entry is
// added back, so the 41-entry state would hold only until the next edit.
//
// Three properties, never a count — a guard pinned to 41 would fail the day an
// entry is legitimately added and teach people to edit the number. Empty inputs
// throw rather than pass: a zero-length anchor set or an unreadable manifest is a
// broken check, not a green one, which this repo has shipped once before
// (presets-json-schema.test.mjs records it).
//
// No exemptions. The one candidate — an uncited, unread, library-only entry
// that declared its contents unverified — was retired in this same slice rather
// than carved out, so the three properties hold for every entry without a
// carve-out for any (M003-decisions.md, 2026-09-22).

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile, readdir } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const HERE = dirname(fileURLToPath(import.meta.url));
const DOCS = join(HERE, '..', 'src', 'content', 'docs');
const APPENDIX_NAME = 'appendix-references.mdx';
const MANIFEST_PATH = join(HERE, '..', 'src', 'data', 'references.json');

const ENTRY = /<span id="((?:ref|fr)-[A-Za-z0-9-]+)"([^>]*)>/g;
const TIER = /data-tier="([^"]*)"/;
const CITATION = /appendix-references\/#((?:ref|fr)-[A-Za-z0-9-]+)/g;
const ALLOWED_TIERS = new Set(['A', 'B']);

async function appendixEntries() {
  const src = await readFile(join(DOCS, APPENDIX_NAME), 'utf8');
  const entries = [...src.matchAll(ENTRY)].map((m) => ({
    anchor: m[1],
    tier: (m[2].match(TIER) ?? [])[1] ?? null,
  }));
  if (entries.length === 0) {
    throw new Error(
      `${APPENDIX_NAME} defines no entries — has its markup changed? An empty ` +
        'set would let every assertion below pass while checking nothing.',
    );
  }
  return entries;
}

// Every anchor cited from a shipping page other than the appendix itself. The
// theory bundle is deliberately not read: its entries are governed by
// theory-bundle-references.test.mjs, and a guard that scanned both would pass
// when a shipping entry was cited only from an unpublished page.
async function citedAnchors() {
  const files = (await readdir(DOCS)).filter((f) => f.endsWith('.mdx') && f !== APPENDIX_NAME);
  if (files.length === 0) {
    throw new Error(`no shipping pages found in ${DOCS} — have they moved?`);
  }
  const cited = new Set();
  for (const f of files) {
    const src = await readFile(join(DOCS, f), 'utf8');
    for (const m of src.matchAll(CITATION)) cited.add(m[1]);
  }
  if (cited.size === 0) {
    throw new Error(
      'no shipping page cites any appendix anchor — has the citation link form ' +
        'changed? The cited arm would fail every entry rather than report drift.',
    );
  }
  return cited;
}

async function loadManifest() {
  const m = JSON.parse(await readFile(MANIFEST_PATH, 'utf8'));
  if (!m || typeof m.entries !== 'object' || Object.keys(m.entries).length === 0) {
    throw new Error(`${MANIFEST_PATH} has no entries — the obtainable arm has nothing to read`);
  }
  return m.entries;
}

test('FR20 cited: every appendix entry is cited from a shipping page', async () => {
  const entries = await appendixEntries();
  const cited = await citedAnchors();
  const uncited = entries.filter((e) => !cited.has(e.anchor)).map((e) => e.anchor);
  assert.deepEqual(
    uncited,
    [],
    'defined in the appendix but cited by no shipping page. Either cite it from ' +
      'a claim, move it to the theory bundle, or retire it with the reason recorded',
  );
});

test('FR20 tier: every appendix entry is Tier A or B', async () => {
  const entries = await appendixEntries();
  const bad = entries.filter((e) => !ALLOWED_TIERS.has(e.tier)).map((e) => `${e.anchor} (${e.tier ?? 'no tier'})`);
  assert.deepEqual(
    bad,
    [],
    'Tier C or untiered in the shipping appendix. Tier C marks material with no ' +
      'text to review — YouTube, marketing blogs, study guides — and every such ' +
      'entry was cited only from the deep dives, which are unpublished. B is ' +
      'allowed for a primary source such as an interview (fr-linn-attack-2020)',
  );
});

test('FR20 obtainable: no appendix entry is library-only', async () => {
  const entries = await appendixEntries();
  const manifest = await loadManifest();
  const bad = [];
  for (const e of entries) {
    const rec = manifest[e.anchor];
    if (!rec) {
      // The completeness test in references-manifest.test.mjs reports this by
      // name; here it is a failure rather than a skip so the arm cannot go
      // green by an entry simply having no record to check.
      bad.push(`${e.anchor} (no manifest record)`);
      continue;
    }
    if (rec.obtainability === 'library-only') bad.push(`${e.anchor} (library-only)`);
  }
  assert.deepEqual(
    bad,
    [],
    'needs an institution, which fails the programme\'s own test of "a reader ' +
      'can get it". Find an obtainable route (M003/S02 found one for all six it ' +
      'tried), or reword the citing claim so it no longer depends on the entry',
  );
});
