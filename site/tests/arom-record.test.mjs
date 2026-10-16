// central-africa M001/S01: Arom 1991 is a source the guide has read.
//
// CA01. The owner read Book VI, Chapter 3 ("Strict polyrhythmics",
// pp. 229–306) from a scan and checked the guide's claims against it, but the
// record still said the text was not read. A verdict limited to the pages read
// is the honest one: the rest of the book stays unexamined, and the evidence
// has to say which pages the verdict covers.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const HERE = dirname(fileURLToPath(import.meta.url));
const MANIFEST_PATH = join(HERE, '..', 'src', 'data', 'references.json');
const ANCHOR = 'fr-arom-1991';

async function aromRecord() {
  const m = JSON.parse(await readFile(MANIFEST_PATH, 'utf8'));
  const rec = m.entries?.[ANCHOR];
  if (!rec) throw new Error(`${ANCHOR} is missing from references.json — this check would assert nothing`);
  return rec;
}

test('fr-arom-1991 is verified for the pages that were read', async () => {
  const rec = await aromRecord();
  assert.equal(rec.description, 'verified');
  assert.match(rec.evidence, /pp\. 229–306/, 'the evidence must name the pages the verdict covers');
  assert.doesNotMatch(rec.evidence, /was not read/, 'the evidence still says the text was not read');
});

test('the scan of a copyrighted book is not archived', async () => {
  const rec = await aromRecord();
  assert.equal(rec.archiveFile, null);
  assert.equal(rec.retrieval.status, 'not-attempted');
});

// CA03. The theory bundle credited Arom with the timeline as the ensemble's
// "shared clock" and with the dance beat as a referent. The chapter read
// (pp. 229–306) names the pulsation — isochronous, usually implicit — as the
// reference unit (p. 230). Each Arom citation in the bundle must now be for
// that, and the timeline sentence must not carry his name.
const THEORY = join(HERE, '..', 'src', 'content', 'theory');

function sentencesCiting(mdx, anchor) {
  const body = mdx.replace(/^---\n[\s\S]*?\n---\n/, '');
  return body
    .split(/(?<=[.!?])\s+(?=[A-Z*_(\[])/)
    .filter((s) => s.includes(`#${anchor}`));
}

test('the timeline sentence no longer cites Arom', async () => {
  const mdx = await readFile(join(THEORY, 'theory-sub-saharan-africa.mdx'), 'utf8');
  const clock = mdx.split('\n').find((l) => l.includes("shared clock"));
  if (!clock) throw new Error('the "shared clock" sentence is gone — this check would assert nothing');
  const sentence = clock.split(/(?<=\.)\s+/).find((s) => s.includes('shared clock'));
  assert.doesNotMatch(sentence, /#fr-arom-1991/);
});

test('every Arom citation in the theory bundle is for the pulsation, p. 230', async () => {
  const bad = [];
  let seen = 0;
  for (const f of ['theory-sub-saharan-africa.mdx', 'theory-counterpoint-overview.mdx']) {
    for (const s of sentencesCiting(await readFile(join(THEORY, f), 'utf8'), 'fr-arom-1991')) {
      seen += 1;
      if (!/pulsation/.test(s) || !/p\. 230/.test(s)) bad.push(`${f}: ${s.slice(0, 120)}`);
    }
  }
  if (seen === 0) throw new Error('no Arom citation found in the theory bundle — this check would assert nothing');
  assert.deepEqual(bad, [], `Arom cited for something other than the pulsation (p. 230):\n${bad.join('\n')}`);
});
