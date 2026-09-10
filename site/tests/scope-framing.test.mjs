// Scope-and-framing cases for the M003 milestone of the theory-audit ledger
// (docs/plans/theory-audit/ledger.md).
//
// M003/S01 creates this file with the F36 case; later M003 slices extend it —
// S02 with Chapter 6's Hindustani scope, S03 with the simplification
// disclosures, S04 with the non-isochrony statements, S05 with the remaining
// framing items.
//
// It is wired into scripts/check-doc-conformance.sh and named in the REQUIRED
// array of doc-conformance-wiring.test.mjs. Both are necessary: nothing in CI
// runs `npm --prefix site test` (poly issue #272), so a host outside the runner
// is a lock that never runs on the remote, and a host the runner names but
// REQUIRED does not is one a later edit can quietly drop.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile, readdir } from 'node:fs/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

import { registerClaimTests } from './helpers/prose-claims.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const DOCS = join(HERE, '..', 'src', 'content', 'docs');

const loadSource = (file) => readFile(join(DOCS, file), 'utf8');

const CLAIMS = [
  {
    id: 'S01-F36',
    file: 'about-this-guide.mdx',
    rule:
      'ledger F36. Whether rumba clave predates or postdates son clave is debated ' +
      '(Acosta 2004, Moore 2006). The audit judges the guide may legitimately ' +
      'sidestep the question under the repositioning frame — but a sidestep that ' +
      'is never stated is indistinguishable from not knowing, so the About page ' +
      'declares it as a deliberate exclusion',
    present: ['rumba clave predates or postdates', 'Acosta', 'Moore'],
  },
];

registerClaimTests({ test, assert, claims: CLAIMS, loadSource });

// M003/S01 task 2. F24 asks that the About page be reachable, not merely
// present — from the introduction and from every theory page. That is thirteen
// near-identical edits, which is exactly where one gets missed, so this case
// reports every file that lacks the link rather than stopping at the first.
//
// The theory set is discovered by glob rather than hard-coded, so a thirteenth
// deep dive added later is covered by construction. The count is asserted too:
// a glob that silently matches nothing would otherwise let the whole case pass
// vacuously.
const ABOUT_LINK = '/about-this-guide/';

test('S01-F24: the About page exists and is reachable from the introduction and every theory page', async () => {
  const entries = await readdir(DOCS);
  assert.ok(
    entries.includes('about-this-guide.mdx'),
    'about-this-guide.mdx is missing — F24 requires the page itself, not only links to it',
  );

  const theory = entries.filter((f) => f.startsWith('theory-') && f.endsWith('.mdx')).sort();
  assert.equal(
    theory.length,
    12,
    `expected 12 theory-*.mdx pages, found ${theory.length} — if a deep dive was ` +
      'added or removed, update this count deliberately rather than loosening the check',
  );

  const missing = [];
  for (const file of ['introduction.mdx', ...theory]) {
    const src = await readFile(join(DOCS, file), 'utf8');
    if (!src.includes(ABOUT_LINK)) missing.push(file);
  }
  assert.deepEqual(
    missing,
    [],
    `${missing.length} of ${theory.length + 1} page(s) do not link to ${ABOUT_LINK}:\n  ` +
      missing.join('\n  '),
  );
});
