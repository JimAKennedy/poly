// Verify every E(k,n) row in appendix-euclidean-reference.mdx is internally
// consistent AND matches Bjorklund's canonical distribution (possibly rotated).
//
// This is the anti-D2 gate. D2's error was arithmetically impossible grouping
// (6 numbers listed for 5 onsets). This test would have caught it — plus any
// pattern-onset-count mismatch (D3-shape error) and any pattern that claims
// E(k,n) but is not actually Euclidean at any rotation (D1-shape error).

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

import {
  parsePattern,
  onsetCount,
  gapSequence,
  findEuclideanRotation,
  parseGrouping,
} from '../src/lib/euclidean-claims.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const APPENDIX = join(HERE, '..', 'src', 'content', 'docs', 'appendix-euclidean-reference.mdx');

// Row shape: | E(k,n) | `pattern` | grouping | association | chapter |
// We match on rows starting with `| E(` and having ≥5 pipe-separated cells.
const ROW_RE = /^\|\s*E\((\d+),(\d+)\)\s*\|\s*`([^`]+)`\s*\|\s*([^|]+?)\s*\|/;

function parseAppendixRows(src) {
  const rows = [];
  const lines = src.split('\n');
  for (let i = 0; i < lines.length; i++) {
    const m = lines[i].match(ROW_RE);
    if (!m) continue;
    rows.push({
      lineno: i + 1,
      k: parseInt(m[1], 10),
      n: parseInt(m[2], 10),
      patternStr: m[3],
      groupingStr: m[4].trim(),
    });
  }
  return rows;
}

test('appendix-euclidean-reference.mdx has expected number of E(k,n) rows', async () => {
  const src = await readFile(APPENDIX, 'utf8');
  const rows = parseAppendixRows(src);
  assert.ok(rows.length >= 25, `expected >= 25 rows, got ${rows.length}`);
});

test('every appendix row: pattern length matches n', async () => {
  const src = await readFile(APPENDIX, 'utf8');
  const rows = parseAppendixRows(src);
  const failures = [];
  for (const row of rows) {
    const p = parsePattern(row.patternStr);
    if (p.length !== row.n) {
      failures.push(
        `line ${row.lineno} E(${row.k},${row.n}): pattern length ${p.length} != n=${row.n}`,
      );
    }
  }
  assert.equal(failures.length, 0, `\n${failures.join('\n')}`);
});

test('every appendix row: onset count matches k', async () => {
  const src = await readFile(APPENDIX, 'utf8');
  const rows = parseAppendixRows(src);
  const failures = [];
  for (const row of rows) {
    const p = parsePattern(row.patternStr);
    const oc = onsetCount(p);
    if (oc !== row.k) {
      failures.push(
        `line ${row.lineno} E(${row.k},${row.n}): pattern has ${oc} onsets, k=${row.k}`,
      );
    }
  }
  assert.equal(failures.length, 0, `\n${failures.join('\n')}`);
});

test('every appendix row: pattern is a rotation of canonical Bjorklund E(k,n)', async () => {
  const src = await readFile(APPENDIX, 'utf8');
  const rows = parseAppendixRows(src);
  const failures = [];
  for (const row of rows) {
    const p = parsePattern(row.patternStr);
    if (p.length !== row.n || onsetCount(p) !== row.k) continue; // already caught
    const rot = findEuclideanRotation(p, row.k, row.n);
    if (rot === null) {
      failures.push(
        `line ${row.lineno} E(${row.k},${row.n}): pattern "${row.patternStr}" is not Euclidean at any rotation (gaps: ${gapSequence(p).join('-')})`,
      );
    }
  }
  assert.equal(failures.length, 0, `\n${failures.join('\n')}`);
});

test('every appendix row: grouping numerals match pattern gap sequence and sum to n', async () => {
  const src = await readFile(APPENDIX, 'utf8');
  const rows = parseAppendixRows(src);
  const failures = [];
  for (const row of rows) {
    let grouping;
    try {
      grouping = parseGrouping(row.groupingStr);
    } catch (e) {
      failures.push(`line ${row.lineno} E(${row.k},${row.n}): ${e.message}`);
      continue;
    }
    if (grouping.length !== row.k) {
      failures.push(
        `line ${row.lineno} E(${row.k},${row.n}): grouping "${row.groupingStr}" has ${grouping.length} numerals for ${row.k} onsets (D2-shape bug)`,
      );
      continue;
    }
    const sum = grouping.reduce((a, b) => a + b, 0);
    if (sum !== row.n) {
      failures.push(
        `line ${row.lineno} E(${row.k},${row.n}): grouping "${row.groupingStr}" sums to ${sum}, expected ${row.n}`,
      );
      continue;
    }
    const p = parsePattern(row.patternStr);
    if (p.length !== row.n || onsetCount(p) !== row.k) continue; // caught earlier
    const actualGaps = gapSequence(p);
    // Grouping must match gap sequence exactly (the appendix prints groupings
    // for the specific rotation shown in the Pattern column).
    if (actualGaps.length !== grouping.length) continue; // already covered above
    let match = true;
    for (let i = 0; i < grouping.length; i++) {
      if (grouping[i] !== actualGaps[i]) {
        match = false;
        break;
      }
    }
    if (!match) {
      failures.push(
        `line ${row.lineno} E(${row.k},${row.n}): grouping "${row.groupingStr}" != pattern gap sequence "${actualGaps.join('+')}"`,
      );
    }
  }
  assert.equal(failures.length, 0, `\n${failures.join('\n')}`);
});
