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
// findings to the same FINDINGS array. F08 occupies two entries because the
// same superlative appears in Ch 2 and in the euclidean-reference appendix. Every case is built on the shared
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
  parseGrouping,
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
    id: 'S04-F04',
    file: '07-balkan.mdx',
    rule:
      'Ch 7 Balkan must frame the Bjorklund/aksak correspondence as a mathematical relationship — ' +
      'citing ref-46 (Bjorklund, neutron-beam timing) and ref-1 (Toussaint) — never as a historical ' +
      'convergence in which the tradition and the algorithm independently arrived at the same solution. ' +
      'Ledger F04.',
    // The pre-correction phrasing said the two traditions "independently
    // arrived at the same solution" — a historical-convergence claim the
    // sources do not support. That phrasing must not reappear.
    forbidden: [
      'independently arrived at the same solution',
    ],
    // Any prose that calls the correspondence a "historical convergence"
    // reverses the correction.
    forbiddenRegex: [/historical(?:ly)?\s+converg/i],
    present: [
      'The correspondence is mathematical, not historical',
      'ref-46',
      'ref-1',
    ],
  },
  {
    id: 'S04-F05',
    file: '07-balkan.mdx',
    rule:
      "Ch 7 Balkan's Daichovo/Kopanitsa section must name 2+2+3+2+2 as kopanitsa's primary five-cell " +
      'form, cite E(5,11) as its canonical Bjorklund spelling, and demote E(4,11) to a four-accent ' +
      'variant rendering rather than the canonical kopanitsa grouping. Per D012 the E(4,11) diagram and ' +
      'the Kopanitsa 11/8 PolyPatch table stay unchanged. Ledger F05.',
    // The pre-correction prose presented E(4,11) as *the* kopanitsa grouping.
    // Reject any residual phrasing that calls E(4,11) the canonical or the
    // primary kopanitsa spelling.
    forbiddenRegex: [
      /E\(4,\s*11\)[^\n]{0,120}(?:is|as)\s+(?:the\s+)?(?:canonical|primary)\s+kopanitsa/i,
    ],
    // Semantic markers only. The Euclidean spellings themselves (E(5,11),
    // 2+2+3+2+2, and E(4,11)=3+3+3+2) are re-derived from bjorklund.ts in
    // the 'S04-F05 arithmetic' case below rather than transcribed here —
    // mirroring the S02-F54 pairing of a FINDINGS row with an arithmetic
    // case, per the S04 must-have that forbids transcribed spellings.
    present: [
      'primary form',
      'five-cell',
      // The demotion language — E(4,11) is a variant, not the canonical.
      'variant rendering',
    ],
  },
  {
    id: 'S04-F06',
    file: '07-balkan.mdx',
    rule:
      'Ch 7 Balkan must state plainly that performed aksak long beats are not exactly 1.5x short beats, ' +
      'and cite fr-goldberg-2015 inline for the field measurements that show systematic deviation from ' +
      'the 3:2 ratio. Per D013 theory-balkan.mdx Rule 8 stays unchanged; the caveat lives in Ch 7. ' +
      'Ledger F06.',
    // Pre-correction phrasing asserted the long-to-short ratio *is* exactly
    // 1.5× (or exactly 3:2). Reject any assertive re-appearance. The corrective
    // sentence uses "not exactly 1.5×" and "look like exactly 1.5×"; both are
    // guarded because neither is preceded by an assertive is/are/equals.
    forbiddenRegex: [
      /(?:are|is|equals?)\s+exactly\s+1\.5/i,
      /(?:are|is|equals?|ratio\s+(?:is|of))\s+exactly\s+(?:3:2|three[-\s]to[-\s]two)/i,
    ],
    present: [
      'not exactly 1.5',
      'short beats',
      'fr-goldberg-2015',
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
  {
    id: 'S05-F07',
    file: '02-sub-saharan-africa.mdx',
    rule:
      'Ch 2 must not assert an unsourced multi-century depth for Ewe ensemble practice. Ethnomusicological ' +
      'fieldwork documenting Ewe drumming dates from the mid-twentieth century; depth before that is poorly ' +
      'attested (audit 2.1.2, §5.9). The claim is therefore bounded by the documentary record and carried by ' +
      'two Tier-A sources: Jones (1959, ref-6), the two-volume study built largely on Ewe material, and Locke ' +
      '(1982, fr-locke-1982), the Ethnomusicology 26(2) analysis of Southern Ewe dance drumming. Ledger F07.',
    // The pre-correction sentence asserted the organising logic had held "for
    // centuries" as bare fact. Any unhedged multi-century assertion reverses
    // the reframe; Ch 2 has no legitimate use of these constructions.
    forbiddenRegex: [
      /\bfor centuries\b/i,
      /\bfor hundreds of years\b/i,
    ],
    // An earlier pass hedged to "understood within the tradition as
    // considerably older than that record". That clause was dropped rather
    // than cited: no source meeting this guide's bar could be found for Ewe
    // musical time depth, and a hedge is still a claim. What remains is only
    // what the documentary record supports, and both citations are pinned so
    // the sentence cannot drift back to a bare assertion.
    present: [
      'documented in the ethnomusicological literature since the mid-twentieth century',
      'Studies in African Music',
      'ref-6',
      'fr-locke-1982',
    ],
  },
  {
    id: 'S05-F08',
    file: '02-sub-saharan-africa.mdx',
    rule:
      'Toussaint (2005, ref-1) calls E(7,12) the most commonly used timeline in sub-Saharan Africa — a ' +
      'descriptive frequency claim, not an evaluative ranking (audit 2.1.1). Ch 2 must carry the descriptive ' +
      'phrasing and cite Toussaint himself; the pre-correction text named Toussaint while footnoting ref-4, ' +
      'a Wikipedia article. Ledger F08.',
    forbiddenRegex: [/single most important/i],
    present: [
      'Toussaint describes as the most commonly used',
      // The reframe is only durable if the descriptive/evaluative distinction
      // is stated, not merely enacted by word choice.
      'not a ranking of importance',
      'ref-1',
    ],
  },
  {
    // Same claim as S05-F08, second site. The audit cited only Ch 2 because
    // that is where it read the sentence, but the appendix repeats it verbatim
    // below the generated E(12) table; fixing one and not the other would
    // leave the overclaim live for the next re-read.
    id: 'S05-F08-appendix',
    file: 'appendix-euclidean-reference.mdx',
    rule:
      'The E(7,12) prose under the 12-step table repeats Ch 2\'s superlative and must carry the same ' +
      'descriptive Toussaint framing. Ledger F08, second site.',
    forbiddenRegex: [/single most important/i],
    present: [
      'Toussaint describes as the most commonly used',
    ],
  },

  {
    id: 'S06-F09',
    file: '04-afrobeat.mdx',
    rule:
      "Ch 4 must not assign Afrobeat's rhythmic vocabulary to Tony Allen alone. The attribution is " +
      'contested: Allen co-attributed it throughout his own account, and Fela\'s horn arranging shaped ' +
      'the rhythmic feel (audit 2.6.1, §5.11). The corrected sentence credits the partnership and cites ' +
      'Allen & Veal (2013, fr-allen-veal-2013), the autobiography in which Allen states the ' +
      'collaboration. Ledger F09.',
    forbidden: ['who created its rhythmic vocabulary'],
    present: [
      'came out of that partnership rather than from either alone',
      'co-attributed the result throughout his own account of it',
    ],
    presentRegex: [/#fr-allen-veal-2013/],
  },
  {
    id: 'S06-F10',
    file: '04-afrobeat.mdx',
    rule:
      "Ch 4 must not present Allen's micro-timing as a measured fact. No timing study of his recordings " +
      'has been published, so "precise but not quantised" is received characterisation rather than ' +
      'measurement (audit 2.6.3). The Humanize bullet keeps the musical guidance and marks the claim as ' +
      'description. Ledger F10.',
    forbidden: ["timing was precise but not quantised"],
    present: [
      'is usually described as precise but unquantised',
      'that is a characterisation rather than a measurement',
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

test('theory-audit-claims covers at least the S02, S03, S04, S05, and S06 findings', () => {
  for (const req of [
    'S02-F01',
    'S02-F12',
    'S02-F54',
    'S03-F02',
    'S03-F03',
    'S04-F04',
    'S04-F05',
    'S04-F06',
    'S05-F07',
    'S05-F08',
    'S05-F08-appendix',
    'S06-F09',
    'S06-F10',
  ]) {
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

// Build a boolean onset pattern of length n whose consecutive onset gaps
// equal `groups` (wrapping from the last onset back to the first). The gap
// sequence returned by gapSequence() over the resulting pattern equals
// `groups` exactly, so a caller can feed a prose grouping "a+b+c+..." into
// findEuclideanRotation to test whether that grouping is any rotation of
// bjorklund(n, groups.length). Sum of `groups` must equal n.
function patternFromGrouping(groups, n) {
  const pat = new Array(n).fill(false);
  let idx = 0;
  for (const g of groups) {
    pat[idx] = true;
    idx += g;
  }
  return pat;
}

// F05 arithmetic case: re-derive kopanitsa's E(5,11) primary form and its
// demoted E(4,11) variant from bjorklund.ts and check them against the
// spellings actually printed in the '## Daichovo and Kopanitsa' section, so
// a mismatch names the derived gap sequence next to the printed one rather
// than leaving the reader with a bare count. Extracts the section directly
// so a paragraph edit that moves the pattern shows up here. Follows the
// S02-F54 precedent — the S04 must-have forbids transcribing the F05
// spellings, so the FINDINGS.present list above only checks semantic
// framing; the numeric authority lives here.
registerCase('S04-F05-arith');
test('S04-F05 arithmetic: kopanitsa spellings re-derive from bjorklund(11,5) and bjorklund(11,4)', async () => {
  const src = await docSource('07-balkan.mdx');
  const section = extractSection(src, '## Daichovo and Kopanitsa');
  assert.ok(
    section,
    "07-balkan.mdx [S04-F05]: '## Daichovo and Kopanitsa' section not found. " +
      'Authority: ledger F05 requires the kopanitsa demotion prose to live in this section.',
  );

  // The prose's five-cell primary form. Verify (a) it sums to 11, (b) it has
  // five cells, and (c) it is genuinely a rotation of bjorklund(11, 5) —
  // not just a plausible-looking additive grouping.
  const primaryMatch = section.match(/primary form is\s+(\d+(?:\+\d+)+)/);
  assert.ok(
    primaryMatch,
    "07-balkan.mdx [S04-F05]: could not locate 'primary form is X+Y+...' phrase " +
      'in the Kopanitsa section. Authority: ledger F05 requires the five-cell primary ' +
      'form to be named in prose so this arithmetic case can re-derive it against bjorklund.ts.',
  );
  const primaryGroups = parseGrouping(primaryMatch[1]);
  const primarySum = primaryGroups.reduce((a, b) => a + b, 0);
  assert.equal(
    primarySum,
    11,
    `07-balkan.mdx [S04-F05]: primary-form grouping "${primaryMatch[1]}" sums to ${primarySum}, ` +
      'not 11. Authority: kopanitsa is 11/8; a primary form that does not sum to 11 is a typo.',
  );
  assert.equal(
    primaryGroups.length,
    5,
    `07-balkan.mdx [S04-F05]: primary-form grouping "${primaryMatch[1]}" has ` +
      `${primaryGroups.length} cells, not 5. Authority: ledger F05 requires the ` +
      'five-cell (five-accent) form as primary; a four-cell form reintroduces the ' +
      'pre-correction E(4,11) contradiction.',
  );

  const primaryPattern = patternFromGrouping(primaryGroups, 11);
  const canonical5 = bjorklund(11, 5);
  const canonical5Gaps = gapSequence(canonical5);
  const fiveRotation = findEuclideanRotation(primaryPattern, 5, 11);
  assert.notEqual(
    fiveRotation,
    null,
    `07-balkan.mdx [S04-F05]: primary-form grouping "${primaryMatch[1]}" is not any ` +
      `rotation of bjorklund(11, 5) (canonical gaps ${canonical5Gaps.join('+')}). ` +
      'Authority: site/src/audio/bjorklund.ts.',
  );

  // The prose must name E(5,11) as the canonical Bjorklund spelling of that
  // five-cell family. Assert it appears inside the section — if the label
  // migrates out of the demotion paragraph, the reader loses the numeric
  // justification for the demotion.
  assert.ok(
    /E\(5,\s*11\)/.test(section),
    "07-balkan.mdx [S04-F05]: '## Daichovo and Kopanitsa' section does not name " +
      'E(5,11) as the canonical Bjorklund spelling of the five-cell family. ' +
      `Authority: bjorklund(11, 5) has 5 onsets across 11 steps (gaps ` +
      `${canonical5Gaps.join('+')}), and the prose grouping "${primaryMatch[1]}" ` +
      'is a rotation of that pattern.',
  );

  // The demoted E(4,11) row. The prose prints its grouping inline; verify
  // that spelling is bjorklund(11, 4) at rotation 0, not a transcribed
  // shape that could silently drift from the algorithm.
  const fourMatch = section.match(/E\(4,\s*11\)\s+gives\s+`(\d+(?:\+\d+)+)`/);
  assert.ok(
    fourMatch,
    "07-balkan.mdx [S04-F05]: could not locate the 'E(4,11) gives `X+Y+...`' " +
      'arithmetic line in the Kopanitsa section. Authority: ledger F05 requires ' +
      'the E(4,11) demotion to print its grouping so this case can re-derive it ' +
      'against bjorklund.ts.',
  );
  const fourGroups = parseGrouping(fourMatch[1]);
  const fourPattern = patternFromGrouping(fourGroups, 11);
  const canonical4 = bjorklund(11, 4);
  const canonical4Gaps = gapSequence(canonical4);
  const fourRotation = findEuclideanRotation(fourPattern, 4, 11);
  assert.equal(
    fourRotation,
    0,
    `07-balkan.mdx [S04-F05]: E(4,11) printed grouping "${fourMatch[1]}" is not ` +
      `bjorklund(11, 4) at rotation 0 (canonical gaps ${canonical4Gaps.join('+')}). ` +
      'Authority: site/src/audio/bjorklund.ts under the rotation-0-canonical convention ' +
      'stated in appendix-euclidean-reference.mdx.',
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
