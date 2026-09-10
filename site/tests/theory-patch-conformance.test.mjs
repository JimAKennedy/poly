// Theory-page construction-patch conformance harness (M069 S01).
//
// The nine "Theory Deep Dive" pages each state numbered counterpoint rules and
// then print a <PolyPatch> lane table meant to *demonstrate* those rules. The
// 2026 conformance review (`docs/reviews/2026-07-30-theory-conformance-review.md`,
// §4 — tracked on main since 2026-09-08; previously only as git 5a8785a)
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
import presetsData from '../src/generated/presets.json' with { type: 'json' };
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
// `title` selects one of a file's <PolyPatch> blocks by its title attribute.
// Omitting it keeps the original behaviour exactly — the first block in the
// file — because the nine review-defect tests above call it that way and this
// parser must stay a pure extension for them (M004/S05 task 1).
function parsePolyPatch(src, title = null) {
  const lines = src.split('\n');
  let inBlock = false;
  let columns = null;
  const rows = [];
  const opensWanted = (line) =>
    title === null ? line.includes('<PolyPatch') : line.includes(`<PolyPatch title="${title}"`);
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    if (!inBlock && opensWanted(line)) {
      inBlock = true;
      continue;
    }
    if (!inBlock) continue;
    if (line.includes('</PolyPatch>')) break;
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

// --- M004/S05: multi-patch parsing ----------------------------------------

// Chapter 2 carries two patches (Ewe, Manding) and Chapter 5 two (Balinese
// kotekan, Javanese colotomy). parsePolyPatch read the first table in a file,
// so the second of each was unreachable — and the first was only addressable
// by position, which is not the same as saying which one you meant.
//
// Asserted against the file, not against transcribed lane data: the default
// parse must equal the first title's parse, and the second title's must differ.
test('S05-parse-by-title: each <PolyPatch> in a file is addressable by title', async () => {
  const path = join(DOCS, '05-gamelan.mdx');
  const src = await readFile(path, 'utf8');

  const titles = [...src.matchAll(/<PolyPatch title="([^"]+)"/g)].map((m) => m[1]);
  assert.ok(
    titles.length >= 2,
    `05-gamelan.mdx is expected to carry two patches; found ${titles.length} (${titles.join(', ')})`,
  );

  const roles = (patch) => patch.rows.map((r) => r.role);
  const first = parsePolyPatch(src, titles[0]);
  const second = parsePolyPatch(src, titles[1]);

  assert.ok(first.rows.length > 0, `no lane rows parsed for "${titles[0]}"`);
  assert.ok(second.rows.length > 0, `no lane rows parsed for "${titles[1]}"`);

  assert.deepEqual(
    roles(parsePolyPatch(src)),
    roles(first),
    'omitting the title must stay byte-identical to the previous behaviour: the ' +
      "first patch in the file. The nine existing tests depend on this default.",
  );
  assert.notDeepEqual(
    roles(second),
    roles(first),
    `selecting "${titles[1]}" returned the same lanes as "${titles[0]}" — the ` +
      'title is being ignored and the second patch is still unreachable',
  );
});

// --- M004/S05: the named-rule checklist ------------------------------------

// The nine tests above hand-write one predicate per known defect. That cannot
// notice a *new* contradiction: an unchecked page is invisible. The checklist
// below is declared as data and iterated, so a page with no entry is a missing
// row in a structure rather than an absence nobody can see.
//
// Rule ids are descriptive, not positional. A page that renumbers its rules
// would leave a positional id pointing at the wrong text silently.
//
// Coverage here is M004/S05's: the three chapter patches its sibling slices
// need, plus the only two theory pages carrying no assertion at all. The
// remaining rules across the other nine theory pages are M007's — see the
// ledger, not this comment, for what that owns.

// A divergence marker is an MDX comment sitting immediately above the patch it
// concerns:
//
//   {/* patch-divergence-ok: <rule-id> — <written reason> */}
//
// It satisfies exactly the rule it names, on exactly the patch that follows it.
// Not the page, not the next rule, not by proximity. In-band so it cannot drift
// from what it excuses; greppable on one fixed token so `grep -rn
// patch-divergence-ok site/` is the audit report; reasoned; and counted, below,
// so the number cannot shrink into silence.
const MARKER_RE = /\{\/\*\s*patch-divergence-ok:\s*([a-z0-9-]+)\s*—\s*([\s\S]*?)\*\/\}/g;

function markersFor(src) {
  const found = new Map();
  for (const m of src.matchAll(MARKER_RE)) {
    const after = src.slice(m.index + m[0].length);
    const next = after.match(/<PolyPatch title="([^"]+)"/);
    if (!next) continue; // a marker with no patch after it excuses nothing
    found.set(`${next[1]}::${m[1]}`, m[2].trim().replace(/\s+/g, ' '));
  }
  return found;
}

