// Completeness test for the M053 WebUI <-> native gap-closure plan.
//
// This is the mechanical verification surface for M053/S02: it asserts the
// decision doc (docs/audits/M053-gap-closure-plan.md) is COMPLETE, so a future
// agent cannot silently drop a sized gap row, blank a size token, or gut the
// go decision that drives the S03-S06 roadmap reassessment.
//
// Like the S01 parity-matrix gate, it intentionally does NOT judge the
// *correctness* of any size or recommendation (that is a human/owner job) — it
// only enforces structural completeness:
//   1. The doc exists and is substantial.
//   2. Every one of the 16 gap-ledger rows (G01..G16) is present.
//   3. Every gap-ledger Size cell holds exactly one token from {S, M, L, XL}.
//   4. Every gap-ledger Rec cell holds exactly one valid recommendation token.
//   5. No `_pending_` placeholder survives anywhere in the doc.
//   6. A Go Decision section is present and carries a valid decision token.
//   7. The roadmap-reassessment section exists.
//
// Run: node --test docs/audits/gap-closure-plan.test.mjs

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const HERE = dirname(fileURLToPath(import.meta.url));
const DOC = join(HERE, 'M053-gap-closure-plan.md');
const md = readFileSync(DOC, 'utf8');
const lines = md.split('\n');

const SIZES = new Set(['S', 'M', 'L', 'XL']);
const RECS = new Set([
  'close-in-webui',
  'close-native-drag-helper',
  'document-as-acceptable-divergence',
]);
const DECISIONS = new Set(['close-all', 'keep-both-document-divergence']);

// The 16 gap IDs that MUST each appear as a sized ledger row. This list is the
// contract; it is duplicated here (not scraped from the doc) so a row
// accidentally deleted still fails the gate.
const GAP_IDS = Array.from({ length: 16 }, (_, i) => `G${String(i + 1).padStart(2, '0')}`);

// Split a markdown table row into trimmed cell strings (drops the leading and
// trailing empty cells produced by the border pipes).
function cells(row) {
  const parts = row.split('|');
  return parts.slice(1, parts.length - 1).map((c) => c.trim());
}

// Extract the single token from a backtick-wrapped cell if it belongs to the
// given schema set, else null. A valid cell holds exactly one schema token and
// nothing else meaningful.
function tokenOf(cell, schema) {
  const stripped = cell.replace(/[`\s]/g, '');
  return schema.has(stripped) ? stripped : null;
}

// Collect every gap-ledger data row. We identify the ledger table by its header
// carrying both a "Gap ID" and a "Size" column, then read the rows beneath it
// until the table ends.
function ledgerRows() {
  const rows = [];
  let inLedger = false;
  let idCol = -1;
  let sizeCol = -1;
  let recCol = -1;
  for (const raw of lines) {
    const line = raw.trimEnd();
    const isTableRow = line.startsWith('|') && line.endsWith('|');
    if (!isTableRow) {
      inLedger = false;
      continue;
    }
    const c = cells(line);
    const isSeparator = c.every((cell) => /^:?-+:?$/.test(cell));
    if (isSeparator) continue;
    const header = c.map((h) => h.toLowerCase());
    const gCol = header.findIndex((h) => h.includes('gap id'));
    const sCol = header.findIndex((h) => h === 'size');
    if (gCol !== -1 && sCol !== -1) {
      // This is the ledger header row.
      inLedger = true;
      idCol = gCol;
      sizeCol = sCol;
      recCol = header.findIndex((h) => h === 'rec');
      continue;
    }
    if (inLedger) {
      rows.push({
        id: c[idCol] ?? '',
        size: c[sizeCol] ?? '',
        rec: recCol === -1 ? '' : (c[recCol] ?? ''),
        raw: line,
      });
    }
  }
  return rows;
}

test('doc exists and is non-empty', () => {
  assert.ok(md.length > 1000, 'gap-closure plan doc should be substantial');
});

test('all 16 gap rows (G01..G16) appear in the ledger', () => {
  const rows = ledgerRows();
  const seen = new Set();
  for (const { id } of rows) {
    for (const gid of GAP_IDS) {
      if (id.includes(gid)) seen.add(gid);
    }
  }
  const missing = GAP_IDS.filter((g) => !seen.has(g));
  assert.deepEqual(missing, [], `gap rows missing from the ledger: ${missing.join(', ')}`);
});

test('every gap-ledger row carries exactly one valid size token', () => {
  const rows = ledgerRows();
  assert.ok(rows.length >= GAP_IDS.length, `expected >= ${GAP_IDS.length} ledger rows, got ${rows.length}`);
  for (const { size, raw } of rows) {
    assert.ok(tokenOf(size, SIZES) !== null, `row has no single valid size token: ${raw}`);
  }
});

test('every gap-ledger row carries exactly one valid recommendation token', () => {
  for (const { rec, raw } of ledgerRows()) {
    assert.ok(tokenOf(rec, RECS) !== null, `row has no single valid rec token: ${raw}`);
  }
});

test('no _pending_ placeholder survives anywhere in the doc', () => {
  assert.ok(!md.includes('_pending_'), 'a _pending_ placeholder is still present in the doc');
});

test('a Go Decision section is present with a valid decision token', () => {
  assert.ok(/^##\s+Go Decision\s*$/m.test(md), 'missing "## Go Decision" section header');
  const hasDecision = [...DECISIONS].some((d) => md.includes(`\`${d}\``));
  assert.ok(hasDecision, `Go Decision section carries no valid decision token (${[...DECISIONS].join(' | ')})`);
});

test('the roadmap-reassessment section exists', () => {
  assert.ok(
    /^##\s+Roadmap reassessment recommendation\s*$/m.test(md),
    'missing "## Roadmap reassessment recommendation" section header',
  );
});
