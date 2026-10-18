// Chapter 2 reconciled with the Central Africa chapter (central-africa M004/S01,
// rows CA29–CA34 of docs/plans/central-africa/ledger.md).
//
// Chapter 2 spoke for "West and Central Africa", built its stacking section on
// an uncited claim about Ewe cycle lengths, asserted a downbeat, and printed a
// patch table that matched no preset. Each correction is locked here: the
// prose cases through the shared prose-claims helper, the structural ones as
// plain tests that derive their expectation rather than transcribe it.
//
// Wired into scripts/check-doc-conformance.sh and named in REQUIRED in
// doc-conformance-wiring.test.mjs.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { existsSync } from 'node:fs';
import { readFile } from 'node:fs/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

import { registerClaimTests } from './helpers/prose-claims.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const DOCS = join(HERE, '..', 'src', 'content', 'docs');
const PAGE = '02-sub-saharan-africa.mdx';

const loadSource = (file) => readFile(join(DOCS, file), 'utf8');

const CLAIMS = [
  {
    id: 'CA29',
    file: PAGE,
    rule:
      'ledger CA29. The description and opening claimed "West and Central African ' +
      'drumming" and offered the timeline/support/lead model for both. The Aka ' +
      'formulae have no timeline in the Ewe sense and their drums barely vary ' +
      '(Arom norm N10), so the chapter is scoped to West Africa',
    forbidden: ['West and Central Africa', 'West and Central African'],
    present: ['West African drumming', 'West Africa'],
  },
  {
    id: 'CA34',
    file: PAGE,
    rule: 'ledger CA34. Chapter 2 sends readers to Chapter 2b for Central African music',
    present: ['/02b-central-africa/'],
  },
];

registerClaimTests({ test, assert, claims: CLAIMS, loadSource });

test('CA34: the Central Africa link resolves to a page', async () => {
  const src = await loadSource(PAGE);
  const slugs = [...src.matchAll(/\]\(\/([a-z0-9-]+)\//g)].map((m) => m[1]);
  const bridge = slugs.filter((s) => s === '02b-central-africa');
  assert.ok(bridge.length > 0, `${PAGE} carries no link to /02b-central-africa/`);
  for (const slug of bridge) {
    // Starlight routes a page by its file name, so the file is the route.
    assert.ok(existsSync(join(DOCS, `${slug}.mdx`)), `${PAGE} links /${slug}/, but no ${slug}.mdx exists`);
  }
});
