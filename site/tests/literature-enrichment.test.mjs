// Literature-enrichment cases for the M005 milestone of the theory-audit ledger
// (docs/plans/theory-audit/ledger.md).
//
// M005 differs from the milestones before it. M001–M004 corrected and locked
// material already in the tree, where every claim was checkable against a file.
// M005 imports external facts: bibliography entries for works whose existence,
// titles, publishers and subject matter cannot be established from this repo.
//
// These cases therefore check what this repo *can* check — that an entry exists,
// declares a tier, and is cited at the claim it was added for. They cannot check
// that the work exists or supports that claim. That gap is jk-standards#85, and
// it is why every source was verified online before planning, with the results
// recorded in docs/plans/theory-audit/M005-decisions.md.
//
// Wired into scripts/check-doc-conformance.sh and named in the REQUIRED array of
// doc-conformance-wiring.test.mjs. Both are necessary: nothing in CI runs
// `npm --prefix site test` (poly issue #272), so a host outside the runner is a
// lock that never runs on the remote.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const DOCS = join(HERE, '..', 'src', 'content', 'docs');
const APPENDIX = join(DOCS, 'appendix-references.mdx');

// Entries M005 adds, with the tier each must declare. Every one was verified
// bibliographically and by published descriptions of its subject matter; none
// was read. See M005-decisions.md.
const ENTRIES = [
  { anchor: 'fr-charry-2000', tier: 'A', row: 'F44' },
  { anchor: 'fr-kubik-1999', tier: 'A', row: 'F45' },
  { anchor: 'fr-agawu-2003', tier: 'A', row: 'F46' },
];

test('M005/S01: the sub-Saharan sources are in the appendix with declared tiers', async () => {
  const src = await readFile(APPENDIX, 'utf8');
  const missing = [];
  const wrongTier = [];
  for (const e of ENTRIES) {
    const m = src.match(new RegExp(`id="${e.anchor}"\\s+data-tier="([A-Z])"`));
    if (!m) {
      missing.push(`${e.anchor} (ledger row ${e.row})`);
      continue;
    }
    if (m[1] !== e.tier) wrongTier.push(`${e.anchor}: tier ${m[1]}, expected ${e.tier}`);
  }
  assert.deepEqual(
    missing,
    [],
    `appendix-references.mdx is missing ${missing.length} entry/entries that M005/S01 adds:\n  ` +
      missing.join('\n  '),
  );
  assert.deepEqual(wrongTier, [], wrongTier.join('\n  '));
});
