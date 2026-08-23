// Shared prose-claim host for the M001 theory-audit remediation ledger
// (docs/audits/M001-theory-audit-remediation-plan.md).
//
// R003 requires every corrected claim to be locked by a named case that fails
// if the claim reverses. Per D010 those cases collect here — one file, one
// FINDINGS array — because neither of the two files R003 originally named is a
// general prose-claim host: prose-pattern-claims.test.mjs is a front-matter
// fixture verifier (patternClaims arithmetic against bjorklund), and
// prose-conformance-claims.test.mjs hard-asserts FINDINGS.length === 11 and
// explicitly declares Euclidean spellings out of scope.
//
// S02 creates this file with the F01, F12, and F54 cases. S03-S06 append their
// findings to the same FINDINGS array. Every case is built on the shared
// assertion helpers in ./helpers/prose-claims.mjs so a red run names the file,
// the finding id, which side moved (forbidden reappeared vs corrective went
// missing), and the authority the claim must agree with.
//
// F54 additionally re-derives the E(3,16) row's onset positions from
// site/src/audio/bjorklund.ts rather than transcribing the printed cells, so a
// mismatch points at the offending row rather than at a bare count.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

import { assertClaim } from './helpers/prose-claims.mjs';
import { bjorklund } from '../src/audio/bjorklund.ts';
import {
  parsePattern,
  gapSequence,
  findEuclideanRotation,
} from '../src/lib/euclidean-claims.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const DOCS = join(HERE, '..', 'src', 'content', 'docs');

// Every finding this host locks. S03-S06 append to this array.
//
// Claim shape (see helpers/prose-claims.mjs):
//   id / file / rule / forbidden / forbiddenRegex / present / presentRegex
const FINDINGS = [
  {
    id: 'S02-F01',
    file: '06-indian-classical.mdx',
    rule:
      "bjorklund(7, 3) is the rachenitsa distribution 2+2+3; only rotation 3 yields the 3+2+2 grouping " +
      'the chapter attributes to rupak. The pre-correction illustration named E(4,7), whose gap sequence ' +
      "is 2+2+2+1 — the wrong distribution entirely (site/src/audio/bjorklund.ts is the authority).",
    // The old illustration named E(4,7) as rupak's 3+2+2 spelling. That token
    // must not reappear as an illustration of rupak. Match it as a Euclidean
    // signature so unrelated occurrences (there are none in this chapter) do
    // not false-positive.
    forbiddenRegex: [/E\(4,\s*7\)/],
    present: [
      'E(3,7) at rotation 3',
      '3+2+2 grouping',
      'Rotation 0 of the same E(3,7) family',
      '2+2+3',
    ],
    // The illustrating EuclideanDiagram must be steps=7 hits=3 rotation=3.
    presentRegex: [/<EuclideanDiagram\s+steps=\{7\}\s+hits=\{3\}\s+rotation=\{3\}\s*\/>/],
  },
  {
    id: 'S02-F12',
    file: '06-indian-classical.mdx',
    rule:
      'theory-indian-classical: thekas are fixed named bol sequences, not Bjorklund distributions. E(7,16) ' +
      'can only be presented as an analogy for the weight of a slow theka, never as its grounding.',
    // The pre-correction phrase presented E(7,16) as the structure a tabla
    // player would outline during a slow theka — a grounding claim, not an
    // analogy. The corrective framing keeps the diagram but marks it as
    // analogue only.
    forbidden: [
      'the kind of structure a tabla player might outline during a slow theka',
    ],
    present: [
      'not itself a tintal theka',
      'thekas are fixed, named bol sequences',
      'rough analogue',
    ],
  },
  {
    id: 'S03-F02',
    file: '08-minimalism.mdx',
    rule:
      "Ch 8's 'Reich and Phase Shifting' section must pair each of Reich's three named works with its own " +
      'technique: Piano Phase (1967) with continuous/gradual phase-shifting, Drumming (1971) with construction ' +
      'and reduction (not phase shifting), and Clapping Music (1972) with discrete one-position jumps. ' +
      'Ledger F02.',
    // The pre-correction opening sentence began by conflating the three
    // techniques into one gradual-phase-shifting narrative. That opening must
    // not reappear.
    forbidden: [
      'Steve Reich starts two identical rhythmic patterns in unison',
    ],
    // Any prose that attributes phase-shifting or gradual/continuous shifting
    // to Drumming reverses the correction. Match a Drumming citation followed
    // shortly by phase-shifting language on the same line.
    forbiddenRegex: [
      /\*Drumming\*[^\n]{0,120}(phase[- ]?shift|gradual (?:phase|shift)|continuous (?:phase|shift))/i,
    ],
    present: [
      'three distinct techniques',
      'Piano Phase',
      'Violin Phase',
      'Drumming',
      'construction and reduction',
      'Clapping Music',
      'one position every twelve bars',
    ],
  },
  {
    id: 'S02-F54',
    file: 'appendix-euclidean-reference.mdx',
    rule:
      "appendix-euclidean-reference.mdx must state the phase convention (rotate-right, rotation-0 canonical) " +
      "that makes its E(3,16) row 'x . . . . x . . . . x . . . . .' with grouping 5+5+6 verifiable against " +
      'bjorklund.ts. Closes issue #91 by explaining that 0/5/10 and 0/6/11 are the same distribution at ' +
      'different rotations. Decision D008.',
    present: [
      'Phase Convention',
      'rotate-right',
      'rotation 0',
      'canonical Bjorklund',
      // The worked E(3,16) example that answers #91.
      'onsets at steps 0, 5, 10',
    ],
  },
];

