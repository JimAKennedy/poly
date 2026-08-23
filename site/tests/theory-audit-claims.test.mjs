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

test('theory-audit-claims covers at least the S02 findings (F01, F12, F54)', () => {
  const ids = new Set(FINDINGS.map((f) => f.id));
  for (const req of ['S02-F01', 'S02-F12', 'S02-F54']) {
    assert.ok(ids.has(req), `theory-audit-claims dropped required S02 finding: ${req}`);
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
