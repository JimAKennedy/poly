// positioning.test.mjs — one sentence says what Poly is, and every surface a
// stranger reads says the same one.
//
// open-source-launch M004/S02, OS21. The repository said "polymetric drum
// pattern generator" and the launch intent said "open-source Euclidean
// sequencer", two descriptions of two different things. The owner chose one
// sentence, recorded with its alternatives in
// docs/plans/open-source-launch/M004-decisions.md; this test holds it
// verbatim in the README's opening, the site's meta description and
// CLAUDE.md. The About panel carries it too, read back into the evidence
// rather than asserted here.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const REPO = resolve(dirname(fileURLToPath(import.meta.url)), '..', '..');
const read = (p) => readFileSync(resolve(REPO, p), 'utf8');

export const SENTENCE =
  'Poly is a free, open-source polymetric drum sequencer for your DAW: grooves grounded in real drumming traditions, a guide that cites where every preset comes from, deterministic output, and an engine that runs in your browser.';

// The first line that is neither blank nor a heading: the opening sentence.
function opening(text) {
  return text.split('\n').find((l) => l.trim() !== '' && !l.startsWith('#')) ?? '';
}

test("README.md opens with the sentence", () => {
  assert.equal(opening(read('README.md')).trim(), SENTENCE);
});

test("the site's meta description is the sentence", () => {
  const cfg = read('site/astro.config.mjs');
  const m = cfg.match(/description:\s*\n?\s*'([^']*)'/);
  assert.ok(m, 'astro.config.mjs has no description');
  assert.equal(m[1], SENTENCE);
});

test('CLAUDE.md opens with the sentence', () => {
  assert.equal(opening(read('CLAUDE.md')).trim(), SENTENCE);
});