const srcCache = new Map();
async function docSource(file) {
  if (!srcCache.has(file)) {
    srcCache.set(file, await readFile(join(DOCS, file), 'utf8'));
  }
  return srcCache.get(file);
}

// Every case registered in this host — whole-file (FINDINGS-driven) and
// section-scoped (standalone test blocks like S03-F03). The coverage test
// asserts against this set so deleting any required case's registration block
// turns the suite red by name, not just its whole-file assertion body.
const REGISTERED_CASE_IDS = new Set();
function registerCase(id) {
  REGISTERED_CASE_IDS.add(id);
}

for (const f of FINDINGS) {
  registerCase(f.id);
}

test('theory-audit-claims covers at least the S02 and S03 findings', () => {
  for (const req of ['S02-F01', 'S02-F12', 'S02-F54', 'S03-F02', 'S03-F03']) {
    assert.ok(
      REGISTERED_CASE_IDS.has(req),
      `theory-audit-claims dropped required finding: ${req}. ` +
        'Every M001 corrected claim must have a registered case in this host ' +
        '(either a FINDINGS row or a section-scoped test block registered via registerCase).',
    );
  }
});

for (const f of FINDINGS) {
  test(`${f.id} (${f.file}): forbidden absent, correction present`, async () => {
    const src = await docSource(f.file);
    assertClaim(assert, f, src);
  });
}

