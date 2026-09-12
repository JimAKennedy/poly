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
import { readFile, readdir } from 'node:fs/promises';
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
    page: 'theory-indian-classical.mdx',
    patch: 'Rule-Checked Tintal Structure',
    rules: [
      {
        id: 'ind-tihai-worked',
        description: "Rule 6: the tihai's arithmetic is worked through, not just stated",
        check: ({ rows }, { block }) => {
          const tihai = findLane(rows, /tihai/i);
          if (!tihai) return 'no tihai lane found';
          const phrase = cellNum(tihai, 'Phrase Len');
          const gap = cellNum(tihai, 'Gap');
          const cycle = tihai.steps;
          if (!Number.isFinite(phrase) || !Number.isFinite(gap)) {
            return 'the tihai lane carries no Phrase Len / Gap cells to work from';
          }
          // Derived from the lane, never from a literal: a test carrying its own
          // copy of 5, 0.5 and 16 would check nothing about the page, and the
          // row's whole point is that the arithmetic is checked rather than
          // asserted.
          const product = 3 * phrase + 2 * gap;
          // Only the prose counts, not the table the numbers came from.
          const lines = block.split('\n');
          const lastRow = lines.map((l) => /^\s*\|/.test(l)).lastIndexOf(true);
          const prose = lines.slice(lastRow + 1).join('\n');
          const missing = [String(phrase), String(gap), String(product)].filter(
            (n) => !new RegExp(`(^|[^0-9.])${n.replace('.', '\\.')}([^0-9]|$)`).test(prose),
          );
          if (missing.length) {
            return `the page states Rule 6's formula but never works it: the prose beside the patch ` +
              `does not print ${missing.join(', ')} (3 × ${phrase} + 2 × ${gap} = ${product})`;
          }
          return product === cycle
            ? null
            : `3 × ${phrase} + 2 × ${gap} = ${product}, which does not close the lane's ` +
                `${cycle}-matra cycle; Rule 6 says a tihai that misses sam is a failed one`;
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

// --- M007/S02: the rule rollout, page by page ------------------------------

// Helpers shared by the rolled-out rules. Each returns null when the rule holds
// and a message naming what the patch does instead when it does not.
const pct = (row, col) => parseFloat(String(row.cell[col] ?? '').replace('%', ''));
const num = (row, col) => parseFloat(String(row.cell[col] ?? ''));
const roles = (rows) => rows.map((r) => r.role).join(', ');

CHECKLIST.push(
  {
    page: 'theory-afrobeat.mdx',
    patch: null, // first patch on the page
    rules: [
      {
        id: 'afb-bell-unvaried',
        description: 'Rule 1: the 12-pulse bell timeline runs unchanged',
        check: ({ rows }) => {
          const bell = findLane(rows, /bell/i);
          if (!bell) return `no bell lane among ${roles(rows)}`;
          if (bell.steps !== 12) return `the bell runs ${bell.steps} steps; Rule 1 keeps the 12-pulse timeline`;
          return pct(bell, 'Mutation') === 0
            ? null
            : `the bell carries ${bell.cell.Mutation} mutation; Rule 1 says the timeline runs unchanged`;
        },
      },
      {
        id: 'afb-kick-sparse-fixed',
        description: 'Rule 3: the kick is three to four hits and does not vary',
        check: ({ rows }) => {
          const kick = findLane(rows, /kick/i);
          if (!kick) return `no kick lane among ${roles(rows)}`;
          if (kick.hits < 3 || kick.hits > 4) {
            return `the kick has ${kick.hits} hits; Rule 3 asks for three to four per cycle`;
          }
          return pct(kick, 'Mutation') === 0
            ? null
            : `the kick carries ${kick.cell.Mutation} mutation; Rule 3 says it does not vary bar to bar`;
        },
      },
      {
        id: 'afb-hat-continuous',
        description: 'Rule 4: the hi-hat is continuous',
        check: ({ rows }) => {
          const hat = findLane(rows, /hat/i);
          if (!hat) return `no hat lane among ${roles(rows)}`;
          // "Steady eighths or sixteenths": most of the grid struck. The
          // accent-contour half of Rule 4 is not checked here — a per-step
          // dynamic contour is not something the table reports.
          return hat.hits >= hat.steps * 0.75
            ? null
            : `the hat strikes ${hat.hits} of ${hat.steps}; Rule 4 asks for a continuous stream`;
        },
      },
      {
        id: 'afb-snare-whispers',
        description: 'Rule 5: the snare whispers rather than striking a backbeat',
        check: ({ rows }) => {
          const snare = findLane(rows, /snare|cross-stick/i);
          const bell = findLane(rows, /bell/i);
          if (!snare || !bell) return 'need a snare and a bell lane to check Rule 5';
          return num(snare, 'Velocity') < num(bell, 'Velocity')
            ? null
            : `the snare is at velocity ${snare.cell.Velocity} against the bell's ${bell.cell.Velocity}; ` +
                'Rule 5 keeps it conversational rather than struck';
        },
      },
      {
        id: 'afb-core-unvaried',
        description: 'Rule 7: the core trio repeats; variation is a section event',
        check: ({ rows }) => {
          const core = rows.filter((r) => /bell|kick|hat/i.test(r.role));
          const varying = core.filter((r) => pct(r, 'Mutation') > 0);
          return varying.length === 0
            ? null
            : `${varying.map((r) => `${r.role} ${r.cell.Mutation}`).join(', ')} carry mutation; ` +
                'Rule 7 keeps the core trio repeating and puts the drama in which voices are present';
        },
      },
    ],
  },
);

CHECKLIST.push(
  {
    page: 'theory-afro-cuban.mdx',
    patch: null,
    rules: [
      {
        id: 'ac-clave-unvaried',
        description: 'Rule 1: the clave is the referent and never varies',
        check: ({ rows }) => {
          const clave = findLane(rows, /clave/i);
          if (!clave) return `no clave lane among ${roles(rows)}`;
          return pct(clave, 'Mutation') === 0
            ? null
            : `the clave carries ${clave.cell.Mutation} mutation; Rule 1 makes it the referent, which never varies`;
        },
      },
      {
        id: 'ac-lilt-band',
        description: 'Rule 7: the feel is a lilt — swing between 0.2 and 0.3, ensemble-wide',
        check: ({ rows }) => {
          const out = rows.filter((r) => {
            const v = num(r, 'Swing');
            return !Number.isFinite(v) || v < 0.2 || v > 0.3;
          });
          return out.length === 0
            ? null
            : `${out.map((r) => `${r.role} ${r.cell.Swing}`).join(', ')} sit outside 0.2-0.3; ` +
                'Rule 7 puts Cuban subdivision between straight sixteenths and triplets, ensemble-wide';
        },
      },
    ],
  },
  {
    page: 'theory-electronic-breakbeat.mdx',
    patch: null,
    rules: [
      {
        id: 'ebb-polymeter-loop-length',
        description: 'Rule 3: polymeter comes from a loop length, not free phase',
        check: ({ rows }) => {
          const anchor = findLane(rows, /snare \(fixed\)|backbeat snare/i);
          if (!anchor) return 'no anchor lane to measure loop lengths against';
          // "Rigid in itself, shifting only against the frame": every lane's
          // step count is a whole number of steps, and at least one differs
          // from the anchor's frame.
          const differing = rows.filter((r) => r.steps !== anchor.steps);
          return differing.length > 0
            ? null
            : `every lane runs ${anchor.steps} steps; Rule 3 makes the breathing come from a loop of a different length`;
        },
      },
      {
        id: 'ebb-snare-backbeat-fixed',
        description: 'Rule 6: the snare backbeat is the fixed stratum',
        check: ({ rows }) => {
          const snare = findLane(rows, /snare \(fixed\)|backbeat snare/i);
          if (!snare) return `no backbeat snare among ${roles(rows)}`;
          const g = laneOnPulseGrid(snare, 16);
          if (g.length !== 2) return `the snare strikes ${g.length} times on the 16-pulse grid; Rule 6 holds a half-time backbeat`;
          return pct(snare, 'Mutation') === 0
            ? null
            : `the backbeat snare carries ${snare.cell.Mutation} mutation; Rule 6 makes it the listener's anchor`;
        },
      },
      {
        id: 'ebb-ghost-rolls',
        description: 'Rule 8: the ghost layer rolls continuously at low velocity',
        check: ({ rows }) => {
          const ghost = findLane(rows, /ghost/i);
          if (!ghost) return `no ghost lane among ${roles(rows)}`;
          const quietest = Math.min(...rows.map((r) => num(r, 'Velocity')));
          if (ghost.hits < ghost.steps * 0.6) {
            return `the ghost layer strikes ${ghost.hits} of ${ghost.steps}; Rule 8 asks for a near-continuous stream`;
          }
          return num(ghost, 'Velocity') === quietest
            ? null
            : `the ghost layer at velocity ${ghost.cell.Velocity} is not the quietest voice; Rule 8 keeps it low`;
        },
      },
    ],
  },
);

CHECKLIST.push(
  {
    page: 'theory-balkan.mdx',
    patch: null,
    rules: [
      {
        id: 'bal-shared-cell-grid',
        description: 'Rule 1: the cell sequence is fixed and shared by every voice',
        check: ({ rows }) => {
          const counts = [...new Set(rows.map((r) => r.steps))];
          return counts.length === 1
            ? null
            : `lanes run ${counts.join(', ')} steps; Rule 1 has every voice agree on the cell grid`;
        },
      },
      {
        id: 'bal-quick-pulse',
        description: 'Rule 3: a continuous quick-pulse stratum runs underneath',
        check: ({ rows }) => {
          const full = rows.filter((r) => r.hits === r.steps);
          return full.length > 0
            ? null
            : `no lane articulates every pulse (densest is ${Math.max(...rows.map((r) => r.hits))} of ` +
                `${rows[0]?.steps}); Rule 3 makes the unequal beats countable`;
        },
      },
      {
        id: 'bal-tupan-two-strokes',
        description: 'Rule 4: the tupan speaks a two-stroke grammar',
        check: ({ rows }) => {
          const tupan = rows.filter((r) => /tupan/i.test(r.role));
          if (tupan.length !== 2) return `${tupan.length} tupan lane(s); Rule 4 needs a low stroke and a stick stroke`;
          const [a, b] = tupan.map((r) => num(r, 'Velocity'));
          return a !== b
            ? null
            : `both tupan lanes sit at velocity ${a}; Rule 4 separates the low open stroke from the stick hand`;
        },
      },
      {
        id: 'bal-density-strata',
        description: 'Rule 5: density strata on the shared grid',
        check: ({ rows }) => {
          const densities = [...new Set(rows.map((r) => r.hits))];
          return densities.length >= 3
            ? null
            : `lanes carry ${densities.length} distinct hit count(s); Rule 5 layers sparse, mid and dense strata`;
        },
      },
      {
        id: 'bal-no-swing',
        description: 'Rule 6: no swing — the asymmetry is metric, not micro-timing',
        check: ({ rows }) => {
          const swung = rows.filter((r) => num(r, 'Swing') !== 0);
          return swung.length === 0
            ? null
            : `${swung.map((r) => `${r.role} ${r.cell.Swing}`).join(', ')} carry swing; Rule 6 keeps the asymmetry metric`;
        },
      },
    ],
  },
  {
    page: 'theory-brazilian.mdx',
    patch: null,
    rules: [
      {
        id: 'bra-caixa-continuous',
        description: 'Rule 3: the caixa never stops',
        check: ({ rows }) => {
          const caixa = findLane(rows, /caixa/i);
          if (!caixa) return `no caixa lane among ${roles(rows)}`;
          return caixa.hits >= caixa.steps * 0.75
            ? null
            : `the caixa strikes ${caixa.hits} of ${caixa.steps}; Rule 3 makes it the connective tissue`;
        },
      },
    ],
  },
  {
    page: 'theory-minimalism.mdx',
    patch: null,
    rules: [
      {
        id: 'min-rotations-differ',
        description: 'Rule 2: the phased pattern is not rotationally symmetric',
        check: ({ rows }) => {
          const voice = rows.find((r) => /phasing/i.test(r.role)) ?? rows[0];
          const onsets = laneOnsets(voice);
          if (!onsets) return 'the phasing voice is in timeline mode; its rotations cannot be derived here';
          // Symmetric when some non-zero rotation maps the onset set onto
          // itself: those phase positions sound identical and the process
          // wastes them.
          const set = new Set(onsets);
          const symmetric = [];
          for (let r = 1; r < voice.steps; r++) {
            if (onsets.every((o) => set.has((o + r) % voice.steps))) symmetric.push(r);
          }
          return symmetric.length === 0
            ? null
            : `${voice.role} maps onto itself at rotation ${symmetric.join(', ')}; Rule 2 wants rotations that differ`;
        },
      },
    ],
  },
);

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

// --- M007/S01: the rule triage --------------------------------------------

// Eleven theory pages state 92 numbered rules. Sixteen are asserted. Until a
// verdict existed for the rest, "this rule is not checked" and "this rule is
// not checkable" looked identical from the outside, which is what B12 is about.
//
// A rule is checkable when a lane table settles it — step counts, hit counts,
// rotations, velocities, mutation, swing, subdivision, or onset relationships
// between lanes. It is not checkable when deciding it needs a judgement no
// predicate makes.
//
// A not-checkable verdict is a claim and carries its reason. "Prose judgement"
// would not be one: without a specific reason the triage becomes a place rules
// go to be excused, which is the failure the divergence marker was designed
// against.
const RULE_TRIAGE = {
  'theory-afro-cuban.mdx': {
    1: { checkable: true, case: 'ac-clave-unvaried', why: 'the clave lane takes no mutation' },
    2: { checkable: false, why: 'asks which half of a part is "busy" relative to the clave sides, a judgement the table reports no cell for' },
    3: { checkable: true, case: 'ac-tumbao-onsets-rendered', why: 'asserted: theory-afro-cuban tumbao case' },
    4: { checkable: false, why: '"archetype part" is a repertoire fact about named patterns, not a property of steps, hits or rotation' },
    5: { checkable: true, case: 'ac-theory-one-free-voice', why: 'asserted: ac-theory-one-free-voice' },
    6: { checkable: false, why: '"almost no doubled strong accents" sets no threshold the table can settle' },
    7: { checkable: true, case: 'ac-lilt-band', why: 'Swing sits in 0.2-0.3 ensemble-wide' },
    8: { checkable: false, why: 'concerns section-scale behaviour over time — crossing and resolving — not a state any single patch has' },
  },
  'theory-afrobeat.mdx': {
    1: { checkable: true, case: 'afb-bell-unvaried', why: 'a 12-step bell lane carrying no mutation' },
    2: { checkable: false, why: '"do not poach" compares characteristic grid territories the table never names' },
    3: { checkable: true, case: 'afb-kick-sparse-fixed', why: 'the kick runs three to four hits and does not vary' },
    4: { checkable: true, case: 'afb-hat-continuous', why: 'the hat is continuous — a high hit count at its subdivision' },
    5: { checkable: true, case: 'afb-snare-whispers', why: 'the snare is sparse and quiet rather than a struck backbeat' },
    6: { checkable: false, why: 'enter and exit schedules measured in bars are arrangement, not a patch' },
    7: { checkable: true, case: 'afb-core-unvaried', why: 'ostinatos repeat with minimal mutation' },
    8: { checkable: false, why: '"dynamic range within any bar" is a performance contour; the table gives one velocity per lane, not a range within a bar' },
  },
  'theory-balkan.mdx': {
    1: { checkable: true, case: 'bal-shared-cell-grid', why: 'every lane shares the cell grid — the same step count' },
    2: { checkable: true, why: 'asserted: theory-balkan tupan cell-head case' },
    3: { checkable: true, case: 'bal-quick-pulse', why: 'some lane runs a continuous quick pulse — high hits at the finest subdivision' },
    4: { checkable: true, case: 'bal-tupan-two-strokes', why: 'the tupan speaks with two lanes, a low stroke and a high one' },
    5: { checkable: true, case: 'bal-density-strata', why: 'density strata sit on the shared grid, which is Rule 1 measured across all lanes' },
    6: { checkable: true, case: 'bal-no-swing', why: 'Swing is zero' },
    7: { checkable: false, why: 'names a Humanize bound, and this page\'s patch table carries no Humanize column' },
    8: { checkable: false, why: 'a claim about measured performance timing, not about anything the patch specifies' },
    9: { checkable: false, why: 'ornament placement before the long cell is not represented in a lane table' },
  },
  'theory-brazilian.mdx': {
    1: { checkable: false, why: 'the patch models the surdo pair as a single lane, so the dialogue\'s dynamic contrast between the two drums is not expressible in it' },
    2: { checkable: true, why: 'asserted: theory-brazilian surdo case' },
    3: { checkable: true, case: 'bra-caixa-continuous', why: 'the caixa never stops — continuous sixteenths' },
    4: { checkable: false, why: 'the call role is a performance function; no cell says which lane cues a break' },
    5: { checkable: false, why: '"around the beat, not against the meter" needs an interpretation of displacement the table does not supply' },
    6: { checkable: false, why: 'a micro-timing profile within each beat, which the lane table does not carry' },
    7: { checkable: false, why: 'states a relation between bossa and the wider system rather than a requirement on a patch' },
  },
  'theory-electronic-breakbeat.mdx': {
    1: { checkable: true, case: 'ebb-anchor-immutable', why: 'asserted: ebb-anchor-immutable' },
    2: { checkable: false, why: 'forbids a layer wandering between territories, which is a change over time; a static patch assigns each lane one position set and cannot wander. Coinciding with another layer is not wandering — a sixteenth hat stream crossing the backbeat keeps its own territory' },
    3: { checkable: true, case: 'ebb-polymeter-loop-length', why: 'polymeter comes from a loop length of 3, 5, 6 or 7 against the 4-unit frame' },
    4: { checkable: false, why: 'names swing percentages, and this page\'s patch table carries no Swing column' },
    5: { checkable: false, why: 'energy management across 8, 16 and 32-bar boundaries is arrangement, not a patch state' },
    6: { checkable: true, case: 'ebb-snare-backbeat-fixed', why: 'the snare holds the half-time backbeat' },
    7: { checkable: true, case: 'ebb-kick-avoids-snare', why: 'asserted: ebb-kick-avoids-snare' },
    8: { checkable: true, case: 'ebb-ghost-rolls', why: 'the ghost layer is near-continuous at low velocity' },
    9: { checkable: false, why: 'two-bar question and answer is a phrasing relation between bars, and a patch describes one cycle' },
  },
  'theory-funk-soul.mdx': {
    1: { checkable: true, why: 'some lane strikes the cycle downbeat' },
    2: { checkable: true, why: 'asserted: theory-funk-soul backbeat case' },
    3: { checkable: true, why: 'the kick runs three to five hits' },
    4: { checkable: true, why: 'ghost strokes sit at the low end of the dynamic range' },
    5: { checkable: true, why: 'one syncopator at a time — only one lane carries an intricate figure' },
    6: { checkable: false, why: 'a fixed micro-offset field in milliseconds, which the lane table does not report' },
    7: { checkable: false, why: 'fills every four or eight bars are arrangement across bars' },
    8: { checkable: false, why: 'states which of Rules 1-6 a neo-soul variant relaxes; it is a meta-rule, not a patch property' },
  },
  'theory-gamelan.mdx': {
    1: { checkable: true, why: 'the composite of the pair strikes almost every pulse' },
    2: { checkable: false, why: 'asks whether a part is playable and idiomatic for a human player, which no cell in the table reports' },
    3: { checkable: true, why: 'asserted: theory-gamelan polos case' },
    4: { checkable: true, why: 'asserted: gam-structural-overlap' },
    5: { checkable: false, why: 'naming an interlock style is a choice about repertoire, not a value any column holds' },
    6: { checkable: true, why: 'asserted: gam-pokok-layer' },
    7: { checkable: true, why: 'stroke rates halve as instruments deepen — step counts in powers of two' },
    8: { checkable: true, why: 'velocity increases with depth, the gong heaviest' },
    9: { checkable: false, why: 'needs each lane\'s register, and this page\'s patch table carries no Note column; role names imply it but naming is not measurement' },
    10: { checkable: false, why: 'irama trades tempo against density across performances; a patch fixes one tempo' },
  },
  'theory-indian-classical.mdx': {
    1: { checkable: true, why: 'the theka is the referent and runs in timeline mode' },
    2: { checkable: false, why: 'phrases resolving on sam is a property of improvisation, not of the printed lanes' },
    3: { checkable: false, why: 'khali is marked by the bayan falling silent in a region, which needs per-step absence the table does not give' },
    4: { checkable: true, why: 'layakari speeds are exact ratios — subdivisions in defined multiples' },
    5: { checkable: true, why: 'a cross-grouping lane carries a step count foreign to the tala' },
    6: { checkable: true, case: 'ind-tihai-worked', why: 'asserted: ind-tihai-worked' },
    7: { checkable: false, why: 'theme-and-variation by systematic permutation is a performance grammar' },
    8: { checkable: false, why: 'a loudness contour marking architecture, which one velocity per lane cannot express' },
  },
  'theory-jazz.mdx': {
    1: { checkable: true, why: 'the ride pattern is constant — no mutation' },
    2: { checkable: true, why: 'the foot hat is planted on 2 and 4' },
    3: { checkable: true, why: 'asserted: theory-jazz comping case' },
    4: { checkable: false, why: '"nothing repeats verbatim, nothing is unrelated" describes variation across choruses' },
    5: { checkable: false, why: 'the form is the meter — chorus structure over 12 or 32 bars' },
    6: { checkable: false, why: 'the swing ratio varies with tempo and between voices; the table holds one swing value' },
    7: { checkable: false, why: 'superimpositions resolving at a form boundary span bars a patch does not describe' },
    8: { checkable: true, why: 'ride and hat are the loudest constant voices, above snare and kick' },
  },
  'theory-minimalism.mdx': {
    1: { checkable: true, case: 'min-one-variable', why: 'asserted: min-one-variable' },
    2: { checkable: true, case: 'min-rotations-differ', why: 'the phased patterns are not rotationally symmetric' },
    3: { checkable: false, why: '"slow enough to inhabit, fast enough to remember" is a judgement about listening time' },
    4: { checkable: true, case: 'min-voices-flat', why: 'asserted: min-voices-flat' },
    5: { checkable: false, why: 'says an anchor is optional and changes the piece; it states no requirement to check' },
    6: { checkable: false, why: 'additive growth and contraction happen across repetitions, not within one patch' },
    7: { checkable: false, why: 'tempo-ratio counterpoint needs simultaneous tempi, which a single patch does not carry' },
    8: { checkable: true, case: 'min-deterministic', why: 'asserted: min-deterministic' },
  },
  'theory-sub-saharan-africa.mdx': {
    1: { checkable: true, why: 'the timeline varies in nothing — no mutation, no drift' },
    2: { checkable: false, why: 'a fixed phase relationship is a property of playing over time; every rotation in a static table is fixed by construction, so the rule cannot fail here' },
    3: { checkable: true, why: 'asserted: theory-sub-saharan-africa support-drum case' },
    4: { checkable: false, why: 'whether the texture supports both a ternary and a binary hearing is an interpretive claim about the composite' },
    5: { checkable: true, why: 'the variation budget rises from timeline to support to lead' },
    6: { checkable: false, why: 'lead phrases resolving at cycle boundaries span one to four cycles' },
    7: { checkable: false, why: 'needs each lane\'s register, and this page\'s patch table carries no Note column' },
    8: { checkable: false, why: 'a measured non-isochronous subdivision profile, which the lane table does not encode' },
    9: { checkable: false, why: 'call-and-response is a structural relation between players over time' },

  },
};

// A rule that is never registered is indistinguishable from a rule that passes.
// That nearly shipped inside this milestone: the first theory-afrobeat block
// appended to CHECKLIST *after* the loop that registers a test per rule, so all
// five cases were silently skipped and the suite went from 26 tests to 26, green.
//
// So the triage names the case for every rule it marks checkable, and this
// asserts the naming resolves in both directions: no triage entry names a case
// that does not exist, and no checklist rule is missing from the triage. The
// count of checkable rules still awaiting a case is printed, and M007/S02's
// close requires it to be zero.
test('M007/S02: every named case exists, and every case is named', async () => {
  const registered = new Set();
  for (const entry of CHECKLIST) for (const r of entry.rules) registered.add(`${entry.page}::${r.id}`);

  const danglingNames = [];
  const awaiting = [];
  for (const [page, rules] of Object.entries(RULE_TRIAGE)) {
    for (const [n, v] of Object.entries(rules)) {
      if (!v.checkable) continue;
      if (!v.case) {
        // 'asserted: <name>' marks a rule covered by one of the nine
        // hand-written per-page tests above rather than by the checklist.
        if (!v.why?.startsWith('asserted')) awaiting.push(`${page} rule ${n}`);
        continue;
      }
      if (!registered.has(`${page}::${v.case}`)) danglingNames.push(`${page} rule ${n} names ${v.case}, which is registered nowhere`);
    }
  }

  const unnamed = [];
  for (const key of registered) {
    const [page, id] = key.split('::');
    // Only theory pages carry numbered rules, so only they have triage entries.
    // The checklist also covers three chapter patches, which M004/S05 added and
    // which no rule triage governs.
    if (!RULE_TRIAGE[page]) continue;
    const named = Object.values(RULE_TRIAGE[page]).some((v) => v.case === id);
    if (!named) unnamed.push(`${page} registers ${id}, which no triage entry names`);
  }

  assert.deepEqual(danglingNames, [], danglingNames.join('\n  '));
  assert.deepEqual(unnamed, [], unnamed.join('\n  '));
  console.log(`  M007/S02: ${awaiting.length} checkable rule(s) still awaiting a case`);
});

test('M007/S01: every numbered rule on every theory page carries a verdict', async () => {
  const files = (await readdir(DOCS)).filter((f) => f.startsWith('theory-') && f.endsWith('.mdx'));
  const missing = [];
  const unreasoned = [];
  for (const f of files) {
    const src = await readFile(join(DOCS, f), 'utf8');
    const i = src.indexOf('## The Rules');
    if (i === -1) continue; // a page with no numbered rules needs no verdicts
    const section = src.slice(i, src.indexOf('\n## ', i + 5));
    // Read the rules off the page, never a hard-coded count: a page that gains
    // a rule must fail this case, which is the second definition-of-done item.
    const numbers = [...section.matchAll(/^(\d+)\. \*\*/gm)].map((m) => Number(m[1]));
    const page = RULE_TRIAGE[f];
    for (const n of numbers) {
      const verdict = page?.[n];
      if (!verdict) missing.push(`${f} rule ${n}`);
      else if (!verdict.checkable && !verdict.why?.trim()) unreasoned.push(`${f} rule ${n}`);
    }
  }
  assert.deepEqual(
    missing,
    [],
    `${missing.length} numbered rule(s) carry no triage verdict:\n  ${missing.join('\n  ')}`,
  );
  assert.deepEqual(
    unreasoned,
    [],
    `${unreasoned.length} rule(s) are marked not-checkable with no reason:\n  ${unreasoned.join('\n  ')}\n` +
      'A not-checkable verdict is a claim. Say what judgement it needs that no cell in the table reports.',
  );
});
