// Completeness gate for docs/audits/M001-theory-audit-remediation-plan.md.
//
// The plan doc names this test as the mechanical enforcement of its own
// completeness (see the "Completeness" line in the doc's front matter). It
// asserts structural invariants that a later slice would otherwise be free to
// silently break — dropping a row, blanking a token, leaving `_pending_`
// behind, or citing a phantom slice.
//
// Contract (fixed here, not scraped from the doc, so a doc + code drift is
// detected rather than silently reconciled):
//   1. Every finding ID F01–F54 appears exactly once as a ledger row.
//   2. Each finding row carries exactly one valid severity token
//      (`P0` / `P1` / `P2`).
//        - M001–M004 tables have explicit Sev/Disp columns.
//        - M005 rows have no Sev/Disp columns; the M005 section header
//          declares the whole milestone as P2 enrichment, so those rows are
//          treated as implicit P2 / `enrich`.
//   3. Each finding row carries exactly one valid disposition token
//      (`correct` / `source` / `reframe` / `disclose` / `patch-align` /
//       `enrich` / `verify` / `accept`).
//   4. Each finding row carries exactly one valid status token
//      (`open` / `in-progress` / `done` / `accepted`).
//   5. Each finding row cites a `M0xx/Syy` slice that appears in that
//      milestone's own breakdown table.
//   6. No `_pending_` placeholder survives anywhere in the ledger.
//   7. Every ledger ID cross-referenced from the "Related issues" table
//      resolves to a real finding row.
//   8. No issue is both carried in that table and declared deliberately
//      absent from it.
//   9. "Out of scope" cites only issues that table carries, so the two prose
//      sites cannot drift apart.
//
// Whether an issue is still *open on GitHub* is deliberately not asserted —
// that needs the network, and CI runs this offline. Items 7-9 guard the
// structure; keeping the table's membership true to the tracker is a human
// step at slice close.
//
// Failure messages always name the offending finding ID or issue number;
// slice-reference failures additionally name the unresolvable token.
//
// Run: node --test docs/audits/theory-audit-remediation.test.mjs

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const HERE = dirname(fileURLToPath(import.meta.url));
const DOC = join(HERE, 'M001-theory-audit-remediation-plan.md');
const md = readFileSync(DOC, 'utf8');
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

const MILESTONES = ['M001', 'M002', 'M003', 'M004', 'M005'];

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

// Extract every `M0xx/Syy` reference in a cell.
function sliceRefs(cell) {
  return (cell.match(/M0\d{2}\/S\d{2}/g) || []);
}

