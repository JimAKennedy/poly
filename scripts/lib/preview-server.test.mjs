#!/usr/bin/env node
// preview-server.test.mjs — the preview server the e2e gate starts is its own,
// and nothing is left listening once it is stopped.
//
// guide-parity M006/S01, GP15. Astro 7.3.8 detaches `astro preview` into a
// locked background server whenever it detects an AI agent, so the subshell
// site-verify-local.sh tracked had already exited and its cleanup stopped
// nothing: a server stayed on port 4322, and the next run's readiness check
// passed against it. The helper starts the server with `--ignore-lock`, which
// keeps it in the foreground, and checks the port itself before and after.
//
// The live case sets CLAUDECODE=1 for the child, so it exercises the agent
// path whoever runs it, and is skipped with its reason when site/dist has not
// been built — as in CI's code-quality job.
//
// Run: `node --test scripts/lib/preview-server.test.mjs` (wired into
// check-guards.sh).
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { existsSync, mkdtempSync, rmSync } from 'node:fs';
import { createServer } from 'node:net';
import { tmpdir } from 'node:os';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, '..', '..');
const HELPER = join(ROOT, 'scripts', 'lib', 'preview-server.sh');
const SITE = join(ROOT, 'site');

function helper(body, env = process.env) {
  return spawnSync('bash', ['-c', `source "${HELPER}"; ${body}`], {
    cwd: ROOT,
    env,
    encoding: 'utf8',
    timeout: 60_000,
  });
}

function listen(port = 0) {
  return new Promise((ok, fail) => {
    const server = createServer();
    server.once('error', fail);
    server.listen(port, () => ok(server));
  });
}

async function freePort() {
  const server = await listen();
  const { port } = server.address();
  await new Promise((ok) => server.close(ok));
  return port;
}

test('a free port is accepted silently', async () => {
  const port = await freePort();
  const r = helper(`preview_require_free_port ${port}`);
  assert.equal(r.status, 0, r.stderr);
  assert.equal(r.stdout + r.stderr, '');
});

test('a held port is refused, naming the process that holds it', async () => {
  const server = await listen();
  try {
    const { port } = server.address();
    const r = helper(`preview_require_free_port ${port}`);
    assert.equal(r.status, 1);
    assert.match(r.stderr, /already in use/);
    assert.ok(r.stderr.includes(String(process.pid)), r.stderr);
  } finally {
    await new Promise((ok) => server.close(ok));
  }
});

test('the holder is printed as its PID and command', async () => {
  const server = await listen();
  try {
    const { port } = server.address();
    const r = helper(`preview_port_holder ${port}`);
    assert.equal(r.status, 0, r.stderr);
    assert.ok(r.stdout.startsWith(`${process.pid} `), r.stdout);
  } finally {
    await new Promise((ok) => server.close(ok));
  }
});

const DIST = join(SITE, 'dist', 'index.html');
test(
  'a preview started under an agent is gone once stopped',
  {
    skip: existsSync(DIST)
      ? false
      : 'site/dist/index.html absent — run the e2e gate or npm --prefix site run build first',
  },
  async () => {
    const port = await freePort();
    const dir = mkdtempSync(join(tmpdir(), 'poly-preview-test-'));
    const log = join(dir, 'preview.log');
    try {
      const r = helper(
        [
          `preview_start "${SITE}" ${port} "${log}"`,
          `for _ in $(seq 1 60); do curl -sf -o /dev/null http://localhost:${port}/ && break; sleep 0.5; done`,
          `curl -sf -o /dev/null http://localhost:${port}/ || exit 3`,
          'preview_stop',
        ].join('; '),
        { ...process.env, CLAUDECODE: '1' },
      );
      assert.equal(r.status, 0, `the preview never served (exit ${r.status}): ${r.stderr}`);
      await new Promise((ok) => setTimeout(ok, 1000));
      const left = helper(`preview_port_holder ${port}`).stdout.trim();
      assert.equal(left, '', `still listening on :${port} after preview_stop: ${left}`);
    } finally {
      const left = helper(`preview_port_holder ${port}`).stdout.trim();
      if (left) spawnSync('kill', [left.split(' ')[0]]);
      rmSync(dir, { recursive: true, force: true });
    }
  },
);
