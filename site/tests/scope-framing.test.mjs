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
import { readFile } from 'node:fs/promises';
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
