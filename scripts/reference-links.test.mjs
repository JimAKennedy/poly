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
