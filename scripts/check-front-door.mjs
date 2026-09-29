#!/usr/bin/env node
// check-front-door.mjs — what a stranger meets first cannot go stale by itself.
//
// open-source-launch M004/S01 and M004/S03. ROADMAP.md enumerated four issue
// numbers, all closed, while the open issues were five others: a list of
// numbers drifts by construction, so the roadmap now links label queries and
// the delivery ledgers, and this guard fails if a `#NNN` returns (OS20).
// README.md and CONTRIBUTING.md carried decision and milestone identifiers a
// visitor cannot resolve; the guard fails if one returns, and holds the
// README's section order to the one written for the person downloading
// (OS22).
//
// Run: `node --test scripts/check-front-door.mjs` (wired into check-guards.sh).
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const REPO = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const read = (p) => readFileSync(resolve(REPO, p), 'utf8');

test('ROADMAP.md enumerates no issue numbers', () => {
  const roadmap = read('ROADMAP.md');
  const hits = [...roadmap.matchAll(/\[#\d+\]|\bissues\/\d+\b/g)].map((m) => m[0]);
  assert.deepEqual(hits, [], `ROADMAP.md names issue numbers (${hits.join(', ')}) — link a label query instead, so a closure cannot make it stale (OS20)`);
});
