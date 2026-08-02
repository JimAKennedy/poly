// Page-agnostic doc-arithmetic guardrail for the nine Theory Deep-Dive pages
// (M069 S02).
//
// S01's theory-patch-conformance harness encodes, per defect, the *rule* each
// page's <PolyPatch> table must satisfy. This guardrail is the complementary
// page-agnostic layer: it makes no per-page rule claim. Instead it extracts the
// (steps, hits, rotation) triple for EVERY non-timeline lane on EVERY theory
// page and asserts the triple is a canonical Bjorklund spelling under the M068
// convention, via the same shared verifier S01 uses
// (../src/lib/euclidean-claims.mjs — bjorklund/rotate). So a future construction
// patch that reintroduces a malformed spelling (hits>steps, non-integer
// rotation, onset-count mismatch) on ANY theory page — including lanes S01's
// per-defect predicates do not touch — fails CI here.
//
// It also fails loudly if a page is missing or its PolyPatch table cannot be
// parsed, so silently dropping a page from coverage is itself a failure.
//
// Failure messages mirror appendix-euclidean-claims.test.mjs and S01's harness:
// they name the page path, the table line number, the lane Role, the
// (steps,hits,rotation) triple, and the violated invariant — so a reintroduced
// wrong spelling names the exact page and lane.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { dirname, join, relative } from 'node:path';
import { fileURLToPath } from 'node:url';

import { bjorklund, rotate, onsetCount } from '../src/lib/euclidean-claims.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const DOCS = join(HERE, '..', 'src', 'content', 'docs');
const REPO = join(HERE, '..', '..');

// The nine Theory Deep-Dive pages. This list is the coverage contract: if a
// slug is dropped here or its file goes missing, the enumeration test fails
// rather than silently shrinking the guarded surface. Matches the nine slugs
// S01's theory-patch-conformance harness rule-checks.
const THEORY_SLUGS = [
  'theory-afro-cuban',
  'theory-afrobeat',
  'theory-balkan',
  'theory-brazilian',
  'theory-funk-soul',
  'theory-gamelan',
  'theory-indian-classical',
  'theory-jazz',
  'theory-sub-saharan-africa',
];

// --- Parsing ---------------------------------------------------------------

// Parse the first <PolyPatch> markdown table in an MDX source. Returns
// { columns, rows } where each row carries its 1-based file line number, the
// lane Role, and parsed steps/hits/rotation. The first five columns are
// canonical across every theory page: Lane | Role | Steps | Hits | Rotation.
// A Rotation cell of "—" (em dash) marks a timeline lane with no Euclidean
// spelling; rotation is left null in that case. Mirrors the parser in
// theory-patch-conformance.test.mjs so both harnesses read tables identically.
function parsePolyPatch(src) {
  const lines = src.split('\n');
  let inBlock = false;
  let columns = null;
  const rows = [];
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    if (line.includes('<PolyPatch')) {
      inBlock = true;
      continue;
    }
    if (line.includes('</PolyPatch>')) break;
    if (!inBlock) continue;
    if (!/^\s*\|/.test(line)) continue;
    const cells = line
      .split('|')
      .slice(1, -1)
      .map((c) => c.trim());
    if (cells.every((c) => /^-+$/.test(c))) continue; // separator row
    if (!columns) {
      columns = cells; // header row
      continue;
    }
    if (!/^\d+$/.test(cells[0])) continue; // only numbered lane rows
    const cell = {};
    for (let c = 0; c < columns.length; c++) cell[columns[c]] = cells[c];
    const rotRaw = cell.Rotation;
    rows.push({
      lineno: i + 1,
      role: cell.Role,
      stepsRaw: cell.Steps,
      hitsRaw: cell.Hits,
      steps: parseInt(cell.Steps, 10),
      hits: parseInt(cell.Hits, 10),
      rotationRaw: rotRaw,
      // "—" (or any non-integer) => timeline lane, no Euclidean spelling.
      rotation: /^-?\d+$/.test(rotRaw) ? parseInt(rotRaw, 10) : null,
    });
  }
  return { columns: columns || [], rows };
}

async function loadPatch(slug) {
  const path = join(DOCS, `${slug}.mdx`);
  const src = await readFile(path, 'utf8');
  return { patch: parsePolyPatch(src), relPath: relative(REPO, path) };
}

// --- The invariant ---------------------------------------------------------

