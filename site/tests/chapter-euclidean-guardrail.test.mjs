// Page-agnostic doc-arithmetic guardrail for the fifteen numbered chapter pages
// (M071 S02 T03).
//
// The companion of theory-euclidean-guardrail.test.mjs (M069 S02), which guards
// the nine Theory Deep-Dive pages. This file extends the same page-agnostic,
// per-lane canonical-spelling invariant to chapters 01-15 so a reintroduced
// wrong EuclideanDiagram prop, PolyPatch rotation, or hit count on ANY chapter
// fails CI. It makes no per-page rule claim: it extracts the
// (steps, hits, rotation) triple for EVERY Euclidean artifact on EVERY chapter —
// both PolyPatch table lanes AND inline <EuclideanDiagram> components — and
// asserts the triple is a canonical Bjorklund spelling under the M068 convention
// via the same shared verifier the EuclideanDiagram component, the theory-page
// guardrail, and the appendix test all import
// (../src/lib/euclidean-claims.mjs — bjorklund/rotate/onsetCount). Chapters
// therefore cannot fork a second generator.
//
// Two shapes distinguish this from the theory guardrail, both driven by the
// recon note that chapters carry MULTIPLE Euclidean blocks per file:
//   1. Multi-block PolyPatch parser. The theory parser `break`s on the first
//      </PolyPatch>; chapters have up to three PolyPatch tables per file, so the
//      parser here collects every block and resets its column header per block.
//   2. EuclideanDiagram coverage. Chapters render standalone <EuclideanDiagram>
//      components (the canonical artifact readers see) in addition to PolyPatch
//      tables; both are validated.
//
// It also fails loudly if a chapter is missing, unreadable, or carries no
// parseable Euclidean artifact, so silently dropping a chapter from coverage is
// itself a failure (enumeration IS the coverage contract).
//
// Scope note: the two appendices are excluded by the S02 slice contract.
// appendix-euclidean-reference.mdx is already guarded by
// appendix-euclidean-claims.test.mjs, and appendix-presets.mdx's PolyPatch
// tables are owned by S04.
//
// Failure messages name the chapter path, the artifact line number, the lane
// Role (or "EuclideanDiagram" for a component), the (steps,hits,rotation)
// triple, and the violated invariant — mirroring theory-euclidean-guardrail.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { dirname, join, relative } from 'node:path';
import { fileURLToPath } from 'node:url';

import { bjorklund, rotate, onsetCount } from '../src/lib/euclidean-claims.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const DOCS = join(HERE, '..', 'src', 'content', 'docs');
const REPO = join(HERE, '..', '..');

// The fifteen numbered chapters. This list is the coverage contract: if a slug
// is dropped here or its file goes missing, the enumeration test fails rather
// than silently shrinking the guarded surface.
const CHAPTER_SLUGS = [
  '01-foundations',
  '02-sub-saharan-africa',
  '03-afro-cuban',
  '04-afrobeat',
  '05-gamelan',
  '06-indian-classical',
  '07-balkan',
  '08-minimalism',
  '09-electronic',
  '10-brazilian',
  '11-funk-soul',
  '12-jazz',
  '13-drum-and-bass',
  '14-synthesis',
  '15-compositional-grammar',
];

// --- Parsing: PolyPatch tables ---------------------------------------------

// Locate the rotation column. Chapters spell it "Rotation" or (04-afrobeat)
// "Rot". A block with neither (08-minimalism's Riley/Nancarrow tables) has no
// Euclidean rotation dimension, so its lanes are read at the engine's default
// rotation 0 — matching EuclideanDiagram's `rotation = 0` default.
function rotationKey(columns) {
  return columns.find((c) => /^rot(ation)?$/i.test(c)) || null;
}

