// Citation-integrity cases for the M002 milestone of the theory-audit ledger
// (docs/plans/theory-audit/ledger.md).
//
// M002/S01 creates this file with the F17 case. M002/S06 (F23) extends it with
// the reference-tier assertions — every bibliography entry carrying a declared
// tier, and every inline citation on a named-theory claim resolving to a
// Tier-A source — and wires it into scripts/check-doc-conformance.sh. Until
// then it runs under the `site-unit` token alone: site/package.json runs
// `node --test tests/**/*.test.mjs`, which picks this file up with no wiring.
//
// F17 is not a tier problem. Ref [2] named a real journal, a real volume, a
// real author and a working URL, and attached a title that does not exist —
// the article at that URL is Goldberg's Hristov/Bulgarian-meter paper. Every
// guard the guide had was blind to it: a link checker sees the URL, a tier
// check sees the venue, and the provenance check sees the anchor. None sees
// the pairing. The tree-wide case below is the one that would catch a repeat,
// because it forbids the fabricated string everywhere rather than in the one
// entry we happen to be looking at.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile, readdir } from 'node:fs/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

import { registerClaimTests } from './helpers/prose-claims.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const DOCS = join(HERE, '..', 'src', 'content', 'docs');

const loadSource = (file) => readFile(join(DOCS, file), 'utf8');

// The title the guide printed, which exists in no MTO issue and nowhere a web
// search reaches. Shared by both cases so they cannot drift apart.
const FABRICATED_TITLE = 'Resultant Patterns in Phase-Shifted Rhythmic Structures';

const CLAIMS = [
  {
    id: 'S01-F17',
    file: 'appendix-references.mdx',
    rule:
      'audit §5 on ref [2]. Verified against the publisher 2026-09-01: MTO is ' +
      'current through Vol 32 No 2 (June 2026), so 31(2) is not a forward ' +
      'reference and its URL resolves. The issue table of contents lists ' +
      "Goldberg's article as the Dobri Hristov / Bulgarian-meter paper. Real " +
      'article, fabricated title — corrected in place',
    forbidden: [FABRICATED_TITLE],
    present: ['Music Theory as an Instrument of Nationalism', 'Dobri Hristov'],
    presentRegex: [/mto\.25\.31\.2\.goldberg\.pdf/],
  },
  {
    id: 'S02-F18',
    file: '03-afro-cuban.mdx',
    rule:
      'ledger F18. The clave matrix is Ch 3\'s central theoretical claim and was ' +
      'cited to a YouTube video (ref-10) while Peñalosa (2009) sat unused in ' +
      'Further Reading. The gap sentence carries a second, different claim: that ' +
      'E(k,n) can only generate gaps of two consecutive values. That is Toussaint ' +
      '(2005, ref-1), not Peñalosa — citing Peñalosa for it would replace a bad ' +
      'citation with a wrong one',
    // forbiddenRegex, not forbidden: the guide cites a numbered reference for a
    // claim as <sup>[N](…)</sup> and lists one bibliographically as a plain
    // link. Only the superscript form is a named-theory claim, and prose
    // normalisation does not reliably see markup.
    forbiddenRegex: [/<sup>\[10\]\(\/appendix-references\/#ref-10\)<\/sup>/],
    // #ref-1\) cannot match #ref-10) — the paren must follow the 1.
    presentRegex: [/#fr-penalosa-2009/, /#ref-1\)/],
  },
  {
    id: 'S02-F18-theory',
    file: 'theory-afro-cuban.mdx',
    rule:
      'ledger F18. The companion page cited the same YouTube video (ref-10) for ' +
      'the same clave-matrix claim, on a sentence that already carried Peñalosa ' +
      '— the real source sitting next to the video. The Sources "See also refs ' +
      '[10]-[13]" listing is a bibliographic pointer, not a named-theory claim, ' +
      'and deliberately survives',
    forbiddenRegex: [/<sup>\[10\]\(\/appendix-references\/#ref-10\)<\/sup>/],
    // Standing guard, not a lock this slice establishes: the page already
    // carried four fr-penalosa-2009 citations. It bites if a later edit strips
    // Peñalosa out, which would leave the claim unsourced rather than
    // mis-sourced.
    presentRegex: [/#fr-penalosa-2009/],
  },
  {
    id: 'S03-F19',
    file: '04-afrobeat.mdx',
    rule:
      "ledger F19. Ch 4's opening cited a YouTube video (ref-14) while Allen & " +
      'Veal (2013) sat unused in Further Reading — and already sat in the same ' +
      'sentence, added by M001/S06. The phrase-architecture claim was uncited ' +
      'and takes Veal (2000), credited by the companion page for ensemble ' +
      'arranging and form. The lane-behaviour sentence is a claim about Poly, ' +
      'not about Afrobeat, so it takes no source at all: ref-17 was an Afro ' +
      'House production guide, a different genre, and re-citing it to Veal ' +
      'would swap a wrong-tier citation for a wrong-claim one',
    forbiddenRegex: [
      /<sup>\[14\]\(\/appendix-references\/#ref-14\)<\/sup>/,
      /<sup>\[17\]\(\/appendix-references\/#ref-17\)<\/sup>/,
    ],
    // #fr-veal-2000 is absent from the chapter today, so that arm is a real
    // lock this slice establishes. #fr-allen-veal-2013 is already present —
    // M001/S06 put it there — so it is a standing guard against a later edit
    // stripping it out, not something this slice creates.
    presentRegex: [/#fr-allen-veal-2013/, /#fr-veal-2000/],
  },
  {
    id: 'S04-F20',
    file: '06-indian-classical.mdx',
    rule:
      "ledger F20. Ch 6's tala and layakari claims were cited to a commercial " +
      'blog, a school textbook PDF and a konnakol video, while Clayton (2000) ' +
      'and Kippen (1988) sat unused in Further Reading — the chapter cited none ' +
      'of the three scholarly sources even once. The theka claim was uncited ' +
      'altogether and takes Kippen, whose subject is theka elaboration. ref-25 ' +
      'was off-topic as well as low-tier: konnakol is Carnatic vocal percussion, ' +
      'not rhythmic augmentation ratios',
    // Generalised from the single-reference form M002/S02 and S03 used, which
    // cannot see a multi-reference block like
    // <sup>[21](…), [22](…)</sup>. Matching any <sup> containing a ref-21..25
    // link catches both, and still cannot match the Sources "See also" listing,
    // which is a plain link with no <sup> around it.
    forbiddenRegex: [/<sup>[^<]*#ref-2[1-5][^<]*<\/sup>/],
    // Both arms are unsatisfied in the chapter today, so each is a real lock
    // this task establishes rather than a standing guard.
    presentRegex: [/#fr-clayton-2000/, /#fr-kippen-1988/],
  },
];

registerClaimTests({ test, assert, claims: CLAIMS, loadSource });

test(`S01-F17-tree: the fabricated ref-2 title appears in no doc`, async () => {
  const entries = await readdir(DOCS, { withFileTypes: true });
  const offenders = [];
  for (const entry of entries) {
    if (!entry.isFile() || !entry.name.endsWith('.mdx')) continue;
    const text = await readFile(join(DOCS, entry.name), 'utf8');
    if (text.includes(FABRICATED_TITLE)) offenders.push(entry.name);
  }
  assert.deepEqual(
    offenders,
    [],
    `fabricated citation title reappeared in: ${offenders.join(', ')}. ` +
      'Authority: MTO 31(2) carries no article of this title (publisher table ' +
      'of contents, checked 2026-09-01).',
  );
});
