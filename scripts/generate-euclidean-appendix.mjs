#!/usr/bin/env node
// Regenerate the Pattern and Grouping columns in
// site/src/content/docs/appendix-euclidean-reference.mdx from the fixture at
// site/src/data/euclidean-appendix.json.
//
// The fixture declares WHICH (k,n) rows exist per step-count section, plus
// each row's hand-authored Traditional Association and Chapter columns.
// This script computes the E(k,n) label, the Pattern column, and the Grouping
// column from bjorklund() so those three can never drift from the shared
// reference implementation in site/src/lib/euclidean-claims.mjs.
//
// One BEGIN/END GENERATED region per step-count section (e.g. "euclidean-4",
// "euclidean-16"). Section-scoped so a future addition of a new (k,n) group
// (e.g. n=15) only touches its own region.
//
// M048 S14.

import { readFile, writeFile } from 'node:fs/promises';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

import { bjorklund, gapSequence, rotate } from '../site/src/lib/euclidean-claims.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO_ROOT = resolve(HERE, '..');
const FIXTURE_PATH = resolve(REPO_ROOT, 'site/src/data/euclidean-appendix.json');
const APPENDIX_PATH = resolve(
  REPO_ROOT,
  'site/src/content/docs/appendix-euclidean-reference.mdx',
);

function log(msg) {
  process.stdout.write(`[generate-euclidean-appendix] ${msg}\n`);
}

function rotatedPattern(k, n, rotation) {
  const pat = bjorklund(n, k);
  return rotate(pat, rotation);
}

function renderPatternCell(pat) {
  // `x` for onset, `.` for rest, space-separated — matches the appendix's
  // hand-authored style so byte-for-byte preservation is possible on first run.
  return pat.map((b) => (b ? 'x' : '.')).join(' ');
}

function renderGroupingCell(pat) {
  // Gap sequence between onsets, wrapping. `+`-joined so it reads as
  // arithmetic (each numeral sums to n). Matches S08's verifier expectation.
  return gapSequence(pat).join('+');
}

function renderRow(row, n) {
  const { k, association, chapter, rotation = 0 } = row;
  const pat = rotatedPattern(k, n, rotation);
  const pattern = renderPatternCell(pat);
  const grouping = renderGroupingCell(pat);
  return `| E(${k},${n}) | \`${pattern}\` | ${grouping} | ${association} | ${chapter} |`;
}

function renderSection(section) {
  const { n, rows } = section;
  const header = [
    '| E(k,n) | Pattern | Grouping | Traditional Association | Chapter |',
    '|--------|---------|----------|------------------------|---------|',
  ];
  const body = rows.map((r) => renderRow(r, n));
  return [...header, ...body].join('\n');
}

function replaceRegion(source, regionName, newBody) {
  // Idempotent JSX-comment marker replacement, per the S05/S07 pattern.
  const begin = `{/* BEGIN GENERATED: ${regionName} */}`;
  const end = `{/* END GENERATED: ${regionName} */}`;
  const beginIdx = source.indexOf(begin);
  const endIdx = source.indexOf(end);
  if (beginIdx === -1 || endIdx === -1) {
    throw new Error(
      `region ${regionName}: markers not found (begin=${beginIdx}, end=${endIdx}). ` +
        `Add "${begin}" and "${end}" around the section table in the appendix.`,
    );
  }
  if (endIdx < beginIdx) {
    throw new Error(`region ${regionName}: END marker precedes BEGIN`);
  }
  const before = source.slice(0, beginIdx + begin.length);
  const after = source.slice(endIdx);
  return `${before}\n${newBody}\n${after}`;
}

async function main() {
  const fixture = JSON.parse(await readFile(FIXTURE_PATH, 'utf8'));
  if (!Array.isArray(fixture.sections) || fixture.sections.length === 0) {
    process.stderr.write('[generate-euclidean-appendix] fixture has no sections\n');
    process.exit(1);
  }

  let source = await readFile(APPENDIX_PATH, 'utf8');
  const rewritten = [];
  for (const section of fixture.sections) {
    const regionName = `euclidean-${section.n}`;
    const body = renderSection(section);
    source = replaceRegion(source, regionName, body);
    rewritten.push(`${regionName} (${section.rows.length} rows)`);
  }

  await writeFile(APPENDIX_PATH, source);
  log(`rewrote ${rewritten.length} region(s): ${rewritten.join(', ')}`);
}

main().catch((err) => {
  process.stderr.write(`[generate-euclidean-appendix] ${err.message}\n`);
  process.exit(1);
});
