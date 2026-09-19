#!/usr/bin/env node
// check-ledger-row-ids.mjs — row IDs within a delivery ledger are unique.
//
// jk-standards' `ledger` check validates structure, statuses and plan paths,
// but not that two rows never share an ID. That gap let guide-parity M004 add a
// row numbered GP11 while M005/S01 already held GP11 further down the same
// file: "the next number after GP10" was picked without reading the rest of it,
// and every gate stayed green through the milestone and its merge.
//
// The cost of a collision is specific. Commit trailers carry `Rows: <id>`, and
// `git log --grep` is how the tree and the plan are joined in either direction.
// Two rows sharing an ID makes that join ambiguous: the query returns commits
// from unrelated milestones and a reader cannot tell which row a trailer meant.
//
// Scoped per ledger file, not globally: two programmes may legitimately number
// their rows from the same sequence, and it is only within one ledger that an
// ID has to resolve to one row.

import { readFile, readdir } from 'node:fs/promises';
import { existsSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join, relative } from 'node:path';

const REPO = dirname(dirname(fileURLToPath(import.meta.url)));
const PLANS = join(REPO, 'docs', 'plans');

// A row line looks like: | GP11 | ... | ... |
const ROW = /^\|\s*([A-Z]{1,4}\d{1,3})\s*\|/;

async function* ledgers(dir) {
  for (const entry of await readdir(dir, { withFileTypes: true })) {
    const full = join(dir, entry.name);
    if (entry.isDirectory()) yield* ledgers(full);
    else if (entry.name === 'ledger.md') yield full;
  }
}

if (!existsSync(PLANS)) {
  console.log('[check-ledger-row-ids] no docs/plans directory — nothing to check');
  process.exit(0);
}

const problems = [];
let ledgerCount = 0;
let rowCount = 0;

for await (const path of ledgers(PLANS)) {
  ledgerCount += 1;
  const rel = relative(REPO, path);
  const seen = new Map();
  const lines = (await readFile(path, 'utf8')).split('\n');

  lines.forEach((line, i) => {
    const m = line.match(ROW);
    if (!m) return;
    const id = m[1];
    rowCount += 1;
    if (seen.has(id)) problems.push(`${rel}: row ${id} appears on lines ${seen.get(id)} and ${i + 1}`);
    else seen.set(id, i + 1);
  });
}

// A parse that silently matches nothing would make this a green check over an
// empty set — the failure mode CLAUDE.md records for the jk-standards rules that
// passed while matching no lines at all.
if (ledgerCount === 0 || rowCount === 0) {
  console.error(
    `[check-ledger-row-ids] found ${ledgerCount} ledger(s) and ${rowCount} row(s) — refusing to report a pass over nothing`,
  );
  process.exit(1);
}

console.log(`[check-ledger-row-ids] ${rowCount} row(s) across ${ledgerCount} ledger(s)`);

if (problems.length === 0) {
  console.log('[check-ledger-row-ids] every row ID is unique within its ledger');
  process.exit(0);
}

console.error(`\n[check-ledger-row-ids] ${problems.length} duplicate row ID(s):\n`);
for (const p of problems) console.error(`  ${p}`);
console.error(
  '\nCommit trailers carry `Rows: <id>`, so a duplicate makes `git log --grep`\n' +
    'ambiguous. Renumber the row that no commit references yet — grep the whole\n' +
    'ledger before choosing a new ID, not just the milestone you are editing.\n',
);
process.exit(1);
