// reference-links.test.mjs — the dead-reference checker's classes, proved
// against a local HTTP server (verifiable-references M006/S01, VR16).
//
// The slice's second DoD item is the one that matters most: the browser-only
// class — hosts that refuse scripts but serve a person — must never be
// reported as dead. Every status the decisions file names has a route here,
// so each class is proved by a real response rather than a mocked one.
//
// Run: `node --test scripts/reference-links.test.mjs` (check-guards.sh).

import { test, before, after } from 'node:test';
import assert from 'node:assert/strict';
import { createServer } from 'node:http';

import { checkUrl } from './reference-links.mjs';

let server;
let base;
let closedPort;

const ARTICLE = '<html><head><title>An article</title></head><body>' + '<p>text</p>'.repeat(50) + '</body></html>';
const CHALLENGE = '<html><head><title>Making sure you&#39;re not a bot!</title></head><body><script id="anubis_version"></script></body></html>';

before(async () => {
  server = createServer((req, res) => {
    const route = req.url;
    if (route === '/ok') return res.writeHead(200, { 'content-type': 'text/html' }).end(ARTICLE);
    if (route === '/challenge') return res.writeHead(200, { 'content-type': 'text/html' }).end(CHALLENGE);
    if (route === '/moved') return res.writeHead(301, { location: '/ok' }).end();
    if (route === '/hang') return; // never answers
    const status = Number(route.slice(1));
    res.writeHead(status, { 'content-type': 'text/plain' }).end(String(status));
  });
  await new Promise((r) => server.listen(0, '127.0.0.1', r));
  base = `http://127.0.0.1:${server.address().port}`;
  // A port that was open and is now closed refuses connections.
  const probe = createServer();
  await new Promise((r) => probe.listen(0, '127.0.0.1', r));
  closedPort = probe.address().port;
  await new Promise((r) => probe.close(r));
});

after(async () => {
  server.closeAllConnections?.();
  await new Promise((r) => server.close(r));
});

const cases = [
  ['/ok', 'ok', 'a 200 article'],
  ['/challenge', 'blocked', 'a 200 that is a bot challenge'],
  ['/moved', 'ok', 'a redirect to a 200'],
  ['/401', 'blocked', '401'],
  ['/403', 'blocked', '403 — the browser-only class'],
  ['/404', 'dead', '404'],
  ['/406', 'blocked', '406 — the browser-only class'],
  ['/410', 'dead', '410'],
  ['/429', 'blocked', '429'],
  ['/500', 'error', 'a server error, usually transient'],
];

for (const [route, want, why] of cases) {
  test(`${why} is ${want}`, async () => {
    const got = await checkUrl(base + route, { timeoutMs: 500 });
    assert.equal(got.class, want, `${route}: ${JSON.stringify(got)}`);
  });
}

test('no response within the timeout is dead, after one retry', async () => {
  const got = await checkUrl(base + '/hang', { timeoutMs: 300 });
  assert.equal(got.class, 'dead', JSON.stringify(got));
  assert.equal(got.attempts, 2, 'a no-response must be retried once before it counts');
});

test('a refused connection is dead', async () => {
  const got = await checkUrl(`http://127.0.0.1:${closedPort}/`, { timeoutMs: 500 });
  assert.equal(got.class, 'dead', JSON.stringify(got));
});

// ---- task 2: collecting URLs, and the command line ------------------------

import { spawnSync } from 'node:child_process';
import { mkdtempSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { collectUrls, readBibliographyTexts } from './reference-links.mjs';

const SCRIPT = join(dirname(fileURLToPath(import.meta.url)), 'reference-links.mjs');

test('collectUrls takes every link in every entry, deduplicated, with its anchors', () => {
  const text = [
    '- <span id="ref-6" data-tier="A">**[6]**</span> Jones. [Scan](https://archive.org/a) · [Review](https://example.org/r) *(Borrowable)*',
    '- <span id="fr-x-2000" data-tier="A">X. (2000).</span> [DOI](https://doi.org/10.1/x) *(Purchasable)*',
    '- <span id="fr-y-2001" data-tier="A">Y. (2001).</span> [DOI](https://doi.org/10.1/x) *(Purchasable)*',
    'Prose with a [link](https://not-an-entry.example) is not an entry.',
  ].join('\n');
  const urls = collectUrls([{ name: 'fixture.mdx', text }]);
  assert.deepEqual(
    [...urls.entries()].map(([u, a]) => [u, [...a]]),
    [
      ['https://archive.org/a', ['ref-6']],
      ['https://example.org/r', ['ref-6']],
      ['https://doi.org/10.1/x', ['fr-x-2000', 'fr-y-2001']],
    ],
  );
});

test('the real bibliographies yield URLs, every one http(s)', async () => {
  const urls = collectUrls(await readBibliographyTexts());
  assert.ok(urls.size > 0, 'no URLs collected from the bibliographies');
  for (const u of urls.keys()) assert.match(u, /^https?:\/\//);
});

function cli(args, env = {}) {
  return spawnSync(process.execPath, [SCRIPT, ...args], { encoding: 'utf8', env: { ...process.env, ...env } });
}

test('--dry-run counts URLs without fetching and exits 0', () => {
  const r = cli(['--dry-run']);
  assert.equal(r.status, 0, r.stderr);
  assert.match(r.stdout, /\d+ URLs? to check/);
});

test('a bibliography that cannot be read exits 2, never 0', () => {
  const dir = mkdtempSync(join(tmpdir(), 'poly-links-'));
  try {
    const r = cli(['--dry-run'], { POLY_BIBLIOGRAPHY_ROOT: dir });
    assert.equal(r.status, 2, `expected 2, got ${r.status}: ${r.stderr}`);
    assert.match(r.stderr, /bibliography/i);
  } finally {
    rmSync(dir, { recursive: true, force: true });
  }
});

// ---- task 2's real run: undici's own timeouts are no-responses -------------
// The first real run classified doi.org → doiserbia.nb.rs, whose host never
// accepts a connection, as "check by hand": undici reports a connect timeout
// as a TypeError whose cause carries UND_ERR_CONNECT_TIMEOUT, not as an abort.
// A host that never answers is the "no-response" the slice calls dead.
import { classifyError, describeError } from './reference-links.mjs';

const undici = (code) => Object.assign(new TypeError('fetch failed'), { cause: { code } });

test('undici connect, headers and body timeouts are no-responses, so dead', () => {
  for (const code of ['UND_ERR_CONNECT_TIMEOUT', 'UND_ERR_HEADERS_TIMEOUT', 'UND_ERR_BODY_TIMEOUT']) {
    assert.equal(classifyError(undici(code)), 'dead', code);
  }
});

test('a redirect loop stays "check by hand", and the report says why', () => {
  const loop = Object.assign(new TypeError('fetch failed'), { cause: new Error('redirect count exceeded') });
  assert.equal(classifyError(loop), 'error');
  assert.equal(describeError(loop), 'redirect count exceeded');
});