// Walk the doc and collect:
//   - findings: [{id, sev, disp, slice, status, rowNum, milestone}]
//   - breakdowns: Map<milestone, Set<sliceId>>   (from the "Milestone breakdown" tables)
function parseDoc() {
  const findings = [];
  const breakdowns = new Map(MILESTONES.map((m) => [m, new Set()]));

  let currentMilestone = null;
  // Table state:
  //   mode: 'finding' | 'breakdown' | null
  //   headerCells: string[] (lower-cased) for the active table
  //   idxId, idxSev, idxDisp, idxSlice, idxStatus, idxBreakdownSlice
  let table = null;

  const H3 = /^###\s+(M0\d{2})\b/;

  for (let i = 0; i < lines.length; i++) {
    const raw = lines[i];
    const line = raw.trimEnd();

    const h3 = line.match(H3);
    if (h3) {
      currentMilestone = h3[1];
      table = null;
      continue;
    }

    if (!isTableRow(line)) {
      table = null;
      continue;
    }

    if (isSeparator(line)) continue;

    const c = cells(line);

    // Detect a table header we care about by column-name shape.
    if (!table) {
      const lower = c.map((h) => h.toLowerCase());
      const idxId = lower.findIndex((h) => h === 'id');
      const idxSlice = lower.findIndex((h) => h === 'slice');
      const idxStatus = lower.findIndex((h) => h === 'status');
      const idxSev = lower.findIndex((h) => h === 'sev');
      const idxDisp = lower.findIndex((h) => h === 'disp');
      const idxBreakdownSlice = lower.findIndex((h) => h === 'slice');
      const idxScope = lower.findIndex((h) => h === 'scope');
      const idxFindings = lower.findIndex((h) => h === 'findings');

      if (idxId !== -1 && idxSlice !== -1 && idxStatus !== -1) {
        table = {
          mode: 'finding',
          idxId,
          idxSev,
          idxDisp,
          idxSlice,
          idxStatus,
        };
        continue;
      }
      if (idxBreakdownSlice !== -1 && idxScope !== -1 && idxFindings !== -1) {
        table = {
          mode: 'breakdown',
          idxSlice: idxBreakdownSlice,
        };
        continue;
      }
      // Not a table we track — leave table null so the next header can be
      // detected without a stale layout.
      continue;
    }

    if (table.mode === 'finding') {
      const id = c[table.idxId];
      if (!/^F\d+$/.test(id)) {
        // Not a finding row (e.g. stray row shape) — drop the table state so
        // the next real header re-arms it.
        table = null;
        continue;
      }
      const sevCell = table.idxSev !== -1 ? c[table.idxSev] : '';
      const dispCell = table.idxDisp !== -1 ? c[table.idxDisp] : '';
      findings.push({
        id,
        sev: table.idxSev !== -1 ? backtickToken(sevCell) : 'P2',
        disp: table.idxDisp !== -1 ? backtickToken(dispCell) : 'enrich',
        sevExplicit: table.idxSev !== -1,
        dispExplicit: table.idxDisp !== -1,
        slice: c[table.idxSlice] || '',
        status: backtickToken(c[table.idxStatus] || ''),
        row: line,
        rowNum: i + 1,
        milestone: currentMilestone,
      });
      continue;
    }

    if (table.mode === 'breakdown') {
      const sliceCell = c[table.idxSlice] || '';
      const m = sliceCell.match(/^S\d{2}$/);
      if (m && currentMilestone) {
        breakdowns.get(currentMilestone).add(sliceCell);
      }
      continue;
    }
  }

  return { findings, breakdowns };
}

const { findings, breakdowns } = parseDoc();
const byId = new Map(findings.map((f) => [f.id, f]));

test('doc exists and is substantial', () => {
  assert.ok(md.length > 5000, 'plan doc should be substantial');
});

test('every finding ID F01–F54 appears exactly once', () => {
  const missing = EXPECTED_IDS.filter((id) => !byId.has(id));
  assert.deepEqual(missing, [], `missing finding IDs: ${missing.join(', ')}`);

  const counts = new Map();
  for (const f of findings) counts.set(f.id, (counts.get(f.id) || 0) + 1);
  const duplicates = [...counts.entries()].filter(([, n]) => n > 1).map(([id]) => id);
  assert.deepEqual(duplicates, [], `duplicate finding rows: ${duplicates.join(', ')}`);

  const unexpected = findings.map((f) => f.id).filter((id) => !EXPECTED_IDS.includes(id));
  assert.deepEqual(unexpected, [], `unexpected finding IDs: ${unexpected.join(', ')}`);
});

test('every finding row carries exactly one valid severity token', () => {
  for (const id of EXPECTED_IDS) {
    const f = byId.get(id);
    if (!f) continue;
    assert.ok(
      f.sev !== null && SEVERITIES.has(f.sev),
      `${id}: severity token missing or invalid (got ${JSON.stringify(f.sev)}; expected one of P0/P1/P2)`,
    );
  }
});

test('every finding row carries exactly one valid disposition token', () => {
  for (const id of EXPECTED_IDS) {
    const f = byId.get(id);
    if (!f) continue;
    assert.ok(
      f.disp !== null && DISPOSITIONS.has(f.disp),
      `${id}: disposition token missing or invalid (got ${JSON.stringify(f.disp)}; expected one of ${[...DISPOSITIONS].join('/')})`,
    );
  }
});

test('every finding row carries exactly one valid status token', () => {
  for (const id of EXPECTED_IDS) {
    const f = byId.get(id);
    if (!f) continue;
    assert.ok(
      f.status !== null && STATUSES.has(f.status),
      `${id}: status token missing or invalid (got ${JSON.stringify(f.status)}; expected one of ${[...STATUSES].join('/')})`,
    );
  }
});

