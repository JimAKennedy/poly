// first-release M002/S02, FR14. The theory deep dives were unpublished and moved
// to site/src/content/theory/. They are deferred, not retired, so the day they
// are published again every citation in them has to resolve — and nothing will
// warn in the meantime, because research_provenance's roots stop at
// site/src/content/docs and no longer scan them.
//
// That silence is the whole reason this guard exists. The shipping appendix is
// being reduced to the references its own chapters cite (M003), so an anchor the
// deep dives cite and only the shipping appendix defines would disappear without
// a single check going red. The bundle therefore carries its own bibliography,
// and this asserts the two stay in step.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile, readdir } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const HERE = dirname(fileURLToPath(import.meta.url));
const THEORY = join(HERE, '..', 'src', 'content', 'theory');
const BIB = join(THEORY, 'theory-references.mdx');

const ANCHOR = /#((?:ref|fr)-[A-Za-z0-9-]+)/g;
const DEFINED = /id="((?:ref|fr)-[A-Za-z0-9-]+)"/g;

async function deepDivePages() {
  const files = (await readdir(THEORY)).filter(
    (f) => f.endsWith('.mdx') && f !== 'theory-references.mdx',
  );
  if (files.length === 0) {
    throw new Error(
      `no deep dives found in ${THEORY} — have they moved again? An empty set ` +
        'would let every assertion below pass while checking nothing.',
    );
  }
  return files;
}

test('every anchor the deep dives cite is defined in their own bibliography', async () => {
  const bib = await readFile(BIB, 'utf8');
  const defined = new Set([...bib.matchAll(DEFINED)].map((m) => m[1]));

  const missing = new Map();
  for (const f of await deepDivePages()) {
    const src = await readFile(join(THEORY, f), 'utf8');
    for (const m of src.matchAll(ANCHOR)) {
      if (!defined.has(m[1])) {
        if (!missing.has(m[1])) missing.set(m[1], new Set());
        missing.get(m[1]).add(f);
      }
    }
  }

  assert.deepEqual(
    [...missing].map(([a, fs]) => `${a} (cited by ${[...fs].sort().join(', ')})`),
    [],
    'cited by a deep dive but not defined in theory-references.mdx. Nothing else ' +
      'will catch this: the provenance check no longer scans these pages, and the ' +
      'shipping appendix is being reduced underneath them',
  );
});

test('the deep dives\' bibliography defines nothing they do not cite', async () => {
  const bib = await readFile(BIB, 'utf8');
  const defined = [...bib.matchAll(DEFINED)].map((m) => m[1]);

  const cited = new Set();
  for (const f of await deepDivePages()) {
    const src = await readFile(join(THEORY, f), 'utf8');
    for (const m of src.matchAll(ANCHOR)) cited.add(m[1]);
  }

  const orphans = defined.filter((a) => !cited.has(a));
  assert.deepEqual(
    orphans,
    [],
    'defined in theory-references.mdx but cited by no deep dive — the bundle is ' +
      'meant to carry what it needs, not a copy of the shipping appendix: ' +
      orphans.join(', '),
  );
});