// Verify one lane's (steps, hits, rotation) is a canonical Bjorklund spelling.
// Returns an array of human-readable violation strings (empty => the lane is a
// well-formed canonical spelling). Timeline lanes (rotation === null) are
// skipped by the caller and never reach here.
//
// The invariants, all under the M068 convention:
//   1. steps parses to a positive integer.
//   2. hits parses to an integer with 0 < hits <= steps.
//   3. rotation parses to an integer (already guaranteed by the caller's guard,
//      re-checked here so the function is safe to call on synthetic lanes).
//   4. onset-count invariant: rotate(bjorklund(steps,hits), rotation) has
//      exactly `hits` onsets. bjorklund() clamps hits<=0 to all-rests and
//      hits>=steps to all-onsets, so this catches any triple where the stated
//      hit count cannot be realized as a canonical distribution.
function validateLane(row) {
  const violations = [];
  const triple = `(steps=${row.stepsRaw},hits=${row.hitsRaw},rotation=${row.rotationRaw})`;

  if (!/^\d+$/.test(String(row.stepsRaw)) || !Number.isInteger(row.steps) || row.steps <= 0) {
    violations.push(`${triple}: Steps must be a positive integer`);
    return violations; // cannot derive a pattern without valid steps
  }
  if (!Number.isInteger(row.hits)) {
    violations.push(`${triple}: Hits must be an integer`);
    return violations;
  }
  if (row.hits <= 0 || row.hits > row.steps) {
    violations.push(`${triple}: requires 0 < hits <= steps (M068 Bjorklund domain)`);
  }
  if (!Number.isInteger(row.rotation)) {
    violations.push(`${triple}: Rotation must be an integer`);
    return violations;
  }

  const pattern = rotate(bjorklund(row.steps, row.hits), row.rotation);
  const onsets = onsetCount(pattern);
  if (onsets !== row.hits) {
    violations.push(
      `${triple}: rotate(bjorklund(${row.steps},${row.hits}),${row.rotation}) yields ${onsets} onsets, not the ${row.hits} the table claims — not a canonical Bjorklund spelling`,
    );
  }
  return violations;
}

function laneTag(relPath, row) {
  return `${relPath}:${row.lineno} lane "${row.role}" (steps=${row.stepsRaw},hits=${row.hitsRaw},rot=${row.rotationRaw})`;
}

// --- Enumeration contract --------------------------------------------------

test('all nine theory pages are present and their PolyPatch table parses', async () => {
  const fail = [];
  for (const slug of THEORY_SLUGS) {
    let patch, relPath;
    try {
      ({ patch, relPath } = await loadPatch(slug));
    } catch (err) {
      fail.push(`${slug}.mdx: missing or unreadable — ${err.message}`);
      continue;
    }
    if (patch.rows.length === 0) {
      fail.push(`${relPath}: no parseable <PolyPatch> lane rows — page dropped from coverage`);
      continue;
    }
    // The first five columns are the canonical spelling surface.
    for (const required of ['Role', 'Steps', 'Hits', 'Rotation']) {
      if (!patch.columns.includes(required))
        fail.push(`${relPath}: PolyPatch table missing required "${required}" column`);
    }
  }
  assert.equal(fail.length, 0, `\n${fail.join('\n')}`);
});

// --- Per-lane canonical-spelling guardrail ---------------------------------

test('every lane on every theory page is a canonical Bjorklund spelling', async () => {
  const fail = [];
  for (const slug of THEORY_SLUGS) {
    const { patch, relPath } = await loadPatch(slug);
    for (const row of patch.rows) {
      if (row.rotation === null) continue; // timeline lane — no Euclidean spelling
      const violations = validateLane(row);
      for (const v of violations) fail.push(`${laneTag(relPath, row)}\n  ${v}`);
    }
  }
  assert.equal(
    fail.length,
    0,
    `\n${fail.length} malformed lane spelling(s):\n${fail.join('\n')}`,
  );
});

// --- Teeth: the guardrail is not vacuous -----------------------------------

// Prove validateLane rejects the malformed spellings a bad construction patch
// would reintroduce. If any of these synthetic lanes passed, the guardrail
// above would be vacuous and could not catch a real regression.
test('teeth: validateLane rejects malformed spellings a wrong patch would reintroduce', () => {
  const bad = [
    { name: 'hits exceed steps', row: { stepsRaw: '5', hitsRaw: '7', rotationRaw: '0', steps: 5, hits: 7, rotation: 0 } },
    { name: 'zero hits', row: { stepsRaw: '8', hitsRaw: '0', rotationRaw: '0', steps: 8, hits: 0, rotation: 0 } },
    { name: 'non-integer rotation', row: { stepsRaw: '8', hitsRaw: '3', rotationRaw: '1.5', steps: 8, hits: 3, rotation: null } },
    { name: 'non-positive steps', row: { stepsRaw: '0', hitsRaw: '0', rotationRaw: '0', steps: 0, hits: 0, rotation: 0 } },
  ];
  for (const { name, row } of bad) {
    const violations = validateLane(row);
    assert.ok(violations.length > 0, `guardrail failed to reject "${name}" — it is vacuous`);
  }
});

// And prove the guardrail ACCEPTS well-formed canonical spellings, so it is not
// rejecting everything (a differently-vacuous failure mode).
test('teeth: validateLane accepts well-formed canonical spellings', () => {
  const good = [
    { stepsRaw: '8', hitsRaw: '3', rotationRaw: '0', steps: 8, hits: 3, rotation: 0 },
    { stepsRaw: '16', hitsRaw: '5', rotationRaw: '3', steps: 16, hits: 5, rotation: 3 },
    { stepsRaw: '12', hitsRaw: '7', rotationRaw: '-2', steps: 12, hits: 7, rotation: -2 },
    { stepsRaw: '4', hitsRaw: '4', rotationRaw: '0', steps: 4, hits: 4, rotation: 0 }, // full lane
  ];
  for (const row of good) {
    const violations = validateLane(row);
    assert.equal(
      violations.length,
      0,
      `guardrail wrongly rejected canonical (${row.steps},${row.hits},${row.rotation}): ${violations.join('; ')}`,
    );
  }
});
