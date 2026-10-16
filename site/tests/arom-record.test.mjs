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
