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

// OS22: the README is written for the person downloading, and neither it nor
// CONTRIBUTING.md asks a visitor to resolve a decision or milestone identifier.
const README_ORDER = ['Try it in the browser', 'Download', 'DAW compatibility', 'Guide', 'Contributing', 'Building'];

test('README.md and CONTRIBUTING.md carry no internal decision or milestone ID', () => {
  for (const file of ['README.md', 'CONTRIBUTING.md']) {
    const hits = [...read(file).matchAll(/\b[DM]0\d\d\b/g)].map((m) => m[0]);
    assert.deepEqual(hits, [], `${file} carries internal identifiers (${hits.join(', ')}) a visitor cannot resolve (OS22)`);
  }
});

test("README.md's sections run positioning, screenshot, try it, download, DAW setup, then contributing and building", () => {
  const readme = read('README.md');
  const headings = [...readme.matchAll(/^## (.+)$/gm)].map((m) => m[1].trim());
  assert.deepEqual(headings.slice(0, README_ORDER.length), README_ORDER, `README.md's first sections are ${headings.slice(0, 6).join(' / ')} (OS22)`);
  const firstHeading = readme.indexOf('\n## ');
  const screenshot = readme.indexOf('](site/public/screenshots/ui-overview.png)');
  assert.ok(screenshot !== -1 && screenshot < firstHeading, 'the screenshot must appear before the first section heading (OS22)');
});
