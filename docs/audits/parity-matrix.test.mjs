// Completeness test for the M053 WebUI <-> native parity matrix.
//
// This is the mechanical verification surface for M053/S01: it asserts the
// audit doc (docs/audits/M053-webui-parity-matrix.md) is COMPLETE, so a future
// agent cannot silently drop a native view or leave a row un-verdicted.
//
// It intentionally does NOT judge the *correctness* of any verdict (that is a
// human/S02 job) — it only enforces structural completeness:
//   1. Every one of the 15 native views in the inventory has a parity-matrix
//      row that carries a real verdict token.
//   2. Every verdict cell in the matrix holds exactly one token from the fixed
//      schema {parity, gap, webui-only, divergent}.
//   3. No `_pending_` placeholder survives in a verdict cell.
//   4. The coverage checklist has every native view checked.
//
// Run: node --test docs/audits/parity-matrix.test.mjs

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const HERE = dirname(fileURLToPath(import.meta.url));
const DOC = join(HERE, 'M053-webui-parity-matrix.md');
const md = readFileSync(DOC, 'utf8');
const lines = md.split('\n');

const VERDICTS = new Set(['parity', 'gap', 'webui-only', 'divergent']);

// The 15 native views that MUST each appear as a verdicted matrix row. This
// list is the contract; it is duplicated here (not scraped from the doc) so a
// row accidentally deleted from BOTH the inventory and the matrix still fails.
const NATIVE_VIEWS = [
  'cell_editor_view',
  'lane_grid_view',
  'timeline_step_editor_view',
  'micro_timing_editor_view',
  'lane_edit_view',
  'cross_rhythm_view',
  'phase_alignment_view',
  'phrase_edit_view',
  'envelope_curve_view',
  'velocity_view',
  'note_map_view',
  'header_view',
  'scene_bar_view',
  'chain_popover_view',
  'export_controls_view',
];

// Split a markdown table row into trimmed cell strings (drops the leading and
// trailing empty cells produced by the border pipes).
function cells(row) {
  const parts = row.split('|');
  return parts.slice(1, parts.length - 1).map((c) => c.trim());
}

// Extract the single verdict token from a verdict cell, or null. The cell may
// wrap the token in backticks (e.g. "`divergent`"); a valid cell holds exactly
// one schema token and nothing else meaningful.
function verdictOf(cell) {
  const tokens = (cell.match(/[a-z-]+/g) || []).filter((t) => VERDICTS.has(t));
  const stripped = cell.replace(/[`\s]/g, '');
  if (tokens.length === 1 && VERDICTS.has(stripped)) return tokens[0];
  return null;
}

// Collect every parity-matrix data row (Lane A-D tables + the webui-only
// table). We identify verdict tables by their 5-column shape with a
// "Parity Verdict" / "Verdict" header, then read the rows beneath.
function matrixRows() {
  const rows = [];
  let inVerdictTable = false;
  let verdictCol = -1;
  let viewCol = -1;
  for (const raw of lines) {
    const line = raw.trimEnd();
    const isTableRow = line.startsWith('|') && line.endsWith('|');
    if (!isTableRow) {
      inVerdictTable = false;
      continue;
    }
    const c = cells(line);
    const isSeparator = c.every((cell) => /^:?-+:?$/.test(cell));
    if (isSeparator) continue;
    const header = c.map((h) => h.toLowerCase());
    const vCol = header.findIndex((h) => h.includes('verdict'));
    if (vCol !== -1) {
      // This is a header row for a verdict table.
      inVerdictTable = true;
      verdictCol = vCol;
      viewCol = header.findIndex((h) => h.includes('view') || h.includes('capability'));
      continue;
    }
    if (inVerdictTable) {
      rows.push({ view: c[viewCol] ?? '', verdict: c[verdictCol] ?? '', raw: line });
    }
  }
  return rows;
}

test('doc exists and is non-empty', () => {
  assert.ok(md.length > 1000, 'parity matrix doc should be substantial');
});

test('every matrix verdict cell holds exactly one valid schema token', () => {
  const rows = matrixRows();
  assert.ok(rows.length >= NATIVE_VIEWS.length, `expected >= ${NATIVE_VIEWS.length} verdict rows, got ${rows.length}`);
  for (const { verdict, raw } of rows) {
    const v = verdictOf(verdict);
    assert.ok(v !== null, `row has no single valid verdict token: ${raw}`);
  }
});

test('no _pending_ placeholder survives in any verdict cell', () => {
  for (const { verdict, raw } of matrixRows()) {
    assert.ok(!verdict.includes('_pending_'), `verdict cell still pending: ${raw}`);
  }
});

test('all 15 native views appear as a verdicted matrix row', () => {
  const rows = matrixRows();
  const verdicted = new Set();
  for (const { view, verdict } of rows) {
    if (verdictOf(verdict) === null) continue;
    for (const name of NATIVE_VIEWS) {
      if (view.includes(name)) verdicted.add(name);
    }
  }
  const missing = NATIVE_VIEWS.filter((v) => !verdicted.has(v));
  assert.deepEqual(missing, [], `native views missing a verdicted matrix row: ${missing.join(', ')}`);
});

test('coverage checklist has every native view checked', () => {
  for (const name of NATIVE_VIEWS) {
    const re = new RegExp(`- \\[x\\] \`${name}\``);
    assert.ok(re.test(md), `coverage checklist missing or unchecked: ${name}`);
  }
});