// Parse EVERY <PolyPatch> markdown table in an MDX source (multi-block). Each
// block resets its own column header. Returns an array of blocks; each block is
// { columns, rows } where every row carries its 1-based file line number, lane
// Role, and parsed steps/hits/rotation. A Rotation cell of "—" (em dash) or any
// other non-integer marks a timeline lane with no Euclidean spelling; rotation
// is left null in that case and the caller skips it.
function parsePolyPatchBlocks(src) {
  const lines = src.split('\n');
  const blocks = [];
  let inBlock = false;
  let columns = null;
  let rows = null;
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    if (line.includes('<PolyPatch')) {
      inBlock = true;
      columns = null;
      rows = [];
      continue;
    }
    if (line.includes('</PolyPatch>')) {
      if (inBlock) blocks.push({ columns: columns || [], rows: rows || [] });
      inBlock = false;
      columns = null;
      rows = null;
      continue;
    }
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
    const rotKey = rotationKey(columns);
    // No rotation column => engine default rotation 0.
    const rotRaw = rotKey ? cell[rotKey] : '0';
    rows.push({
      kind: 'lane',
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
  return blocks;
}

// --- Parsing: EuclideanDiagram components ----------------------------------

// Parse every <EuclideanDiagram .../> component. steps and hits are required
// props; rotation is optional and defaults to 0 (matching the component's
// `rotation = 0` default in EuclideanDiagram.astro). Each diagram is a single
// line in every chapter, so a per-line scan is sufficient and keeps line
// numbers exact.
function parseDiagrams(src) {
  const lines = src.split('\n');
  const diagrams = [];
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    const re = /<EuclideanDiagram\b([^>]*?)\/?>/g;
    let m;
    while ((m = re.exec(line)) !== null) {
      const attrs = m[1];
      const stepsM = /\bsteps=\{(-?\d+)\}/.exec(attrs);
      const hitsM = /\bhits=\{(-?\d+)\}/.exec(attrs);
      const rotM = /\brotation=\{(-?\d+)\}/.exec(attrs);
      diagrams.push({
        kind: 'diagram',
        lineno: i + 1,
        role: 'EuclideanDiagram',
        stepsRaw: stepsM ? stepsM[1] : String(attrs.trim()),
        hitsRaw: hitsM ? hitsM[1] : String(attrs.trim()),
        steps: stepsM ? parseInt(stepsM[1], 10) : NaN,
        hits: hitsM ? parseInt(hitsM[1], 10) : NaN,
        rotationRaw: rotM ? rotM[1] : '0',
        rotation: rotM ? parseInt(rotM[1], 10) : 0,
      });
    }
  }
  return diagrams;
}

async function loadChapter(slug) {
  const path = join(DOCS, `${slug}.mdx`);
  const src = await readFile(path, 'utf8');
  return {
    blocks: parsePolyPatchBlocks(src),
    diagrams: parseDiagrams(src),
    relPath: relative(REPO, path),
  };
}

// --- The invariant ---------------------------------------------------------

// Verify one artifact's (steps, hits, rotation) is a canonical Bjorklund
// spelling. Returns an array of human-readable violation strings (empty => the
// artifact is a well-formed canonical spelling). Timeline lanes (rotation ===
// null) are skipped by the caller and never reach here. Identical to the
// invariant in theory-euclidean-guardrail.test.mjs so both harnesses judge
// spellings by exactly the same rule.
//
// The invariants, all under the M068 convention:
//   1. steps parses to a positive integer.
//   2. hits parses to an integer with 0 < hits <= steps.
//   3. rotation parses to an integer.
//   4. onset-count invariant: rotate(bjorklund(steps,hits), rotation) has
//      exactly `hits` onsets. bjorklund() clamps hits<=0 to all-rests and
//      hits>=steps to all-onsets, so this catches any triple where the stated
//      hit count cannot be realized as a canonical distribution.
function validateArtifact(row) {
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
      `${triple}: rotate(bjorklund(${row.steps},${row.hits}),${row.rotation}) yields ${onsets} onsets, not the ${row.hits} the ${row.kind} claims — not a canonical Bjorklund spelling`,
    );
  }
  return violations;
}

function artifactTag(relPath, row) {
  return `${relPath}:${row.lineno} ${row.kind} "${row.role}" (steps=${row.stepsRaw},hits=${row.hitsRaw},rot=${row.rotationRaw})`;
}

// --- Enumeration contract --------------------------------------------------

