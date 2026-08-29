// Completeness gate for docs/plans/theory-audit/ledger.md.
//
// The ledger is the plan of record for the M001-M005 music-theory audit
// remediation. `jk-standards ledger` checks its structure — ID shape, status
// vocabulary, definition-of-done presence, validation tokens, placeholders —
// but has no idea that this particular programme owes exactly 54 findings, and
// does not look at the Related-issues list at all. That is this file's job.
// The overlap on status tokens and placeholders is deliberate belt-and-braces.
//
// Contract (fixed here, not scraped from the ledger, so a ledger + code drift
// is detected rather than silently reconciled):
//   1. Every finding ID F01-F54 appears exactly once as a ledger row.
//   2. Every row ID is well-formed: `F` + two digits (an audit finding) or
//      `H` + two digits (a harness row this programme added for itself).
//   3. Each row carries exactly one valid severity token
//      (`P0` / `P1` / `P2`). Unlike the retired plan doc, the ledger carries
//      explicit Sev/Disp columns on every row including M005's, so an absent
//      token is a failure rather than an implicit P2.
//   4. Each row carries exactly one valid disposition token
//      (`correct` / `source` / `reframe` / `disclose` / `patch-align` /
//       `enrich` / `verify` / `accept`).
//   5. Each row carries exactly one valid status token
//      (`open` / `in-progress` / `done` / `accepted`).
//   6. Every row sits under a real slice by nesting, and no row table declares
//      a `Slice` column. The retired plan doc linked rows to slices through
//      such a column plus a separate breakdown table, which let the two
//      disagree; nesting makes "every row belongs to a real slice" true by
//      construction.
//   7. No `_pending_` placeholder survives anywhere in the ledger.
//   8. Every ledger ID cross-referenced from the "Related issues" table
//      resolves to a real row.
//   9. No issue is both carried in that table and declared deliberately
//      absent from it.
//  10. "Out of scope" cites only issues that table carries, so the two prose
//      sites cannot drift apart.
//
// Whether an issue is still *open on GitHub* is deliberately not asserted —
// that needs the network, and CI runs this offline. Items 8-10 guard the
// structure; keeping the membership true to the tracker is a human step at
// slice close.
//
// Failure messages always name the offending row ID or issue number.
//
// Run: node --test docs/audits/theory-audit-remediation.test.mjs

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const HERE = dirname(fileURLToPath(import.meta.url));
const LEDGER = join(HERE, '..', 'plans', 'theory-audit', 'ledger.md');
const md = readFileSync(LEDGER, 'utf8');
const lines = md.split('\n');

const SEVERITIES = new Set(['P0', 'P1', 'P2']);
const DISPOSITIONS = new Set([
  'correct',
  'source',
  'reframe',
  'disclose',
  'patch-align',
  'enrich',
  'verify',
  'accept',
]);
const STATUSES = new Set(['open', 'in-progress', 'done', 'accepted']);

// The full finding-ID contract. Duplicated here rather than scraped so a row
// deleted from BOTH the ledger and the code still fails loudly.
const EXPECTED_IDS = Array.from({ length: 54 }, (_, i) => `F${String(i + 1).padStart(2, '0')}`);

// Split a markdown table row into trimmed cell strings (drops the leading and
// trailing empties produced by the border pipes).
function cells(row) {
  const parts = row.split('|');
  return parts.slice(1, parts.length - 1).map((c) => c.trim());
}

function isTableRow(line) {
  const t = line.trimEnd();
  return t.startsWith('|') && t.endsWith('|');
}

function isSeparator(row) {
  return cells(row).every((c) => /^:?-+:?$/.test(c));
}