const cellPct = (row, col) => parseFloat(String(row.cell[col] ?? '').replace('%', ''));
const cellNum = (row, col) => parseFloat(String(row.cell[col] ?? ''));

const CHECKLIST = [
  {
    page: 'theory-minimalism.mdx',
    patch: 'Rule-Checked Phase Study',
    rules: [
      {
        id: 'min-one-variable',
        description: 'Rule 1: one process at a time, moving one variable',
        check: ({ rows }) => {
          const moving = rows.filter((r) => cellNum(r, 'Drift') !== 0);
          return moving.length === 1
            ? null
            : `${moving.length} lanes carry non-zero Drift (${moving.map((r) => r.role).join(', ') || 'none'}); ` +
                'Rule 1 moves exactly one variable';
        },
      },
      {
        id: 'min-voices-flat',
        description: 'Rule 4: voices are dynamically flat and timbrally near-identical',
        check: ({ rows }) => {
          const voices = rows.filter((r) => /voice/i.test(r.role));
          const vels = new Set(voices.map((r) => cellNum(r, 'Velocity')));
          return vels.size === 1
            ? null
            : `the ${voices.length} voice lanes carry velocities ${[...vels].join(', ')}; ` +
                'Rule 4 keeps them dynamically flat';
        },
      },
      {
        id: 'min-deterministic',
        description: 'Rule 8: determinism is the aesthetic',
        check: ({ rows }) => {
          const random = rows.filter((r) => cellPct(r, 'Mutation') !== 0);
          return random.length === 0
            ? null
            : `${random.map((r) => r.role).join(', ')} carry non-zero Mutation; Rule 8 is deterministic`;
        },
      },
    ],
  },
  {
    page: 'theory-electronic-breakbeat.mdx',
    patch: 'Rule-Checked Jungle Frame',
    rules: [
      {
        id: 'ebb-anchor-immutable',
        description: 'Rule 1: the anchor is immutable',
        check: ({ rows }) => {
          const anchor = findLane(rows, /snare/i);
          if (!anchor) return 'no snare lane found; Rule 1 needs an anchor to be immutable';
          return cellPct(anchor, 'Mutation') === 0
            ? null
            : `the anchor "${anchor.role}" carries ${anchor.cell.Mutation} Mutation; Rule 1 keeps it immutable`;
        },
      },
      {
        id: 'ebb-kick-avoids-snare',
        description: "Rule 7: the kick syncopates against the snare, avoiding its slots",
        check: ({ rows }) => {
          const snare = findLane(rows, /snare/i);
          const kick = findLane(rows, /kick/i);
          if (!snare || !kick) return 'need both a snare and a kick lane to check Rule 7';
          const sg = laneOnPulseGrid(snare, 16);
          const kg = laneOnPulseGrid(kick, 16);
          const clash = kg.filter((p) => sg.includes(p));
          return clash.length === 0
            ? null
            : `kick onsets ${JSON.stringify(kg)} intersect snare slots ${JSON.stringify(sg)} at ` +
                `${JSON.stringify(clash)}; Rule 7 has the kick avoid them`;
        },
      },
    ],
  },
  {
    page: 'theory-afro-cuban.mdx',
    patch: 'Rule-Checked Son Ensemble (3-2)',
    rules: [
      {
        id: 'ac-theory-one-free-voice',
        description: 'Rule 5: the variation budget belongs to one voice at a time',
        check: ({ rows }) => {
          const busy = rows.filter((r) => cellPct(r, 'Mutation') > 0);
          return busy.length <= 1
            ? null
            : `${busy.length} lanes carry a variation budget ` +
                `(${busy.map((r) => `${r.role} ${r.cell.Mutation}`).join(', ')}); ` +
                'Rule 5 gives it to one voice at a time, and the construction names the quinto';
        },
      },
      {
        id: 'ac-tumbao-onsets-rendered',
        description: "Rule 3: the tumbao's onset positions are printed, not left to be derived",
        check: ({ rows }, { block }) => {
          const tumbao = findLane(rows, /tumbao/i);
          if (!tumbao) return 'no tumbao lane found';
          const want = laneOnsets(tumbao);
          if (!want) return 'the tumbao lane is in timeline mode; its onsets cannot be derived here';
          // Asserted against the derivation, never against a literal: the point
          // is that the printed prose and the (steps, hits, rotation) spelling
          // cannot drift apart. A hard-coded list would let them.
          const printed = block.match(/\{([0-9,\s]+)\}/);
          if (!printed) {
            return `rotation ${tumbao.rotation} is an unusual spelling and the page prints no onset ` +
              `set for it; Rule 3's claim rests on ${JSON.stringify(want)}`;
          }
          const got = printed[1].split(',').map((n) => parseInt(n.trim(), 10));
          const same = got.length === want.length && got.every((v, i) => v === want[i]);
          return same
            ? null
            : `the page prints onsets ${JSON.stringify(got)} but ` +
                `E(${tumbao.hits},${tumbao.steps}) at rotation ${tumbao.rotation} is ${JSON.stringify(want)}`;
        },
      },
    ],
  },
  {
    page: '02-sub-saharan-africa.mdx',
    patch: 'Ewe-Inspired Polymetric Ensemble',
    rules: [
      {
        id: 'ssa-dance-beat',
        description: 'construction step 2: a low drum lays the dance beat, 12 steps, 4 hits',
        check: ({ rows }) => {
          const beat = rows.find((r) => r.steps === 12 && r.hits === 4);
          return beat
            ? null
            : 'no lane at 12 steps / 4 hits — theory-sub-saharan-africa construction step 2 lays ' +
                'the dance beat as E(4,12), every third pulse, and the parts are learned against it';
        },
      },
      {
        id: 'ssa-timeline-legible',
        description: "Rule 1: the timeline's fixity is legible from the table",
        check: ({ columns }) =>
          columns.includes('Timeline')
            ? null
            : `the table has no Timeline column (${columns.join(', ')}), so a reader cannot see ` +
              'that the bell runs in timeline mode — which Rule 1 makes the patch depend on',
      },
    ],
  },
  {
    page: '03-afro-cuban.mdx',
    patch: 'Cuban Son Ensemble',
    rules: [
      {
        id: 'ac-clave-approximation',
        description: 'the clave lane names itself the Euclidean approximation and links the exact construction',
        check: ({ rows }, { block }) => {
          const clave = findLane(rows, /clave/i);
          if (!clave) return 'no clave lane found';
          const problems = [];
          if (!/approx/i.test(clave.cell.Role)) {
            problems.push(`the clave lane header reads "${clave.cell.Role}" and does not mark itself an approximation`);
          }
          if (!/\]\(\//.test(block)) {
            problems.push('the patch carries no link to the exact-timeline construction');
          }
          return problems.length ? problems.join('; ') : null;
        },
      },
      {
        id: 'ac-one-free-voice',
        description: 'Rule 5: the variation budget belongs to one voice at a time',
        check: (_patch, { preset }) => {
          if (!preset) return 'preset "Cuban Son Montuno" not found in src/generated/presets.json';
          const busy = preset.lanes.filter((l) => (l.mutationRate ?? 0) > 0);
          return busy.length <= 1
            ? null
            : `${busy.length} lanes carry a mutation budget (${busy.map((l) => l.roleLabel).join(', ')}); ` +
                'theory-afro-cuban Rule 5 gives it to one voice at a time';
        },
      },
    ],
  },
  {
    page: '05-gamelan.mdx',
    patch: 'Balinese Kotekan Interlocking',
    rules: [
      {
        id: 'gam-pokok-layer',
        description: 'Rule 6: the interlock serves a pokok melody',
        check: ({ rows }) =>
          findLane(rows, /pokok/i)
            ? null
            : `no pokok lane among ${rows.map((r) => r.role).join(', ')} — theory-gamelan Rule 6 ` +
              'makes the kotekan an ornamentation system over a slower core melody',
      },
      {
        id: 'gam-structural-overlap',
        description: 'Rule 4: a deliberate doubling marks the cycle boundary',
        // Construction step 4, not Rule 4's headline. Rule 4 asks the pair to
        // overlap at structural tones, but its own parenthetical says Poly's
        // Kotekan L-mode implements the strict case and that doublings come
        // "via a third lane or accent masks until kotekan modes ship" — and
        // theory-gamelan's own reference patch uses L6. A predicate demanding
        // pair-overlap therefore condemns the worked example the rule is drawn
        // from, and earns a suppression nobody could ever burn down. So this
        // checks what step 4 specifies: a lane outside the pair striking the
        // gong point together with a pair lane.
        check: ({ rows }) => {
          const polos = findLane(rows, /polos/i);
          const sangsih = findLane(rows, /sangsih/i);
          if (!polos || !sangsih) return 'need both a polos and a sangsih lane to check Rule 4';
          // Sangsih's own triple is not what sounds when its Kotekan cell is
          // L<n>: the engine derives its onsets from the source lane. Polos is
          // the pair member the table actually describes.
          const pair = new Set([polos.role, sangsih.role]);
          const atBoundary = (r) => (laneOnsets(r) ?? []).includes(0);
          if (!atBoundary(polos)) {
            return `polos sounds on ${JSON.stringify(laneOnsets(polos))} and not on the cycle ` +
              'boundary, so nothing can double it there; Rule 4 marks structure at the gong point';
          }
          const doubling = rows.filter((r) => !pair.has(r.role) && atBoundary(r));
          return doubling.length > 0
            ? null
            : 'no lane outside the kotekan pair sounds on the cycle boundary, so the gong point ' +
                'carries no deliberate doubling — construction step 4 adds a sparse third lane there';
        },
      },
    ],
  },
];

