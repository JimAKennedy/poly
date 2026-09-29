#!/usr/bin/env node
// check-docs-index.mjs — docs/README.md names everything under docs/, and
// nothing that is gone.
//
// open-source-launch M004/S03, OS25. docs/ holds some 250 files, most of them
// delivery records, and nothing told a contributor which few to read. The
// index classifies them; this guard keeps it complete the way
// check-scripts-readme.sh keeps scripts/README.md complete, because an index
// that silently omits a new document is worse than none. Two directions over
// the top level of docs/: every `*.md` (other than the index) and every
// immediate subdirectory appears as a backticked token — `name.md` or
// `name/` — and every such token resolves to a real file or directory.
//
// Run: `node --test scripts/check-docs-index.mjs` (wired into check-guards.sh).
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { existsSync, readdirSync, readFileSync, statSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const REPO = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const DOCS = resolve(REPO, 'docs');
const INDEX = resolve(DOCS, 'README.md');

const entries = readdirSync(DOCS, { withFileTypes: true });
const expected = entries
  .filter((e) => (e.isFile() && e.name.endsWith('.md') && e.name !== 'README.md') || e.isDirectory())
  .map((e) => (e.isDirectory() ? `${e.name}/` : e.name))
  .sort();

test('docs/README.md exists', () => {
  assert.ok(existsSync(INDEX), 'docs/README.md is missing — the index a contributor reads first');
});

const index = existsSync(INDEX) ? readFileSync(INDEX, 'utf8') : '';
// `docs/` names the directory itself, which the index naturally mentions;
// every other token names something inside it.
const tokens = [...index.matchAll(/`([A-Za-z0-9._-]+(?:\.md|\/))`/g)].map((m) => m[1]).filter((t) => t !== 'docs/');

test('every top-level document and directory under docs/ is listed in the index', () => {
  const missing = expected.filter((name) => !tokens.includes(name));
  assert.deepEqual(missing, [], `docs/README.md does not list: ${missing.join(', ')}`);
});

test('every document or directory the index names exists', () => {
  const stale = [...new Set(tokens)].filter((t) => {
    const p = resolve(DOCS, t.endsWith('/') ? t.slice(0, -1) : t);
    return !existsSync(p) || (t.endsWith('/') ? !statSync(p).isDirectory() : !statSync(p).isFile());
  });
  assert.deepEqual(stale, [], `docs/README.md names things that are gone: ${stale.join(', ')}`);
});