// Pull the first backticked token from a cell (e.g. "`P0`" → "P0"). Returns
// null if the cell doesn't hold exactly one backticked token as its content
// (loose whitespace is tolerated).
function backtickToken(cell) {
  const m = cell.match(/^`([^`]+)`$/);
  return m ? m[1] : null;
}

// Walk the ledger and collect:
//   - rows: [{id, sev, disp, status, milestone, slice, row, rowNum}]
//   - sliceIds: Set<'M001/S07'>   (every slice heading seen)
//   - sliceColumnSeen: boolean    (any row table declaring a `Slice` column)
//
// Rows attach to the slice heading above them. There is no breakdown table to
// resolve against, which is the point: nesting cannot disagree with itself.
function parseLedger() {
  const rows = [];
  const sliceIds = new Set();
  let sliceColumnSeen = false;

  const MILESTONE_H2 = /^##\s+Milestone\s+(M\d{3})\b/;
  const SLICE_H3 = /^###\s+Slice\s+(M\d{3})\/(S\d{2})\b/;

  let currentMilestone = null;
  let currentSlice = null;
  let table = null;

  for (let i = 0; i < lines.length; i++) {
    const line = lines[i].trimEnd();

    const mh = line.match(MILESTONE_H2);
    if (mh) {
      currentMilestone = mh[1];
      currentSlice = null;
      table = null;
      continue;
    }

    const sh = line.match(SLICE_H3);
    if (sh) {
      currentSlice = `${sh[1]}/${sh[2]}`;
      sliceIds.add(currentSlice);
      table = null;
      continue;
    }

    if (!isTableRow(line)) {
      table = null;
      continue;
    }

    if (isSeparator(line)) continue;

    const c = cells(line);

    if (!table) {
      const lower = c.map((h) => h.toLowerCase());
      const idxId = lower.indexOf('id');
      const idxItem = lower.indexOf('item');
      const idxStatus = lower.indexOf('status');

      if (idxId !== -1 && idxItem !== -1 && idxStatus !== -1) {
        if (lower.includes('slice')) sliceColumnSeen = true;
        table = {
          idxId,
          idxSev: lower.indexOf('sev'),
          idxDisp: lower.indexOf('disp'),
          idxStatus,
        };
      }
      continue;
    }

    const id = c[table.idxId];
    if (!/^[FH]\d{2}$/.test(id)) {
      // Not a row of this table — drop the layout so the next header re-arms.
      table = null;
      continue;
    }

    rows.push({
      id,
      sev: backtickToken(c[table.idxSev] || ''),
      disp: backtickToken(c[table.idxDisp] || ''),
      status: backtickToken(c[table.idxStatus] || ''),
      milestone: currentMilestone,
      slice: currentSlice,
      row: line,
      rowNum: i + 1,
    });
  }

  return { rows, sliceIds, sliceColumnSeen };
}

const { rows, sliceIds, sliceColumnSeen } = parseLedger();
const byId = new Map(rows.map((r) => [r.id, r]));
const findingRows = rows.filter((r) => r.id.startsWith('F'));

test('ledger exists and is substantial', () => {
  assert.ok(md.length > 5000, 'ledger should be substantial');
});

test('every finding ID F01\u2013F54 appears exactly once', () => {
  const seen = new Set(findingRows.map((r) => r.id));
  const missing = EXPECTED_IDS.filter((id) => !seen.has(id));
  assert.deepEqual(missing, [], `missing finding IDs: ${missing.join(', ')}`);

  const counts = new Map();
  for (const r of findingRows) counts.set(r.id, (counts.get(r.id) || 0) + 1);
  const duplicates = [...counts.entries()].filter(([, n]) => n > 1).map(([id]) => id);
  assert.deepEqual(duplicates, [], `duplicate finding rows: ${duplicates.join(', ')}`);

  const unexpected = findingRows.map((r) => r.id).filter((id) => !EXPECTED_IDS.includes(id));
  assert.deepEqual(unexpected, [], `unexpected finding IDs: ${unexpected.join(', ')}`);
});

test('every row carries a well-formed ID', () => {
  // The F-only case above cannot see an `H`-prefixed typo, because a malformed
  // harness ID simply never enters findingRows.
  const malformed = rows.filter((r) => !/^(F\d{2}|H\d{2})$/.test(r.id)).map((r) => r.id);
  assert.deepEqual(malformed, [], `malformed row IDs: ${malformed.join(', ')}`);
});

test('every row carries exactly one valid severity token', () => {
  for (const r of rows) {
    assert.ok(
      r.sev !== null && SEVERITIES.has(r.sev),
      `${r.id}: severity token missing or invalid (got ${JSON.stringify(r.sev)}; expected one of P0/P1/P2)`,
    );
  }
});

test('every row carries exactly one valid disposition token', () => {
  for (const r of rows) {
    assert.ok(
      r.disp !== null && DISPOSITIONS.has(r.disp),
      `${r.id}: disposition token missing or invalid (got ${JSON.stringify(r.disp)}; expected one of ${[...DISPOSITIONS].join('/')})`,
    );
  }
});

test('every row carries exactly one valid status token', () => {
  for (const r of rows) {
    assert.ok(
      r.status !== null && STATUSES.has(r.status),
      `${r.id}: status token missing or invalid (got ${JSON.stringify(r.status)}; expected one of ${[...STATUSES].join('/')})`,
    );
  }
});

test('every row sits under a real slice, and no row table declares a Slice column', () => {
  assert.equal(
    sliceColumnSeen,
    false,
    'row tables must link rows to slices by nesting, not by a Slice column',
  );

  assert.ok(
    sliceIds.size >= 1,
    'no slice headings parsed — the nesting check would otherwise pass vacuously',
  );

  const orphans = rows.filter((r) => r.slice === null).map((r) => r.id);
  assert.deepEqual(orphans, [], `row(s) not nested under any slice: ${orphans.join(', ')}`);
});

test('no _pending_ placeholder survives in any row', () => {
  for (const r of rows) {
    assert.ok(!r.row.includes('_pending_'), `${r.id}: row still contains _pending_ placeholder`);
  }
});

// --- Related issues -----------------------------------------------------
//
// The "Related issues" section claims to enumerate every GitHub issue
// overlapping this programme's remit, and "Out of scope" names a subset of the
// same issues. Whether an issue is *open on GitHub* is not checkable offline,
// so these cases guard the parts that are: that the list's row cross-references
// resolve, that an issue is not simultaneously listed and declared absent, and
// that the two prose sites cite the same issue set. The desync these were
// written against was an "Out of scope" list naming only two of the seven
// engine issues the section carried.
//
// The ledger writes this section as a bullet list rather than a table, because
// the `ledger` check's parser reads any table under a slice as that slice's
// rows. Entries wrap across lines, so continuation lines are folded into the
// bullet above before the issue number and row references are read.
//
// NOTE: the second guard is currently VACUOUS. The ledger carries no
// absent-prose, so `absentIssues` is empty and nothing can collide with the
// listed set. It is retained deliberately: it costs nothing and it bites the
// day someone records a deliberately-absent issue in this section. An
// always-passing test that nobody has explained is worse than no test, so this
// is the explanation.

const ISSUES_HEADING = '## Related issues';
const OUT_OF_SCOPE_HEADING = '## Out of scope';

function sectionLines(heading) {
  const start = lines.findIndex((l) => l.trimEnd() === heading);
  assert.ok(start !== -1, `ledger has no "${heading}" section`);
  let end = lines.length;
  for (let i = start + 1; i < lines.length; i++) {
    if (/^##\s/.test(lines[i])) {
      end = i;
      break;
    }
  }
  return lines.slice(start + 1, end);
}

function issueNumbers(text) {
  return new Set((text.match(/#(\d+)\b/g) || []).map((m) => m.slice(1)));
}

const isBullet = (l) => /^\s*-\s+/.test(l);
const isContinuation = (l) => /^\s+\S/.test(l);

const issuesSection = sectionLines(ISSUES_HEADING);

// Fold each bullet and its wrapped continuation lines into one string, so an
// issue number and its row reference are seen together even when the paragraph
// breaks between them.
const issueEntries = [];
for (const line of issuesSection) {
  if (isBullet(line)) {
    issueEntries.push(line);
  } else if (issueEntries.length && isContinuation(line)) {
    issueEntries[issueEntries.length - 1] += ' ' + line.trim();
  }
}

const issueBullets = issueEntries
  .map((text) => {
    const num = text.match(/#(\d+)\b/);
    return num ? { issue: num[1], rowRefs: text.match(/[FH]\d{2}/g) || [], text } : null;
  })
  .filter(Boolean);

// Prose that is neither a bullet nor its continuation records issues
// deliberately left out, so their absence is a judgement rather than an
// oversight.
const absentProse = issuesSection
  .filter((l) => !isBullet(l) && !isContinuation(l))
  .join('\n');
const absentIssues = issueNumbers(absentProse);

test('Related issues: the list is non-empty and every entry cites resolvable ledger rows', () => {
  assert.ok(issueBullets.length > 0, 'Related issues list has no issue entries');
  for (const entry of issueBullets) {
    assert.ok(
      entry.rowRefs.length >= 1,
      `#${entry.issue}: entry names no ledger row (entry: ${JSON.stringify(entry.text)})`,
    );
    for (const id of entry.rowRefs) {
      assert.ok(
        byId.has(id),
        `#${entry.issue}: entry cites ${id}, which is not a row in this ledger`,
      );
    }
  }
});

test('Related issues: no issue is both listed and declared deliberately absent', () => {
  const listed = new Set(issueBullets.map((e) => e.issue));
  const both = [...listed].filter((n) => absentIssues.has(n));
  assert.deepEqual(
    both,
    [],
    `issue(s) both listed and declared absent: ${both.map((n) => `#${n}`).join(', ')}`,
  );
});

test('Out of scope cites only issues the Related-issues list carries', () => {
  const listed = new Set(issueBullets.map((e) => e.issue));
  const cited = issueNumbers(sectionLines(OUT_OF_SCOPE_HEADING).join('\n'));
  const unlisted = [...cited].filter((n) => !listed.has(n));
  assert.deepEqual(
    unlisted,
    [],
    `Out of scope names issue(s) absent from the Related issues list: ${unlisted
      .map((n) => `#${n}`)
      .join(', ')}`,
  );
});