test('every finding row cites a slice present in that milestone\'s breakdown table', () => {
  for (const id of EXPECTED_IDS) {
    const f = byId.get(id);
    if (!f) continue;
    const refs = sliceRefs(f.slice);
    assert.ok(
      refs.length >= 1,
      `${id}: slice cell has no M0xx/Syy reference (cell: ${JSON.stringify(f.slice)})`,
    );
    for (const ref of refs) {
      const [milestone, slice] = ref.split('/');
      const known = breakdowns.get(milestone);
      assert.ok(
        known && known.size > 0,
        `${id}: slice reference ${ref} names milestone ${milestone} which has no breakdown table in this doc`,
      );
      assert.ok(
        known.has(slice),
        `${id}: slice reference ${ref} does not resolve — ${milestone}'s breakdown table has slices {${[...known].sort().join(', ')}}`,
      );
    }
  }
});

test('no _pending_ placeholder survives in any finding row', () => {
  for (const f of findings) {
    assert.ok(!f.row.includes('_pending_'), `${f.id}: row still contains _pending_ placeholder`);
  }
});

// --- Related-issues table -----------------------------------------------
//
// The "Related issues" section claims to enumerate every GitHub issue
// overlapping this plan's remit, and the "Out of scope" section names a subset
// of the same issues. Whether an issue is *open on GitHub* is not
// checkable offline, so these cases guard the parts that are: that the table's
// ledger cross-references resolve, that an issue is not simultaneously tabled
// and declared absent, and that the two prose sites cite the same issue set.
// The desync these were written against was an "Out of scope" list naming only
// two of the seven engine issues the table carries.

const ISSUES_HEADING = '## Related issues';
const OUT_OF_SCOPE_HEADING = '## Out of scope';

function sectionLines(heading) {
  const start = lines.findIndex((l) => l.trimEnd() === heading);
  assert.ok(start !== -1, `plan doc has no "${heading}" section`);
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

const issuesSection = sectionLines(ISSUES_HEADING);

// Rows of the issues table: first cell is an issue link, second is ledger IDs.
const issueRows = [];
for (const line of issuesSection) {
  const t = line.trimEnd();
  if (!isTableRow(t) || isSeparator(t)) continue;
  const c = cells(t);
  if (c.length < 3) continue;
  const num = c[0].match(/#(\d+)\b/);
  if (!num) continue; // header row
  issueRows.push({ issue: num[1], ledger: c[1], relationship: c[2] });
}

// Prose after the table records issues deliberately left out, so their absence
// is a judgement rather than an oversight.
const absentProse = issuesSection
  .filter((l) => !isTableRow(l.trimEnd()))
  .join('\n');
const absentIssues = issueNumbers(absentProse);

test('Related issues: the table is non-empty and every row cites resolvable ledger findings', () => {
  assert.ok(issueRows.length > 0, 'Related issues table has no issue rows');
  for (const row of issueRows) {
    const ids = row.ledger.match(/F\d{2}/g) || [];
    assert.ok(
      ids.length >= 1 || row.ledger === '—',
      `#${row.issue}: ledger cell names no finding (cell: ${JSON.stringify(row.ledger)})`,
    );
    for (const id of ids) {
      assert.ok(
        byId.has(id),
        `#${row.issue}: ledger cell cites ${id}, which is not a finding row in this doc`,
      );
    }
  }
});

test('Related issues: no issue is both tabled and declared deliberately absent', () => {
  const tabled = new Set(issueRows.map((r) => r.issue));
  const both = [...tabled].filter((n) => absentIssues.has(n));
  assert.deepEqual(
    both,
    [],
    `issue(s) both listed in the table and declared absent: ${both.map((n) => `#${n}`).join(', ')}`,
  );
});

test('Out of scope cites only issues the Related-issues table carries', () => {
  const tabled = new Set(issueRows.map((r) => r.issue));
  const cited = issueNumbers(sectionLines(OUT_OF_SCOPE_HEADING).join('\n'));
  const unlisted = [...cited].filter((n) => !tabled.has(n));
  assert.deepEqual(
    unlisted,
    [],
    `Out of scope names issue(s) absent from the Related issues table: ${unlisted
      .map((n) => `#${n}`)
      .join(', ')}`,
  );
});
