// verifiable-references M003/S01 task 2: the retrieval script.
//
// The DoD's second box: the retrieval reads its destination from the
// environment and fails with a clear message when unset, rather than writing
// somewhere arbitrary. The read-side helper falls back to a gitignored
// .references/ so tests run anywhere; a writer that did the same would put
// copyrighted PDFs wherever the repo happened to be checked out. So the
// script has no fallback, and these cases prove it refuses.
//
// No case here touches the network: --dry-run selects candidates and stops.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { mkdtempSync, readdirSync, readFileSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

import { isChallengePage, selectCandidates } from '../../scripts/fetch-references.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = join(HERE, '..', '..');
const SCRIPT = join(REPO, 'scripts', 'fetch-references.mjs');
const MANIFEST = join(REPO, 'site', 'src', 'data', 'references.json');

function run(args, archive) {
  const env = { ...process.env };
  delete env.POLY_REFERENCES_ARCHIVE;
  if (archive !== undefined) env.POLY_REFERENCES_ARCHIVE = archive;
  return spawnSync(process.execPath, [SCRIPT, ...args], { env, encoding: 'utf8', cwd: REPO });
}

test('unset POLY_REFERENCES_ARCHIVE exits 2 and says there is no default', () => {
  const before = readFileSync(MANIFEST, 'utf8');
  const r = run(['--dry-run']);
  assert.equal(r.status, 2, `expected exit 2, got ${r.status}; stderr: ${r.stderr}`);
  assert.match(r.stderr, /POLY_REFERENCES_ARCHIVE/);
  assert.match(r.stderr, /no default|not fall back/i);
  assert.equal(readFileSync(MANIFEST, 'utf8'), before, 'the manifest must not be touched');
});

test('a whitespace-only value is refused like an unset one', () => {
  const r = run(['--dry-run'], '   ');
  assert.equal(r.status, 2);
  assert.match(r.stderr, /POLY_REFERENCES_ARCHIVE/);
});

test('a directory that does not exist is refused, and named', () => {
  const missing = join(tmpdir(), 'poly-refs-does-not-exist-' + process.pid);
  const r = run(['--dry-run'], missing);
  assert.equal(r.status, 2);
  assert.ok(r.stderr.includes(missing), `stderr should name ${missing}: ${r.stderr}`);
});

test('--dry-run lists candidates and writes nothing', () => {
  const dir = mkdtempSync(join(tmpdir(), 'poly-refs-'));
  try {
    const before = readFileSync(MANIFEST, 'utf8');
    const r = run(['--dry-run'], dir);
    assert.equal(r.status, 0, r.stderr);
    assert.match(r.stdout, /candidate/i);
    assert.deepEqual(readdirSync(dir), [], 'the archive must be untouched');
    assert.equal(readFileSync(MANIFEST, 'utf8'), before, 'the manifest must be untouched');
  } finally {
    rmSync(dir, { recursive: true, force: true });
  }
});

test('isChallengePage recognises bot challenges and passes an article', () => {
  assert.equal(isChallengePage('<title>Making sure you&#39;re not a bot!</title><script src="/.within.website/x/cmd/anubis/static/js/main.mjs">'), true);
  assert.equal(isChallengePage('<title>Just a moment...</title>'), true);
  assert.equal(isChallengePage('<form id="challenge-form" action="/?__cf_chl_f_tk=abc">'), true);
  assert.equal(isChallengePage('<div class="g-recaptcha" data-sitekey="x"></div> please complete the captcha'), true);
  assert.equal(
    isChallengePage('<html><head><title>Rhythm in Sub-Saharan Africa - Wikipedia</title></head><body><p>Rhythm…</p></body></html>'),
    false,
  );
});

const record = (over) => ({
  obtainability: 'open-access',
  identifier: { doi: null, isbn: null },
  archiveFile: null,
  retrieval: { status: 'not-attempted', checked: null, detail: null },
  ...over,
});

test('selectCandidates takes untried open-access entries with a route', () => {
  const manifest = {
    entries: {
      'ref-1': record({}),
      'ref-2': record({ archiveFile: '02 - Goldberg.pdf', retrieval: { status: 'archived', checked: '2026-09-20', detail: 'x' } }),
      'ref-3': record({ retrieval: { status: 'script-refused', checked: '2026-09-30', detail: 'HTTP 403' } }),
      'ref-5': record({ obtainability: 'purchasable' }),
      'ref-6': record({ obtainability: 'browser-only' }),
      'fr-anku-2000': record({ identifier: { doi: '10.30535/mto.6.1.2', isbn: null } }),
      'fr-linn-attack-2020': record({}),
    },
  };
  const bib = new Map([
    ['ref-1', { text: 'Toussaint, G. T.', url: 'https://example.org/a.pdf' }],
    ['ref-2', { text: 'Goldberg, D.', url: 'https://example.org/b.pdf' }],
    ['ref-3', { text: 'Someone, A.', url: 'https://example.org/c.pdf' }],
    ['ref-5', { text: 'Book, A.', url: 'https://example.org/d' }],
    ['ref-6', { text: 'Blocked, A.', url: 'https://example.org/e' }],
    ['fr-anku-2000', { text: 'Anku, W.', url: null }],
    ['fr-linn-attack-2020', { text: 'Linn, R.', url: null }],
  ]);

  const got = selectCandidates(manifest, bib, { retry: false });
  assert.deepEqual(
    got.map((c) => [c.anchor, c.url]),
    [
      ['ref-1', 'https://example.org/a.pdf'],
      ['fr-anku-2000', 'https://doi.org/10.30535/mto.6.1.2'],
    ],
    'archived, refused, non-open-access and routeless entries are skipped; a DOI stands in for a missing link',
  );

  const retried = selectCandidates(manifest, bib, { retry: true }).map((c) => c.anchor);
  assert.ok(retried.includes('ref-3'), '--retry brings a refused entry back');
});
