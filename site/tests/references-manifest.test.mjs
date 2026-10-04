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
  'retrieval',
];

async function loadManifest() {
  const raw = await readFile(MANIFEST_PATH, 'utf8');
  return JSON.parse(raw);
}

test('references manifest declares schemaVersion 2 and an entries object', async () => {
  const m = await loadManifest();
  assert.equal(m.schemaVersion, 2, 'schemaVersion must be 2 (M003/S01 added retrieval)');
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

// M002/S01 task 2: completeness, both directions.
//
// The anchor set is read from the .mdx at test time, never from a literal.
// A hardcoded 107 would agree with a stale manifest for exactly as long as
// nobody noticed — the failure presets-json-schema.test.mjs records this repo
// shipping once before.
//
// The regex must fail loudly if the bibliography's anchor convention changes:
// zero matches is a broken check, not a passing one.
// first-release M003/S02: there are now two bibliographies. The shipping
// appendix holds only what a shipping page cites; the deferred theory bundle
// carries what the deep dives cite. A verdict is about a work, not about which
// file lists it, and every verdict M002 recorded is still true -- so the
// manifest spans both and these arms check the union.
const BIBLIOGRAPHY_PATHS = [
  join(HERE, '..', 'src', 'content', 'docs', 'appendix-references.mdx'),
  join(HERE, '..', 'src', 'content', 'theory', 'theory-references.mdx'),
];

async function bibliographyAnchors() {
  const found = [];
  for (const path of BIBLIOGRAPHY_PATHS) {
    const mdx = await readFile(path, 'utf8');
    const here = [...mdx.matchAll(/id="((?:ref|fr)-[A-Za-z0-9-]+)"/g)].map((m) => m[1]);
    if (here.length === 0) {
      throw new Error(
        `no ref-/fr- anchors matched in ${path} — has the anchor convention ` +
          'changed, or has the file moved? An unmatched pattern is a broken ' +
          'check, not a passing one.',
      );
    }
    found.push(...here);
  }
  return found;
}

test('every bibliography anchor has a manifest record', async () => {
  const m = await loadManifest();
  const anchors = await bibliographyAnchors();
  const missing = anchors.filter((a) => !(a in m.entries));
  assert.deepEqual(
    missing,
    [],
    'cited in the bibliography but absent from the manifest, so no verdict can ' +
      'ever be recorded for it: ' + missing.join(', '),
  );
});

test('every manifest record names an anchor that exists', async () => {
  const m = await loadManifest();
  const anchors = new Set(await bibliographyAnchors());
  const orphans = Object.keys(m.entries).filter((a) => !anchors.has(a));
  assert.deepEqual(
    orphans,
    [],
    'recorded in the manifest but not present in the bibliography — a verdict ' +
      'about nothing, or an anchor that was renamed without the manifest ' +
      'following: ' + orphans.join(', '),
  );
});

// M002/S01 task 3 (VR05): the archive root is never written to a tracked file.
//
// check-personal-paths rejected the absolute archive path in the vision's own
// first draft. The fix is structural rather than editorial: the manifest stores
// a bare filename, and the root arrives from the environment at read time, so
// there is no form of this data that could carry someone's home directory into
// git.
import { archiveRoot, resolveArchiveFile } from '../src/data/references-archive.mjs';

test('archiveRoot honours POLY_REFERENCES_ARCHIVE when set', () => {
  const prev = process.env.POLY_REFERENCES_ARCHIVE;
  try {
    process.env.POLY_REFERENCES_ARCHIVE = '/tmp/poly-refs-test';
    assert.equal(archiveRoot(), '/tmp/poly-refs-test');
  } finally {
    if (prev === undefined) delete process.env.POLY_REFERENCES_ARCHIVE;
    else process.env.POLY_REFERENCES_ARCHIVE = prev;
  }
});

test('archiveRoot falls back to a gitignored .references directory', () => {
  const prev = process.env.POLY_REFERENCES_ARCHIVE;
  try {
    delete process.env.POLY_REFERENCES_ARCHIVE;
    assert.ok(
      archiveRoot().endsWith('.references'),
      `expected a .references fallback, got ${archiveRoot()}`,
    );
    process.env.POLY_REFERENCES_ARCHIVE = '   ';
    assert.ok(
      archiveRoot().endsWith('.references'),
      'a whitespace-only variable must fall back, not resolve to an empty root',
    );
  } finally {
    if (prev === undefined) delete process.env.POLY_REFERENCES_ARCHIVE;
    else process.env.POLY_REFERENCES_ARCHIVE = prev;
  }
});

test('every archiveFile in the manifest is a bare filename', async () => {
  const m = await loadManifest();
  const offenders = Object.entries(m.entries)
    .filter(([, r]) => typeof r.archiveFile === 'string')
    .filter(([, r]) => /[\\/]/.test(r.archiveFile) || r.archiveFile.split(/[\\/]/).includes('..'))
    .map(([a, r]) => `${a} -> ${r.archiveFile}`);
  assert.deepEqual(
    offenders,
    [],
    'archiveFile must be a bare filename: a path in a tracked file is how a ' +
      "machine-specific root gets committed: " + offenders.join(', '),
  );
});

test('resolveArchiveFile refuses to escape the archive root', () => {
  for (const bad of ['../etc/passwd', 'sub/dir.pdf', '..', '', 'a\\b.pdf']) {
    assert.throws(
      () => resolveArchiveFile(bad),
      /bare filename/,
      `resolveArchiveFile(${JSON.stringify(bad)}) should have thrown`,
    );
  }
  const ok = resolveArchiveFile('ref-9-oluranti-2012.pdf');
  assert.ok(ok.endsWith('ref-9-oluranti-2012.pdf'));
  assert.ok(ok.startsWith(archiveRoot()));
});

// M002/S02 task 8: an entry nobody could verify must be queued for someone who
// can. This is the generalised form of the ref-22 guard M001/S01 added: that one
// hardcoded a single host and a single anchor, and was dominated by
// REF22-CLAYTON the moment the host was forbidden tree-wide. This case is keyed
// on the manifest's own verdicts, so it applies to every entry that reaches the
// same state and keeps applying as entries move in and out of it.
//
// The pairing that matters is browser-only AND unverified: obtainable by a
// human, not obtained by us. An entry that is unverified because it is
// purchasable or library-only is not something a browser session can settle, so
// queueing it would fill the worklist with work nobody can do.
const WORKLIST_PATH = join(
  HERE, '..', '..', 'docs', 'plans', 'verifiable-references', 'browser-worklist.md',
);

test('every browser-only unverified entry is named in the browser worklist', async () => {
  const m = await loadManifest();
  const queueable = Object.entries(m.entries)
    .filter(([, r]) => r.obtainability === 'browser-only' && r.description === 'unverified')
    .map(([a]) => a);

  let worklist;
  try {
    worklist = await readFile(WORKLIST_PATH, 'utf8');
  } catch {
    assert.fail(
      `${queueable.length} entries are browser-only and unverified, but ` +
        `${WORKLIST_PATH} does not exist. An entry a human could settle and ` +
        'nobody has been asked to settle is indistinguishable from one that is fine.',
    );
  }

  const missing = queueable.filter((a) => !worklist.includes(a));
  assert.deepEqual(
    missing,
    [],
    'browser-only and unverified, but not named in the worklist, so no one has ' +
      'been asked to look: ' + missing.join(', '),
  );
});

// verifiable-references M003/S01 task 1: retrieval state.
//
// VR10. `archiveFile: null` alone could mean "never tried", "tried and the
// host refused" or "there is nothing to archive", and those call for different
// work. Every record now says which, and `archived` is tied to `archiveFile`
// in both directions so the two can never disagree.
import { RETRIEVAL_STATUS } from '../src/data/references-archive.mjs';

test('RETRIEVAL_STATUS is the seven declared states', () => {
  assert.ok(
    Array.isArray(RETRIEVAL_STATUS) && RETRIEVAL_STATUS.length === 7,
    'RETRIEVAL_STATUS must be an array of seven states — a renamed or missing ' +
      'export would make every status check below assert nothing',
  );
});

test('every record carries a well-formed retrieval state', async () => {
  const m = await loadManifest();
  const bad = [];
  for (const [anchor, r] of Object.entries(m.entries)) {
    const ret = r.retrieval;
    if (Object.prototype.toString.call(ret) !== '[object Object]') {
      bad.push(`${anchor}: retrieval must be an object`);
      continue;
    }
    const keys = Object.keys(ret).sort();
    if (JSON.stringify(keys) !== JSON.stringify(['checked', 'detail', 'status'])) {
      bad.push(`${anchor}: retrieval keys are ${keys.join(', ')}`);
    }
    if (!RETRIEVAL_STATUS.includes(ret.status)) {
      bad.push(`${anchor}: retrieval.status ${JSON.stringify(ret.status)} is not one of ${RETRIEVAL_STATUS.join(', ')}`);
    }
    if (!(ret.checked === null || (typeof ret.checked === 'string' && /^\d{4}-\d{2}-\d{2}$/.test(ret.checked)))) {
      bad.push(`${anchor}: retrieval.checked must be null or YYYY-MM-DD`);
    }
    if (!(ret.detail === null || typeof ret.detail === 'string')) {
      bad.push(`${anchor}: retrieval.detail must be null or a string`);
    }
  }
  assert.deepEqual(bad, [], bad.join('\n'));
});

test('an archived record names its file', async () => {
  const m = await loadManifest();
  const offenders = Object.entries(m.entries)
    .filter(([, r]) => r.retrieval?.status === 'archived' && typeof r.archiveFile !== 'string')
    .map(([a]) => a);
  assert.deepEqual(offenders, [], 'retrieval says archived but no archiveFile is named: ' + offenders.join(', '));
});

test('a record that names a file is archived', async () => {
  const m = await loadManifest();
  const offenders = Object.entries(m.entries)
    .filter(([, r]) => typeof r.archiveFile === 'string' && r.retrieval?.status !== 'archived')
    .map(([a]) => a);
  assert.deepEqual(offenders, [], 'archiveFile is named but retrieval does not say archived: ' + offenders.join(', '));
});

// verifiable-references M003/S01 task 3: every archived file is named by the
// convention (VR09). A file saved under whatever name its host gave it
// (`cohnreich1992.pdf`) is findable only by whoever saved it.
import { archiveFileName } from '../src/data/references-archive.mjs';
import { readBibliography } from '../src/data/references-bibliography.mjs';

test('every archived file follows the naming convention', async () => {
  const m = await loadManifest();
  const bib = await readBibliography();
  const off = Object.entries(m.entries)
    .filter(([, r]) => typeof r.archiveFile === 'string')
    .map(([a, r]) => [a, r.archiveFile, archiveFileName(a, bib.get(a)?.text ?? '')])
    .filter(([, actual, expected]) => actual !== expected)
    .map(([a, actual, expected]) => `${a}: ${actual} → ${expected}`);
  assert.deepEqual(off, [], 'archived under a name other than the convention gives:\n' + off.join('\n'));
});

// verifiable-references M003/S01 task 4: a refusal is a queue entry. The
// DoD's third box — sources that resist scripted fetching are recorded as
// such, not retried silently — means a `script-refused` record must have been
// handed to a person, which is what the worklist's Pending section is.
export function pendingSection(worklist) {
  const start = worklist.indexOf('\n## Pending');
  const end = worklist.indexOf('\n## Resolved');
  if (start < 0 || end < 0 || end < start) {
    throw new Error('browser-worklist.md must have ## Pending before ## Resolved');
  }
  return worklist.slice(start, end);
}

export function refusedNotQueued(manifest, worklist) {
  const pending = pendingSection(worklist);
  return Object.entries(manifest.entries)
    .filter(([, r]) => r.retrieval?.status === 'script-refused')
    .map(([a]) => a)
    .filter((a) => !pending.includes(`\`${a}\``));
}

test('every script-refused entry is queued in the worklist', async () => {
  const m = await loadManifest();
  const worklist = await readFile(WORKLIST_PATH, 'utf8');
  const missing = refusedNotQueued(m, worklist);
  assert.deepEqual(missing, [], 'refused by the script and not queued for a person: ' + missing.join(', '));
});

test('the refusal guard fires on an unqueued refusal', async () => {
  const m = structuredClone(await loadManifest());
  const worklist = await readFile(WORKLIST_PATH, 'utf8');
  m.entries['ref-1'].retrieval = { status: 'script-refused', checked: '2026-09-30', detail: 'HTTP 403' };
  assert.deepEqual(refusedNotQueued(m, worklist), ['ref-1'], 'a check that cannot fire is not a check');
});

// verifiable-references M003/S02 task 1 (VR10): every pending worklist row
// tells the person working it exactly what to save it as. A file saved under
// the host's own name has to be found and renamed afterwards, which is the
// state the archive was in before M003. Video rows have nothing to save and
// carry a fixed marker instead.
const NO_TEXT_MARK = '— no text; record existence only —';

export function pendingRows(worklist) {
  const rows = [];
  let header = null;
  for (const line of pendingSection(worklist).split('\n')) {
    if (!line.startsWith('|')) { header = null; continue; }
    const cells = line.split('|').slice(1, -1).map((c) => c.trim());
    if (header === null) { header = cells; continue; }
    if (cells.every((c) => /^-+$/.test(c))) continue;
    const anchor = /^`((?:ref|fr)-[a-z0-9-]+)`$/.exec(cells[0])?.[1];
    if (!anchor) continue;
    rows.push({ anchor, cell: Object.fromEntries(header.map((h, i) => [h, cells[i] ?? ''])) });
  }
  return rows;
}

test('every pending worklist row names its URL, the task, and the exact file to save', async () => {
  const m = await loadManifest();
  const bib = await readBibliography();
  const worklist = await readFile(WORKLIST_PATH, 'utf8');
  const rows = pendingRows(worklist);
  // Zero rows is a pass only when the section says so in words; otherwise it
  // means the table shape changed and this check stopped seeing anything.
  if (rows.length === 0) {
    assert.match(pendingSection(worklist), /Nothing is pending\./, 'no pending rows parsed — has the table shape changed?');
    return;
  }
  const bad = [];
  for (const { anchor, cell } of rows) {
    const url = cell.URL ?? '';
    const task = cell['What to check'] ?? cell['What to save'] ?? '';
    const saveAs = cell['Save as'];
    if (url === '') bad.push(`${anchor}: empty URL cell`);
    if (task === '') bad.push(`${anchor}: no instruction`);
    if (saveAs === undefined) { bad.push(`${anchor}: no Save as column`); continue; }
    const video = /youtube\.com/.test(url) && m.entries[anchor]?.obtainability === 'browser-only';
    const want = video ? NO_TEXT_MARK : '`' + archiveFileName(anchor, bib.get(anchor)?.text ?? '') + '`';
    if (saveAs !== want) bad.push(`${anchor}: Save as is ${saveAs}, want ${want}`);
  }
  assert.deepEqual(bad, [], bad.join('\n'));
});

// verifiable-references M003/S02 task 3 (VR10): the archive is finished. Every
// source a person can reach for free is either in the archive or recorded as
// what it is — a scan, an institution's, a video, or one the owner judged
// unsuitable and M004 replaces. Neither "not tried" nor "refused" is an end
// state: both mean nobody has finished looking.
const END_STATES = ['archived', 'scan-only', 'institution-only', 'no-text', 'to-replace'];

test('every free source is archived or recorded as what it is', async () => {
  const m = await loadManifest();
  const open = Object.entries(m.entries)
    .filter(([, r]) => ['open-access', 'browser-only'].includes(r.obtainability))
    .filter(([, r]) => !END_STATES.includes(r.retrieval?.status))
    .map(([a, r]) => `${a} (${r.obtainability}, ${r.retrieval?.status})`);
  assert.deepEqual(open, [], 'free, but neither archived nor recorded as unarchivable:\n' + open.join('\n'));
});

// verifiable-references M004/S02 (VR13): every source the owner judged
// unsuitable in M003 has been replaced or dropped. `to-replace` is a hand-off
// state, and a hand-off nobody picked up is the failure this closes.
test('no record is waiting to be replaced', async () => {
  const m = await loadManifest();
  const waiting = Object.entries(m.entries)
    .filter(([, r]) => r.retrieval?.status === 'to-replace')
    .map(([a]) => a);
  assert.deepEqual(waiting, [], 'judged unsuitable and still cited: ' + waiting.join(', '));
});

// verifiable-references M004/S02 task 4: every description mismatch M002 and
// M003 found has been corrected. A new one would mean a citation describes a
// different work from the one it links — the defect this programme exists for.
test('no citation describes a different work from the one it links', async () => {
  const m = await loadManifest();
  const off = Object.entries(m.entries).filter(([, r]) => r.description === 'mismatch').map(([a]) => a);
  assert.deepEqual(off, [], 'still a mismatch: ' + off.join(', '));
});

// The kept non-scholarly entries say why they stay (VR13: "kept with a
// recorded reason"). Named, not inferred: these are the three the owner kept.
test('every kept non-scholarly entry records why it stays', async () => {
  const m = await loadManifest();
  const kept = ['ref-20', 'ref-42', 'fr-linn-attack-2020'];
  const silent = kept.filter((a) => !/Kept \(VR13\):/.test(m.entries[a]?.note ?? ''));
  assert.deepEqual(silent, [], 'kept without a recorded reason: ' + silent.join(', '));
});

// verifiable-references M005/S02 (VR15): each bibliography is one list. Before
// M005 each was a numbered section followed by a Further Reading section with
// its own chapter grouping, so a reader had to know which list a source used
// to be in. Now every entry is a list item under one heading per group.
const ONE_LIST_FILES = [
  join(HERE, '..', 'src', 'content', 'docs', 'appendix-references.mdx'),
  join(HERE, '..', 'src', 'content', 'theory', 'theory-references.mdx'),
];

export function oneListProblems(mdx) {
  const body = mdx.replace(/^---\n[\s\S]*?\n---\n/, '');
  const lines = body.split('\n');
  const problems = [];
  const firstH2 = lines.findIndex((l) => l.startsWith('## '));
  if (firstH2 < 0) problems.push('no ## heading');
  const headings = lines.filter((l) => /^#{2,3} /.test(l));
  const seen = new Set();
  for (const h of headings) {
    if (h.startsWith('### ')) problems.push(`a sub-list heading remains: ${h}`);
    if (seen.has(h)) problems.push(`heading repeats: ${h}`);
    seen.add(h);
  }
  lines.forEach((l, i) => {
    if (l === '---') problems.push(`a section separator remains at body line ${i + 1}`);
    if (/id="(?:ref|fr)-/.test(l)) {
      if (!l.startsWith('- <span id=')) problems.push(`not a list item: ${l.slice(0, 60)}`);
      if (i < firstH2) problems.push(`entry before the first heading: ${l.slice(0, 60)}`);
    }
  });
  return problems;
}

test('each bibliography is one list, grouped once', async () => {
  const all = [];
  for (const f of ONE_LIST_FILES) {
    for (const p of oneListProblems(await readFile(f, 'utf8'))) all.push(`${f.split('/').pop()}: ${p}`);
  }
  assert.deepEqual(all, [], all.join('\n'));
});

// verifiable-references M005/S02 task 3 (VR15): every entry carries the same
// fields. Tier is the `data-tier` attribute; access and route are visible, so
// a reader can tell from the entry itself whether and how to get the source.
// Both are checked against the manifest, so the page cannot drift from the
// verdicts behind it.
const ACCESS_LABEL = {
  'open-access': 'Open access',
  'browser-only': 'Free online',
  borrowable: 'Borrowable',
  purchasable: 'Purchasable',
  'library-only': 'Library',
};
// The owner is finding Novotney's record (ProQuest, IDEALS or WorldCat):
// M005/S02 task 4 removes this exemption. Nothing else may be listed here.
const PENDING_ROUTE = ['fr-novotney-1998'];

test('every entry shows its route and access', async () => {
  const m = await loadManifest();
  const bad = [];
  for (const f of ONE_LIST_FILES) {
    const name = f.split('/').pop();
    for (const line of (await readFile(f, 'utf8')).split('\n')) {
      const anchor = /id="((?:ref|fr)-[a-z0-9-]+)"/.exec(line)?.[1];
      if (!anchor) continue;
      const rec = m.entries[anchor];
      const want = ACCESS_LABEL[rec?.obtainability];
      if (!want) { bad.push(`${name} ${anchor}: no access label for obtainability ${rec?.obtainability}`); continue; }
      const labels = [...line.matchAll(/\*\(([^)]+)\)\*/g)].map((x) => x[1]).filter((l) => Object.values(ACCESS_LABEL).includes(l));
      if (labels.length !== 1 || labels[0] !== want) bad.push(`${name} ${anchor}: access label ${JSON.stringify(labels)}, want ["${want}"]`);
      const routed = /\]\(https?:\/\/[^)\s]+\)/.test(line) || /ISBN [0-9X-]{10,}/.test(line);
      if (PENDING_ROUTE.includes(anchor)) {
        if (routed) bad.push(`${name} ${anchor}: has a route now — remove it from PENDING_ROUTE`);
      } else if (!routed) {
        bad.push(`${name} ${anchor}: no route (a link, a DOI link, or an ISBN)`);
      }
    }
  }
  assert.deepEqual(bad, [], bad.join('\n'));
});
