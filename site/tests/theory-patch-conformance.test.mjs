// Theory-page construction-patch conformance harness (M069 S01).
//
// The nine "Theory Deep Dive" pages each state numbered counterpoint rules and
// then print a <PolyPatch> lane table meant to *demonstrate* those rules. The
// 2026 conformance review (git 5a8785a:docs/theory-conformance-review.md, §4)
// found that all nine tables contradict their own page's rules. This harness
// encodes, per defect, the rule-satisfaction predicate the fix must pass.
//
// Every pattern is re-derived under the canonical M068 Bjorklund convention via
// the shared verifier in ../src/lib/euclidean-claims.mjs (bjorklund(steps,hits)
// at rotation 0, then rotate() == bjorklund.ts rotateRight by the table's
// Rotation cell) — NOT transcribed from the review's engine-phase arithmetic.
//
// Failure messages mirror appendix-euclidean-claims.test.mjs: they name the
// page path, the table line number, the (steps,hits,rotation) triple, the
// derived onset set / gap sequence, and the rule violated — so a reintroduced
// wrong spelling names the exact lane and rule.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { dirname, join, relative } from 'node:path';
import { fileURLToPath } from 'node:url';

import { bjorklund, rotate, gapSequence } from '../src/lib/euclidean-claims.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const DOCS = join(HERE, '..', 'src', 'content', 'docs');
const REPO = join(HERE, '..', '..');

// --- Parsing ---------------------------------------------------------------

// Parse the first <PolyPatch> markdown table in an MDX source. Returns
// { columns, rows } where each row carries its 1-based file line number and a
// by-column-name cell map plus parsed steps/hits/rotation. The first five
// columns are canonical across every theory page: Lane | Role | Steps | Hits |
// Rotation; later columns vary per page and are read by header name.
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
      cell,
      role: cell.Role,
      steps: parseInt(cell.Steps, 10),
      hits: parseInt(cell.Hits, 10),
      rotationRaw: rotRaw,
      rotation: /^-?\d+$/.test(rotRaw) ? parseInt(rotRaw, 10) : null, // "—" => timeline
    });
  }
  return { columns: columns || [], rows };
}

async function loadPatch(slug) {
  const path = join(DOCS, `${slug}.mdx`);
  const src = await readFile(path, 'utf8');
  return { patch: parsePolyPatch(src), relPath: relative(REPO, path) };
}

// --- Derivation ------------------------------------------------------------

// Onset step indices for a lane (null for timeline lanes with "—" rotation).
function laneOnsets(row) {
  if (row.rotation === null) return null;
  const p = rotate(bjorklund(row.steps, row.hits), row.rotation);
  const out = [];
  for (let i = 0; i < p.length; i++) if (p[i]) out.push(i);
  return out;
}

// Onset positions mapped onto a page's referent cycle of N pulses. A lane of
// `steps` steps that evenly divides N contributes each onset at index*N/steps.
function laneOnPulseGrid(row, N) {
  const onsets = laneOnsets(row);
  if (onsets === null) return null;
  assert.equal(N % row.steps, 0, `lane steps ${row.steps} must divide referent cycle ${N}`);
  const mult = N / row.steps;
  return onsets.map((i) => i * mult);
}

function findLane(rows, re) {
  return rows.find((r) => re.test(r.role));
}

function laneTag(relPath, row) {
  const onsets = laneOnsets(row);
  const set = onsets === null ? 'timeline' : `{${onsets.join(',')}}`;
  const gaps = onsets === null ? '—' : gapSequence(rotate(bjorklund(row.steps, row.hits), row.rotation)).join('-');
  return `${relPath}:${row.lineno} lane "${row.role}" (steps=${row.steps},hits=${row.hits},rot=${row.rotationRaw}) onsets=${set} gaps=${gaps}`;
}

// --- Per-page conformance tests --------------------------------------------

// #1 (review §4.1) Rule 3: the bass tumbao avoids the downbeat and targets the
// bombo (pulse 3) and ponche (pulse 6) of the 16-pulse clave matrix.
test('theory-afro-cuban: tumbao dodges beat one and reaches bombo(3) and ponche(6)', async () => {
  const { patch, relPath } = await loadPatch('theory-afro-cuban');
  const lane = findLane(patch.rows, /tumbao|bass/i);
  assert.ok(lane, 'no tumbao/bass lane found');
  const grid = laneOnPulseGrid(lane, 16);
  const fail = [];
  if (grid.includes(0)) fail.push('hits beat one (pulse 0) — Rule 3 forbids beat-one bass');
  if (!grid.includes(3)) fail.push('does not reach the bombo (pulse 3) — Rule 3');
  if (!grid.includes(6)) fail.push('does not reach the ponche (pulse 6) — Rule 3');
  assert.equal(fail.length, 0, `\n${laneTag(relPath, lane)} pulses={${grid.join(',')}}\n  ${fail.join('\n  ')}`);
});