// F54 arithmetic case: re-derive the E(3,16) row from bjorklund.ts and check
// it against the appendix's printed cells, so a mismatch names the derived
// onset indices and gap sequence next to the printed spelling rather than
// leaving the reader with a bare count. Extracts the appendix's E(3,16) row
// directly so a table edit that moves the pattern shows up here.
test('S02-F54 arithmetic: E(3,16) appendix row matches bjorklund(16, 3) at rotation 0', async () => {
  const src = await docSource('appendix-euclidean-reference.mdx');
  const rowRe = /^\|\s*E\(3,\s*16\)\s*\|\s*`([^`]+)`\s*\|\s*([^|]+?)\s*\|/m;
  const m = src.match(rowRe);
  assert.ok(
    m,
    'appendix-euclidean-reference.mdx [S02-F54]: no E(3,16) row found. ' +
      'The Phase Convention preamble is meaningless without the row it explains.',
  );
  const [, patternCell, groupingCell] = m;

  const printed = parsePattern(patternCell);
  const printedGaps = gapSequence(printed);
  const printedOnsets = [];
  for (let i = 0; i < printed.length; i++) if (printed[i]) printedOnsets.push(i);

  const canonical = bjorklund(16, 3);
  const canonicalOnsets = [];
  for (let i = 0; i < canonical.length; i++) if (canonical[i]) canonicalOnsets.push(i);
  const canonicalGaps = gapSequence(canonical);

  const rotation = findEuclideanRotation(printed, 3, 16);
  assert.equal(
    rotation,
    0,
    `appendix-euclidean-reference.mdx [S02-F54]: E(3,16) printed row ` +
      `"${patternCell}" (onsets ${JSON.stringify(printedOnsets)}, gaps ${printedGaps.join('+')}) ` +
      `is not bjorklund(16, 3) at rotation 0 ` +
      `(canonical onsets ${JSON.stringify(canonicalOnsets)}, gaps ${canonicalGaps.join('+')}). ` +
      'Authority: site/src/audio/bjorklund.ts under the rotate-right, rotation-0-canonical convention ' +
      'stated in the appendix preamble.',
  );

  assert.equal(
    groupingCell.trim(),
    '5+5+6',
    `appendix-euclidean-reference.mdx [S02-F54]: E(3,16) grouping cell "${groupingCell.trim()}" ` +
      `disagrees with derived gap sequence ${printedGaps.join('+')}. Authority: bjorklund(16, 3) at rotation 0.`,
  );
});

// F03 is asserted section-scoped rather than whole-file: the Drift → Piano
// Phase re-point only makes sense when the "## Drift as Phase Engine" section
// itself names the work Drift models. A whole-file check would false-pass on
// a Piano Phase mention that lives back in the Reich section while the Drift
// section still speaks of Reich's process without naming its source, which is
// exactly the pre-correction shape ledger F03 records. Extracting the section
// makes the failure name the Drift section rather than the chapter at large.
function extractSection(src, heading) {
  const lines = src.split('\n');
  const start = lines.findIndex((l) => l.trim() === heading);
  if (start < 0) return null;
  let end = lines.length;
  for (let i = start + 1; i < lines.length; i++) {
    if (/^##\s/.test(lines[i])) {
      end = i;
      break;
    }
  }
  return lines.slice(start, end).join('\n');
}

// Register the section-scoped case's id alongside its test() call so the
// coverage assertion above catches deletion of this block — removing the
// test() removes the registerCase() line with it.
registerCase('S03-F03');
test('S03-F03 (08-minimalism.mdx, section-scoped): Drift section cites Piano Phase, not an unnamed Reich process', async () => {
  const src = await docSource('08-minimalism.mdx');
  const section = extractSection(src, '## Drift as Phase Engine');
  assert.ok(
    section,
    "08-minimalism.mdx [S03-F03]: '## Drift as Phase Engine' section not found. " +
      'Authority: ledger F03 requires the Drift section to name the Reich work it models (Piano Phase).',
  );
  assertClaim(
    assert,
    {
      id: 'S03-F03',
      file: '08-minimalism.mdx (## Drift as Phase Engine)',
      rule:
        'Ch 8 Drift section must name Piano Phase (1967) as the work Drift models — Drift produces ' +
        'continuous phase shift, which is Piano Phase, not the construction/reduction of Drumming. ' +
        'Ledger F03.',
      // The pre-correction Drift section referred to "the gradual process
      // that Reich described" without naming which work — the exact shape
      // that lets a reader assume Drumming (since Drumming was the only
      // Reich work named upstream before S03-F02 fixed that too).
      forbidden: [
        'the gradual process that Reich described',
      ],
      present: [
        'Piano Phase',
        '(1967)',
        'ref-35',
        'not the additive construction of Drumming',
      ],
    },
    section,
  );
});
