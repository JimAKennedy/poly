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
const THEORY = join(HERE, '..', 'src', 'content', 'theory');

// first-release M002/S02: the deep dives are deferred, not retired, so their
// citation claims still hold -- they just resolve from the new root.
const docRoot = (file) => (file.startsWith('theory-') ? THEORY : DOCS);

const loadSource = (file) => readFile(join(docRoot(file), file), 'utf8');

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
    id: 'REF9-OLURANTI',
    file: 'appendix-references.mdx',
    rule:
      'ref [9] was the F17 defect a second time, and worse. It printed the title ' +
      '"Polyrhythmic Structures in West African Drumming" against a real author ' +
      'and a real institution, and no such dissertation exists. Verified against ' +
      'the PDF itself: Oluranti (2012) is "Polyrhythm as an Integral Feature of ' +
      'African Pianism", a University of Pittsburgh PhD analysing piano works by ' +
      'Euba, Ligeti and Uzoigwe. The subject is African PIANISM, not drumming, so ' +
      'the old entry misdescribed what the source is about as well as what it is ' +
      'called. The URL had also rotted: D-Scholarship migrated platforms and the ' +
      'old /11866/4/*.pdf path 404s',
    forbidden: ['Polyrhythmic Structures in West African Drumming'],
    present: [
      'Polyrhythm as an Integral Feature of African Pianism',
      'Oluranti, S. A. (2012)',
    ],
    // The dead path must not come back. The live record is on the migrated
    // platform; d-scholarship 403s every automated fetch, so this asserts the
    // shape of the citation rather than that the server answers.
    forbiddenRegex: [/11866\/4\/DISSERTATION_-_FULL_Oluranti\.pdf/],
  },
  {
    id: 'REF6-JONES',
    file: 'appendix-references.mdx',
    rule:
      'VR03. ref [6] cited Jones (1959) *Studies in African Music* and linked to ' +
      'a Cambridge review OF that book, so a reader reached two pages of someone ' +
      "else's opinion of a work they still could not read. The work itself is on " +
      'archive.org as a borrowable scan (identifier studiesinafrican0000amjo, ' +
      'A.M. Jones, Oxford University Press, collections inlibrary and ' +
      'printdisabled — controlled digital lending). The review survives only as ' +
      'a labelled secondary. The definition of done\'s other arm, citing the ' +
      'book by ISBN, is unavailable: the archive.org record carries no ISBN and ' +
      'a 1959 imprint predates the ISBN system',
    present: ['Studies in African Music', 'Jones, A. M. (1959)'],
    presentRegex: [/archive\.org\/details\/studiesinafrican0000amjo/],
    // The present arm only proves the scan URL is somewhere in the file. This
    // proves it is on ref-6's own line, and ahead of the review: the match
    // succeeds only when the Cambridge URL is reachable from id="ref-6"
    // without passing an archive.org link first. Both the old single-link form
    // and a form that puts the opinion before the work are therefore red.
    forbiddenRegex: [/id="ref-6"(?:(?!archive\.org)[^\n])*cambridge\.org/],
  },
  {
    id: 'REF26-AKSAK',
    file: 'appendix-references.mdx',
    rule:
      'VR01. ref [26] was a Fiveable course-marketing study guide and returned ' +
      '404. Under the obtainability principle that is a replacement, not a ' +
      'repair: the replacement is Bonini Baraldi, Bigand & Pozzo (2015), ' +
      '"Measuring Aksak Rhythm and Synchronization in Transylvanian Village ' +
      'Music by Using Motion Capture", Empirical Musicology Review 10(4), ' +
      '265-291, the lead article of that journal\'s open-access aksak special ' +
      'issue. Author list, title, volume, issue, pages and DOI come from ' +
      'Crossref. It is cited by DOI rather than by a host path so it cannot rot ' +
      'the way its predecessor did, and it is replaced in place so refs 27-43 ' +
      'need no renumbering. It must not be the Goldberg paper from the same ' +
      'issue, which the Balkan chapter already cites as fr-goldberg-2015',
    // Tree-wide on the bibliography: the dead study-guide host must not return
    // under any entry number, not merely under 26.
    forbiddenRegex: [/fiveable\.me/],
    present: [
      'Measuring Aksak Rhythm and Synchronization',
      'Empirical Musicology Review',
    ],
    presentRegex: [/10\.18061\/emr\.v10i4\.4891/],
  },
  {
    id: 'REF22-CLAYTON',
    file: 'appendix-references.mdx',
    rule:
      'VR02. ref [22] was the National Institute of Open Schooling\'s Hindustani ' +
      'Music (242) teaching text -- course material, the same class as the ' +
      'Fiveable study guide removed from ref [26]. It was replaced on editorial ' +
      'grounds rather than liveness grounds: whether a teaching PDF loads does ' +
      'not make it a citable source, so the browser check the worklist had ' +
      'queued was never needed. The replacement is Clayton (2020), Music Theory ' +
      'Online 26(1), DOI 10.30535/mto.26.1.2 -- peer-reviewed, platinum open ' +
      'access, freely readable without login, and on North Indian rupak tal, ' +
      'which is the slot ref [22] occupied. Verified against the publisher ' +
      'page, the served HTML and Crossref, with the DOI resolving to the ' +
      'article. Clayton is already the chapter\'s primary authority as ' +
      'fr-clayton-2000',
    // Tree-wide, like the fiveable.me arm: the teaching text must not return
    // under any entry number.
    forbiddenRegex: [/nios\.ac\.in/],
    present: [
      'Theory and Practice of Long-form Non-isochronous Meters',
      'Music Theory Online',
    ],
    presentRegex: [/10\.30535\/mto\.26\.1\.2/],
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
  {
    id: 'S04-F20-theory',
    file: 'theory-indian-classical.mdx',
    rule:
      'ledger F20. The companion page cited the same commercial blog and school ' +
      'textbook PDF for its tala definition, on a sentence already carrying ' +
      'Clayton, and cited a journal PDF and a pubpub article for the layakari ' +
      'ratios. The Sources "See also" listing is a bibliographic pointer, not a ' +
      'named-theory claim, and deliberately survives',
    // This page puts two references inside one <sup>, which the single-reference
    // pattern M002/S02 and S03 used cannot see. Hence the generalised form.
    forbiddenRegex: [/<sup>[^<]*#ref-2[1-5][^<]*<\/sup>/],
    // Standing guard, not a lock this task establishes: the page already cites
    // Clayton. It bites if a later edit strips Clayton out, leaving the claims
    // unsourced rather than mis-sourced.
    presentRegex: [/#fr-clayton-2000/],
  },
  {
    id: 'S05-F21',
    file: '07-balkan.mdx',
    rule:
      'ledger F21. Ch 7 cited an educational aggregator (ref-26, Fiveable) for ' +
      'the definition of aksak and a theory blog (ref-27, Chromatone) for ' +
      'svatbarska muzika, while Brăiloiu — who coined the term — and Rice, whose ' +
      'ethnography is the standard on Bulgarian practice, sat in Further Reading ' +
      'only. Goldberg (2015) stays where it is, cited at the long-beat timing ' +
      'claim, and is not the authority for either of these two',
    forbiddenRegex: [/<sup>[^<]*#ref-2[67][^<]*<\/sup>/],
    // Both arms are unsatisfied in the chapter today — it cites only
    // fr-goldberg-2015 — so each is a real lock this task establishes.
    presentRegex: [/#fr-brailoiu-1951/, /#fr-rice-1994/],
  },
  {
    id: 'S06-F22',
    file: '03-afro-cuban.mdx',
    rule:
      'ledger F22. Ch 3 attributes "the Spanish tinge" to Jelly Roll Morton. The ' +
      'attribution is accurate but was uncited, and no Lomax entry existed ' +
      'anywhere in the appendix — so this needed a new Further Reading entry as ' +
      'well as an inline citation. Lomax\'s Mister Jelly Roll (1950) is the book ' +
      'built from the Library of Congress interviews in which Morton uses the ' +
      'phrase, so it is a primary source',
    // No forbidden arm: nothing wrong is being removed here. An uncited claim is
    // being sourced, so this case can only ever fail on the present side.
    presentRegex: [/#fr-lomax-1950/],
  },
];

registerClaimTests({ test, assert, claims: CLAIMS, loadSource });

// M002/S06 task 1. Every bibliography entry declares a tier, so the Tier-A rule
// in `S06-claims-are-tier-a` has something to resolve against and a new
// reference cannot arrive untiered. Tier values follow the audit's definitions
// (docs/audits/poly_theory_audit.md section 4): A peer-reviewed scholarship or
// a primary source, B secondary but legitimate, C hobbyist media.
export const TIER_RE = /<span id="((?:ref|fr)-[A-Za-z0-9.-]+)"([^>]*)>/g;

export async function readTierMap(bibPath) {
  const text = await readFile(bibPath, 'utf8');
  const tiers = new Map();
  for (const m of text.matchAll(TIER_RE)) {
    const tier = /data-tier="([^"]*)"/.exec(m[2] ?? '');
    tiers.set(m[1], tier ? tier[1] : null);
  }
  return tiers;
}

test('S06-tiers-declared: every bibliography entry declares a valid tier', async () => {
  const tiers = await readTierMap(join(DOCS, 'appendix-references.mdx'));
  assert.ok(tiers.size > 0, 'no bibliography entries found — the span pattern has drifted');
  const bad = [...tiers.entries()]
    .filter(([, t]) => !['A', 'B', 'C'].includes(t))
    .map(([id, t]) => `${id} (${t === null ? 'no data-tier' : `data-tier="${t}"`})`);
  assert.deepEqual(
    bad,
    [],
    `${bad.length} of ${tiers.size} entries lack a valid tier:\n  ${bad.join('\n  ')}\n` +
      'Add data-tier="A"|"B"|"C" to the entry span. Authority: audit section 4 — ' +
      'A peer-reviewed or primary, B secondary but legitimate, C hobbyist media.',
  );
});

// M002/S06 task 2. The rule F23 asks for: a named-theory claim may not cite a
// source below Tier A. The guide's citation grammar is what makes that
// mechanisable without judging prose — a <sup> marks a claim, a plain link
// marks bibliography — so the Sources "See also" listings that deliberately
// point at low-tier refs are out of scope structurally rather than one by one.
//
// Escape hatch: `citation-tier-ok: <reason>` on the citing line or the line
// immediately above it, written as an MDX comment. Deliberately narrow — there
// is no file-level or global form, because several exceptions for several
// different reasons must not collapse into one blanket waiver. The live
// suppression count is printed so a rising number is visible rather than silent.
const SUP_RE = /<sup>[^<]*<\/sup>/g;
const REF_IN_SUP_RE = /#((?:ref|fr)-[A-Za-z0-9.-]+)/g;
const HATCH = 'citation-tier-ok';

test('S06-claims-are-tier-a: every claim citation resolves to a Tier-A source', async () => {
  const tiers = await readTierMap(join(DOCS, 'appendix-references.mdx'));
  const files = (await readdir(DOCS)).filter((f) => f.endsWith('.mdx') && f !== 'appendix-references.mdx');

  const violations = [];
  let suppressed = 0;

  for (const file of files.sort()) {
    const lines = (await readFile(join(DOCS, file), 'utf8')).split('\n');
    lines.forEach((line, i) => {
      const exempt = line.includes(HATCH) || (i > 0 && lines[i - 1].includes(HATCH));
      for (const block of line.match(SUP_RE) ?? []) {
        for (const [, anchor] of block.matchAll(REF_IN_SUP_RE)) {
          const tier = tiers.get(anchor) ?? 'undeclared';
          if (tier === 'A') continue;
          if (exempt) { suppressed += 1; continue; }
          violations.push(`${file}:${i + 1} cites ${anchor} (tier ${tier})`);
        }
      }
    });
  }

  console.log(`    S06-claims-are-tier-a: ${suppressed} live suppression(s) via ${HATCH}`);
  assert.deepEqual(
    violations,
    [],
    `${violations.length} claim citation(s) resolve below Tier A:\n  ${violations.join('\n  ')}\n` +
      `Cite a Tier-A source, or add {/* ${HATCH}: <reason> */} on the line above ` +
      'saying why this source is acceptable here or which row owns replacing it.',
  );
});

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

// VR02. ref [22] cites a chapter of the National Institute of Open Schooling's
// Hindustani Music (242) theory book. NIOS is India's national open-schooling
// board and the source is legitimate; what is NOT established is that the link
// is dead. Nothing on nios.ac.in answers from this network — not the PDF, not a
// sibling chapter a search engine lists as live, not the homepage — while DNS
// resolves to a single A record. That is the D-Scholarship class the
// REF9-OLURANTI case describes, not link rot.
//
// This is deliberately not an assertion that the URL works; no test run here
// can settle that. It asserts an implication: if the bibliography still cites
// the host, the browser worklist must name the entry. "We could not check it"
// and "we checked it and it was fine" are indistinguishable states once the
// session ends, and only one of them is true here — so an entry nobody could
// verify must carry an owner and a place in a queue.
//
// It is a standalone case rather than a CLAIMS entry because it reads two
// files. registerClaimTests loads exactly one source per claim, and a claim
// pointed at the worklist alone could only assert the obligation
// unconditionally — going red the day the entry is legitimately resolved.
test('ref-22 stays queued while its URL is unverified', async () => {
  const bibliography = await loadSource('appendix-references.mdx');
  if (!bibliography.includes('nios.ac.in')) return; // condition false, obligation lifts

  const worklistPath = join(
    HERE,
    '..',
    '..',
    'docs',
    'plans',
    'verifiable-references',
    'browser-worklist.md',
  );
  let worklist;
  try {
    worklist = await readFile(worklistPath, 'utf8');
  } catch {
    assert.fail(
      `appendix-references.mdx still cites nios.ac.in, but ${worklistPath} ` +
        'does not exist. An entry no automated fetch can verify must be queued ' +
        "for a human, or it is silently indistinguishable from one that's fine.",
    );
  }
  assert.ok(
    worklist.includes('ref-22'),
    'appendix-references.mdx still cites nios.ac.in, but the browser worklist ' +
      'does not name ref-22. Either queue the entry or resolve it — leaving it ' +
      'cited and unqueued records a verification that never happened.',
  );
});