// #2 (review §4.2) funk-soul carries three defects at once.
test('theory-funk-soul: backbeat on 2&4, kick is the One+syncopes not four-on-the-floor', async () => {
  const { patch, relPath } = await loadPatch('theory-funk-soul');
  const fail = [];

  // Rule 2: snare backbeat on beats 2 and 4 (pulses 4,12), never 1/3.
  const snare = findLane(patch.rows, /backbeat/i);
  assert.ok(snare, 'no snare backbeat lane found');
  const sg = laneOnPulseGrid(snare, 16);
  if (!(sg.includes(4) && sg.includes(12) && !sg.includes(0) && !sg.includes(8)))
    fail.push(`snare backbeat pulses={${sg.join(',')}} are not beats 2&4 (pulses 4,12) — Rule 2\n  ${laneTag(relPath, snare)}`);

  // Rules 1 & 3: kick strikes the One (pulse 0), dodges the backbeat slots
  // (4,12), and is not a metronomic four-on-the-floor.
  const kick = findLane(patch.rows, /kick/i);
  assert.ok(kick, 'no kick lane found');
  const kg = laneOnPulseGrid(kick, 16);
  const fourOnFloor = [0, 4, 8, 12].every((p) => kg.includes(p)) && kg.length === 4;
  if (!kg.includes(0)) fail.push(`kick omits the One (pulse 0) — Rule 1\n  ${laneTag(relPath, kick)}`);
  if (kg.includes(4) || kg.includes(12)) fail.push(`kick lands on a backbeat slot (pulse 4 or 12) — Rule 3\n  ${laneTag(relPath, kick)}`);
  if (fourOnFloor) fail.push(`kick is four-on-the-floor {0,4,8,12} — Rule 3 wants the One + syncopes\n  ${laneTag(relPath, kick)}`);

  assert.equal(fail.length, 0, `\n${fail.join('\n')}`);
});

// #3 (review §4.3) Rule 1 / Construction 1: the surdo floor lands on the beat
// (rotation 0), answering beat one; rotation 1 lands on no beat.
test('theory-brazilian: surdo floor answers beat one and lands on beats', async () => {
  const { patch, relPath } = await loadPatch('theory-brazilian');
  const lane = findLane(patch.rows, /surdo/i);
  assert.ok(lane, 'no surdo lane found');
  const onsets = laneOnsets(lane);
  const fail = [];
  if (!onsets.includes(0)) fail.push('surdo does not answer beat one (step 0) — Rule 1 / Construction 1 (rotation 0)');
  const offBeat = onsets.filter((i) => i % 2 !== 0);
  if (offBeat.length) fail.push(`surdo strokes at steps {${offBeat.join(',')}} land between beats — Rule 1`);
  assert.equal(fail.length, 0, `\n${laneTag(relPath, lane)}\n  ${fail.join('\n  ')}`);
});

// #4 (review §4.4) Rules 2 & 4: accents live on the additive cell heads. For
// kopanitsa (cells 2+2+3+2+2) the heads are pulses {0,2,4,7,9}; the maximally
// even E(5,11) at rotation 0 gives {0,2,4,6,8}, which is NOT the additive grid.
test('theory-balkan: tupan low strikes the additive cell heads {0,2,4,7,9}', async () => {
  const { patch, relPath } = await loadPatch('theory-balkan');
  const lane = findLane(patch.rows, /tupan low|cell heads/i);
  assert.ok(lane, 'no tupan low (cell heads) lane found');
  const onsets = laneOnsets(lane);
  const cellHeads = [0, 2, 4, 7, 9];
  const same = onsets.length === cellHeads.length && onsets.every((v, i) => v === cellHeads[i]);
  assert.ok(
    same,
    `\n${laneTag(relPath, lane)}\n  onsets {${onsets.join(',')}} are not the kopanitsa cell heads {${cellHeads.join(',')}} — Rules 2 & 4 (accents on cell heads)`,
  );
});

// #5 (review §4.5) Rule 3: comping answers, it does not run — figures are one to
// four attacks. Five-attack comping is the "running the time" student error.
test('theory-jazz: comping lane is one to four attacks, not a running line', async () => {
  const { patch, relPath } = await loadPatch('theory-jazz');
  const lane = findLane(patch.rows, /comp/i);
  assert.ok(lane, 'no comping lane found');
  assert.ok(
    lane.hits >= 1 && lane.hits <= 4,
    `\n${laneTag(relPath, lane)}\n  comping has ${lane.hits} attacks; Rule 3 allows one to four`,
  );
});

// #6 (review §4.6) gamelan: the construction prose prescribes 5 polos hits (the
// patch uses 9) and four colotomic lanes (ketuk/kempul/kenong/gong — the patch
// supplies only three, omitting the fastest ketuk stratum).
test('theory-gamelan: polos has 5 hits and four colotomic strata are present', async () => {
  const { patch, relPath } = await loadPatch('theory-gamelan');
  const fail = [];

  const polos = findLane(patch.rows, /polos/i);
  assert.ok(polos, 'no polos lane found');
  if (polos.hits !== 5) fail.push(`polos has ${polos.hits} hits; Construction 3 prescribes 5\n  ${laneTag(relPath, polos)}`);

  // Colotomic strata are the single-hit cycle markers (gong/kenong/kempul/ketuk)
  // at steps 32/16/8/4. Rule 7 requires the full power-of-two ladder incl. ketuk.
  const colotomic = patch.rows.filter((r) => r.hits === 1);
  if (colotomic.length !== 4)
    fail.push(`${colotomic.length} single-hit colotomic strata; Construction 1 prescribes four (steps 4/8/16/32)`);
  if (!colotomic.some((r) => r.steps === 4))
    fail.push('no 4-step colotomic stratum (ketuk) — Construction 1 / Rule 7 power-of-two ladder');

  assert.equal(fail.length, 0, `\n${fail.join('\n')}`);
});

