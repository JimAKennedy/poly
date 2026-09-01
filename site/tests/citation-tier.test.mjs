// Citation-integrity cases for the M002 milestone of the theory-audit ledger
// (docs/plans/theory-audit/ledger.md).
//
// M002/S01 creates this file with the F17 case. M002/S06 (F23) extends it with
// the reference-tier assertions — every bibliography entry carrying a declared
// tier, and every inline citation on a named-theory claim resolving to a
// Tier-A source — and wires it into scripts/check-doc-conformance.sh. Until
// then it runs under the `site-unit` token alone: site/package.json runs
// `node --test tests/**/*.test.mjs`, which picks this file up with no wiring.
//
// F17 is not a tier problem. Ref [2] named a real journal, a real volume, a
// real author and a working URL, and attached a title that does not exist —
// the article at that URL is Goldberg's Hristov/Bulgarian-meter paper. Every
// guard the guide had was blind to it: a link checker sees the URL, a tier
// check sees the venue, and the provenance check sees the anchor. None sees
// the pairing. The tree-wide case below is the one that would catch a repeat,
// because it forbids the fabricated string everywhere rather than in the one
// entry we happen to be looking at.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile, readdir } from 'node:fs/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

import { registerClaimTests } from './helpers/prose-claims.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const DOCS = join(HERE, '..', 'src', 'content', 'docs');

const loadSource = (file) => readFile(join(DOCS, file), 'utf8');

// The title the guide printed, which exists in no MTO issue and nowhere a web
// search reaches. Shared by both cases so they cannot drift apart.
const FABRICATED_TITLE = 'Resultant Patterns in Phase-Shifted Rhythmic Structures';

const CLAIMS = [
  {
    id: 'S01-F17',
    file: 'appendix-references.mdx',
    rule:
      'audit §5 on ref [2]. Verified against the publisher 2026-09-01: MTO is ' +
      'current through Vol 32 No 2 (June 2026), so 31(2) is not a forward ' +
      'reference and its URL resolves. The issue table of contents lists ' +
      "Goldberg's article as the Dobri Hristov / Bulgarian-meter paper. Real " +
      'article, fabricated title — corrected in place',
    forbidden: [FABRICATED_TITLE],
    present: ['Music Theory as an Instrument of Nationalism', 'Dobri Hristov'],
    presentRegex: [/mto\.25\.31\.2\.goldberg\.pdf/],
  },
];

registerClaimTests({ test, assert, claims: CLAIMS, loadSource });

test(`S01-F17-tree: the fabricated ref-2 title appears in no doc`, async () => {
  const entries = await readdir(DOCS, { withFileTypes: true });
  const offenders = [];
  for (const entry of entries) {
    if (!entry.isFile() || !entry.name.endsWith('.mdx')) continue;
    const text = await readFile(join(DOCS, entry.name), 'utf8');
    if (text.includes(FABRICATED_TITLE)) offenders.push(entry.name);
  }
  assert.deepEqual(
    offenders,
    [],
    `fabricated citation title reappeared in: ${offenders.join(', ')}. ` +
      'Authority: MTO 31(2) carries no article of this title (publisher table ' +
      'of contents, checked 2026-09-01).',
  );
});