test('all fifteen chapter pages are present and carry a parseable Euclidean artifact', async () => {
  const fail = [];
  for (const slug of CHAPTER_SLUGS) {
    let chapter;
    try {
      chapter = await loadChapter(slug);
    } catch (err) {
      fail.push(`${slug}.mdx: missing or unreadable — ${err.message}`);
      continue;
    }
    const laneCount = chapter.blocks.reduce((n, b) => n + b.rows.length, 0);
    if (laneCount === 0 && chapter.diagrams.length === 0) {
      fail.push(
        `${chapter.relPath}: no parseable <PolyPatch> lane rows and no <EuclideanDiagram> — chapter dropped from coverage`,
      );
      continue;
    }
    // Every PolyPatch *lane* table must carry the Steps/Hits spelling columns.
    // The Rotation column is optional (some blocks have no rotation dimension).
    // Non-spelling PolyPatch blocks — e.g. 14-synthesis's "Setting | Value"
    // scene-chain config table — carry no numbered lane rows and are correctly
    // ignored, so the column requirement applies only to blocks with lane rows.
    for (const block of chapter.blocks) {
      if (block.rows.length === 0) continue; // not a spelling table
      for (const required of ['Role', 'Steps', 'Hits']) {
        if (!block.columns.includes(required))
          fail.push(`${chapter.relPath}: a PolyPatch lane table is missing required "${required}" column`);
      }
    }
  }
  assert.equal(fail.length, 0, `\n${fail.join('\n')}`);
});

// --- Per-artifact canonical-spelling guardrail -----------------------------

test('every PolyPatch lane on every chapter is a canonical Bjorklund spelling', async () => {
  const fail = [];
  for (const slug of CHAPTER_SLUGS) {
    const { blocks, relPath } = await loadChapter(slug);
    for (const block of blocks) {
      for (const row of block.rows) {
        if (row.rotation === null) continue; // timeline lane — no Euclidean spelling
        const violations = validateArtifact(row);
        for (const v of violations) fail.push(`${artifactTag(relPath, row)}\n  ${v}`);
      }
    }
  }
  assert.equal(
    fail.length,
    0,
    `\n${fail.length} malformed PolyPatch lane spelling(s):\n${fail.join('\n')}`,
  );
});

test('every EuclideanDiagram on every chapter is a canonical Bjorklund spelling', async () => {
  const fail = [];
  for (const slug of CHAPTER_SLUGS) {
    const { diagrams, relPath } = await loadChapter(slug);
    for (const d of diagrams) {
      const violations = validateArtifact(d);
      for (const v of violations) fail.push(`${artifactTag(relPath, d)}\n  ${v}`);
    }
  }
  assert.equal(
    fail.length,
    0,
    `\n${fail.length} malformed EuclideanDiagram spelling(s):\n${fail.join('\n')}`,
  );
});

// --- Teeth: the guardrail is not vacuous -----------------------------------

// Prove validateArtifact rejects the malformed spellings a bad construction
// patch would reintroduce. If any of these synthetic artifacts passed, the
// guardrail above would be vacuous and could not catch a real regression.
test('teeth: validateArtifact rejects malformed spellings a wrong patch would reintroduce', () => {
  const bad = [
    { name: 'hits exceed steps', row: { kind: 'lane', stepsRaw: '5', hitsRaw: '7', rotationRaw: '0', steps: 5, hits: 7, rotation: 0 } },
    { name: 'zero hits', row: { kind: 'lane', stepsRaw: '8', hitsRaw: '0', rotationRaw: '0', steps: 8, hits: 0, rotation: 0 } },
    { name: 'non-integer rotation', row: { kind: 'lane', stepsRaw: '8', hitsRaw: '3', rotationRaw: '1.5', steps: 8, hits: 3, rotation: null } },
    { name: 'non-positive steps', row: { kind: 'lane', stepsRaw: '0', hitsRaw: '0', rotationRaw: '0', steps: 0, hits: 0, rotation: 0 } },
    { name: 'diagram hits exceed steps', row: { kind: 'diagram', stepsRaw: '8', hitsRaw: '9', rotationRaw: '0', steps: 8, hits: 9, rotation: 0 } },
  ];
  for (const { name, row } of bad) {
    const violations = validateArtifact(row);
    assert.ok(violations.length > 0, `guardrail failed to reject "${name}" — it is vacuous`);
  }
});