// #7 (review §4.7) Rule 6: the tihai is arithmetic — 3×phrase + 2×gap ≡ the full
// cycle, landing on sam. The units must be the theka's matra units; a tihai lane
// at a finer subdivision than the theka conflates matra and beat and lands on
// sam only on the first repetition.
test('theory-indian-classical: tihai closes the full cycle in the theka matra units', async () => {
  const { patch, relPath } = await loadPatch('theory-indian-classical');
  const theka = findLane(patch.rows, /theka/i);
  const tihai = findLane(patch.rows, /tihai/i);
  assert.ok(theka && tihai, 'need both a theka and a tihai lane');
  const fail = [];

  if (tihai.cell.Subdivision !== theka.cell.Subdivision)
    fail.push(
      `tihai subdivision ${tihai.cell.Subdivision} != theka subdivision ${theka.cell.Subdivision} — Rule 6 (matra/beat units conflated)`,
    );

  const len = parseFloat(tihai.cell['Phrase Len']);
  const gap = parseFloat(tihai.cell.Gap);
  const span = 3 * len + 2 * gap;
  if (span !== theka.steps)
    fail.push(`3×${len} + 2×${gap} = ${span} matras != cycle length ${theka.steps} — Rule 6 (tihai must land on sam)`);

  assert.equal(fail.length, 0, `\n${relPath}:${tihai.lineno} tihai (len=${len},gap=${gap},sub=${tihai.cell.Subdivision})\n  ${fail.join('\n  ')}`);
});

// #8 (review §4.8) Rule 3: support drums are placed OFF the dance beat,
// systematically — the part lives offbeat, it does not visit. Any onset shared
// with the dance beat E(4,12)={0,3,6,9} is a collision. A 3-hit support lane is
// arithmetically forced to collide once per 12-pulse cycle, so it cannot satisfy
// the rule as spelled and must be respelled (4 offbeat hits).
test('theory-sub-saharan-africa: support drums never collide with the dance beat', async () => {
  const { patch, relPath } = await loadPatch('theory-sub-saharan-africa');
  const dance = findLane(patch.rows, /dance beat/i);
  assert.ok(dance, 'no dance beat lane found');
  const danceSet = new Set(laneOnsets(dance));
  const supports = patch.rows.filter((r) => /kidi|sogo|support/i.test(r.role));
  assert.ok(supports.length >= 2, 'expected at least two support lanes');
  const fail = [];
  for (const s of supports) {
    const onsets = laneOnsets(s);
    const clash = onsets.filter((i) => danceSet.has(i));
    if (clash.length)
      fail.push(`support "${s.role}" onsets {${onsets.join(',')}} collide with dance beat {${[...danceSet].join(',')}} at {${clash.join(',')}} — Rule 3\n  ${laneTag(relPath, s)}`);
  }
  assert.equal(fail.length, 0, `\n${fail.join('\n')}`);
});

// #9 (review §4.9) afrobeat: (a) Construction demands a Spread column the patch
// omits; (b) Rule 2 forbids the bell and shekere sharing a stratum — they must
// not occupy the same (steps, subdivision) grid; (c) Rules 4 & 7 assign mutation
// to the colour voices, so the hat's accent-voice carries zero mutation.
test('theory-afrobeat: Spread column present, bell/shekere on separate strata, hat unmutated', async () => {
  const { patch, relPath } = await loadPatch('theory-afrobeat');
  const fail = [];

  if (!patch.columns.includes('Spread'))
    fail.push(`table columns {${patch.columns.join(', ')}} omit the Spread column the construction demands`);

  const bell = findLane(patch.rows, /bell/i);
  const shekere = findLane(patch.rows, /shekere/i);
  assert.ok(bell && shekere, 'need both a bell and a shekere lane');
  if (bell.steps === shekere.steps && bell.cell.Subdivision === shekere.cell.Subdivision)
    fail.push(
      `bell and shekere share the same stratum (steps=${bell.steps}, subdivision=${bell.cell.Subdivision}) — Rule 2 forbids poaching\n  ${relPath}:${shekere.lineno}`,
    );

  const hat = findLane(patch.rows, /hat/i);
  assert.ok(hat, 'no hat lane found');
  const hatMut = parseFloat(hat.cell.Mutation);
  if (hatMut !== 0)
    fail.push(`hat carries ${hat.cell.Mutation} mutation — Rules 4 & 7 assign the mutation budget to the colour voices\n  ${relPath}:${hat.lineno}`);

  assert.equal(fail.length, 0, `\n${fail.join('\n')}`);
});