const PRESETS = Array.isArray(presetsData) ? presetsData : (presetsData.presets ?? []);

let liveMarkers = 0;

for (const entry of CHECKLIST) {
  for (const rule of entry.rules) {
    test(`${entry.page} [${rule.id}]: ${rule.description}`, async () => {
      const path = join(DOCS, entry.page);
      const src = await readFile(path, 'utf8');
      const rel = relative(REPO, path);
      const patch = parsePolyPatch(src, entry.patch);
      assert.ok(
        patch.rows.length > 0,
        `${rel}: the checklist names patch "${entry.patch}" but no lane rows were parsed for it — ` +
          'the patch was renamed, removed, or its table is malformed',
      );

      // The patch's own source window, and the preset it binds to if it names
      // one — a rule about a header or a mutation budget cannot be settled from
      // the parsed lane table alone.
      const start = src.indexOf(`<PolyPatch title="${entry.patch}"`);
      const block = src.slice(start, src.indexOf('</PolyPatch>', start));
      const presetName = block.match(/preset="([^"]+)"/)?.[1] ?? null;
      const preset = presetName ? (PRESETS.find((p) => p.name === presetName) ?? null) : null;

      const excuse = markersFor(src).get(`${entry.patch}::${rule.id}`);
      const failure = rule.check(patch, { src, block, preset });
      if (excuse) {
        liveMarkers += 1;
        assert.ok(
          failure,
          `${rel}: "${entry.patch}" carries a patch-divergence-ok marker for ${rule.id}, but the rule ` +
            'passes — remove the marker rather than leaving a suppression nobody needs',
        );
        return;
      }
      assert.equal(failure, null, `${rel} "${entry.patch}" [${rule.id}]: ${failure}`);
    });
  }
}

test('patch-divergence-ok suppression count', () => {
  // Printed, not asserted against a number: a rising count is signal, and an
  // invisible count is rot. The assertion that matters is above — a marker on a
  // rule that passes fails its own case.
  console.log(`  patch-divergence-ok: ${liveMarkers} live suppression(s)`);
  assert.ok(liveMarkers >= 0);
});