// And prove the guardrail ACCEPTS well-formed canonical spellings, so it is not
// rejecting everything (a differently-vacuous failure mode).
test('teeth: validateArtifact accepts well-formed canonical spellings', () => {
  const good = [
    { kind: 'lane', stepsRaw: '8', hitsRaw: '3', rotationRaw: '0', steps: 8, hits: 3, rotation: 0 },
    { kind: 'lane', stepsRaw: '16', hitsRaw: '5', rotationRaw: '3', steps: 16, hits: 5, rotation: 3 },
    { kind: 'lane', stepsRaw: '12', hitsRaw: '7', rotationRaw: '-2', steps: 12, hits: 7, rotation: -2 },
    { kind: 'lane', stepsRaw: '4', hitsRaw: '4', rotationRaw: '0', steps: 4, hits: 4, rotation: 0 }, // full lane
    { kind: 'diagram', stepsRaw: '16', hitsRaw: '7', rotationRaw: '2', steps: 16, hits: 7, rotation: 2 },
  ];
  for (const row of good) {
    const violations = validateArtifact(row);
    assert.equal(
      violations.length,
      0,
      `guardrail wrongly rejected canonical (${row.steps},${row.hits},${row.rotation}): ${violations.join('; ')}`,
    );
  }
});

// --- Teeth: the multi-block parser sees past the first PolyPatch ------------

// The theory parser reads only the FIRST PolyPatch per file. Chapters carry
// several; a regression hidden in a later block must still be caught. This
// proves the parser collects every block and every EuclideanDiagram from a
// synthetic multi-block source.
test('teeth: parser collects every PolyPatch block and every EuclideanDiagram in a file', () => {
  const src = [
    '<EuclideanDiagram steps={8} hits={3} />',
    '<PolyPatch title="A">',
    '',
    '| Lane | Role | Steps | Hits | Rotation |',
    '|------|------|-------|------|----------|',
    '| 1 | First | 8 | 3 | 0 |',
    '',
    '</PolyPatch>',
    '',
    '<EuclideanDiagram steps={16} hits={5} rotation={3} />',
    '',
    '<PolyPatch title="B" preset="X">',
    '',
    '| Lane | Role | Steps | Hits | Rot |',
    '|------|------|-------|------|-----|',
    '| 1 | Second | 12 | 7 | 2 |',
    '| 2 | Third | 7 | 5 | 0 |',
    '',
    '</PolyPatch>',
  ].join('\n');

  const blocks = parsePolyPatchBlocks(src);
  assert.equal(blocks.length, 2, 'parser must collect BOTH PolyPatch blocks, not just the first');
  assert.equal(blocks[0].rows.length, 1);
  assert.equal(blocks[1].rows.length, 2);
  // The "Rot" alias in block B must resolve to the rotation dimension.
  assert.equal(blocks[1].rows[0].rotation, 2);
  assert.equal(blocks[1].rows[1].role, 'Third');

  const diagrams = parseDiagrams(src);
  assert.equal(diagrams.length, 2, 'parser must collect BOTH EuclideanDiagram components');
  assert.equal(diagrams[0].rotation, 0, 'diagram with no rotation prop defaults to 0');
  assert.equal(diagrams[1].rotation, 3);
});

// A PolyPatch block with no rotation column reads its lanes at rotation 0, not
// as a parse failure (08-minimalism's Riley/Nancarrow tables have no Rotation
// column). Proves the default-rotation path is exercised.
test('teeth: a PolyPatch block with no rotation column defaults to rotation 0', () => {
  const src = [
    '<PolyPatch title="No rotation dimension">',
    '',
    '| Lane | Role | Steps | Hits | Subdivision | Offset |',
    '|------|------|-------|------|-------------|--------|',
    '| 1 | Foundation | 8 | 5 | 1/8 | 4 |',
    '',
    '</PolyPatch>',
  ].join('\n');
  const blocks = parsePolyPatchBlocks(src);
  assert.equal(blocks.length, 1);
  const row = blocks[0].rows[0];
  assert.equal(row.rotation, 0, 'missing rotation column => rotation 0');
  // The Offset column (4) must NOT be misread as a rotation.
  assert.equal(row.rotationRaw, '0');
  assert.equal(validateArtifact(row).length, 0, 'the lane is a canonical spelling at rotation 0');
});
