// Literature-enrichment cases for the M005 milestone of the theory-audit ledger
// (docs/plans/theory-audit/ledger.md).
//
// M005 differs from the milestones before it. M001–M004 corrected and locked
// material already in the tree, where every claim was checkable against a file.
// M005 imports external facts: bibliography entries for works whose existence,
// titles, publishers and subject matter cannot be established from this repo.
//
// These cases therefore check what this repo *can* check — that an entry exists,
// declares a tier, and is cited at the claim it was added for. They cannot check
// that the work exists or supports that claim. That gap is jk-standards#85, and
// it is why every source was verified online before planning, with the results
// recorded in docs/plans/theory-audit/M005-decisions.md.
//
// Wired into scripts/check-doc-conformance.sh and named in the REQUIRED array of
// doc-conformance-wiring.test.mjs. Both are necessary: nothing in CI runs
// `npm --prefix site test` (poly issue #272), so a host outside the runner is a
// lock that never runs on the remote.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile, readdir } from 'node:fs/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const DOCS = join(HERE, '..', 'src', 'content', 'docs');
const APPENDIX = join(DOCS, 'appendix-references.mdx');

// Entries M005 adds, with the tier each must declare. Every one was verified
// bibliographically and by published descriptions of its subject matter; none
// was read. See M005-decisions.md.
const ENTRIES = [
  { anchor: 'fr-charry-2000', tier: 'A', row: 'F44' },
  { anchor: 'fr-kubik-1999', tier: 'A', row: 'F45' },
  { anchor: 'fr-agawu-2003', tier: 'A', row: 'F46' },
  { anchor: 'fr-acosta-2003', tier: 'A', row: 'F47' },
  { anchor: 'fr-powers-1980', tier: 'A', row: 'F48' },
  { anchor: 'fr-silverman-2007', tier: 'A', row: 'F49' },
  { anchor: 'fr-born-hesmondhalgh-2000', tier: 'A', row: 'F51' },
  { anchor: 'fr-crook-2009', tier: 'A', row: 'F53' },
  { anchor: 'fr-peycheva-dimov-2002', tier: 'B', row: 'F49' },
];

test('M005/S01: the sub-Saharan sources are in the appendix with declared tiers', async () => {
  const src = await readFile(APPENDIX, 'utf8');
  const missing = [];
  const wrongTier = [];
  for (const e of ENTRIES) {
    const m = src.match(new RegExp(`id="${e.anchor}"\\s+data-tier="([A-Z])"`));
    if (!m) {
      missing.push(`${e.anchor} (ledger row ${e.row})`);
      continue;
    }
    if (m[1] !== e.tier) wrongTier.push(`${e.anchor}: tier ${m[1]}, expected ${e.tier}`);
  }
  assert.deepEqual(
    missing,
    [],
    `appendix-references.mdx is missing ${missing.length} entry/entries that M005/S01 adds:\n  ` +
      missing.join('\n  '),
  );
  assert.deepEqual(wrongTier, [], wrongTier.join('\n  '));
});

// Each citation is bound to a phrase from the passage it belongs to, not to the
// page as a whole. An unbounded match would pass on a page that cites the work
// somewhere else entirely, which is how a citation ends up attached to nothing.
const CITATIONS = [
  {
    row: 'F44',
    file: '02-sub-saharan-africa.mdx',
    anchor: 'fr-charry-2000',
    near: 'pedagogical simplification',
    why: "Charry is what corrects the same-cycle simplification, so he belongs in the note that admits it",
  },
  {
    row: 'F45',
    file: '02-sub-saharan-africa.mdx',
    anchor: 'fr-kubik-1999',
    // Bound to the body sentence, not the bare place-name: the frontmatter
    // description also says "West and Central African drumming", and matching
    // that first would have tested a window with no citation in it.
    near: 'it is the polymetric drumming of West and Central Africa',
    why: "Kubik traces retentions through the Western and Central Sudanic belt, which is the scope the chapter's opening claims",
  },
  {
    row: 'F46',
    file: '08-minimalism.mdx',
    anchor: 'fr-agawu-2003',
    near: 'West African and Indonesian musicians already knew',
    why: 'the F11 reframe is where the guide handles the African–minimalist connection',
  },
  {
    row: 'F52',
    file: 'theory-sub-saharan-africa.mdx',
    anchor: 'fr-arom-1991',
    near: 'Western transcription usually omits',
    why: "Arom's referent theory is the methodology behind the dance-beat claim; the Sources listing is not a citation at a claim",
  },
  {
    row: 'F47',
    file: '03-afro-cuban.mdx',
    anchor: 'fr-acosta-2003',
    near: 'the Spanish tinge',
    why: 'Acosta covers the Cuban presence in New Orleans behind the phrase Lomax records Morton using',
  },
  {
    row: 'F48',
    file: '06-indian-classical.mdx',
    anchor: 'fr-powers-1980',
    near: 'None of that is covered here',
    why: "Powers's New Grove survey is the wider frame this chapter narrows from; it is cited for Indian art-music theory generally, its coverage of the Carnatic tala system specifically being unconfirmed",
  },
  {
    // Passes on the day it is written: M002/S04 added this citation. The case
    // exists so a later edit cannot drop it, and its non-vacuity is the
    // deletion test in the evidence file, not this assertion passing.
    row: 'F48-kippen',
    file: '06-indian-classical.mdx',
    anchor: 'fr-kippen-1988',
    near: 'learns by rote',
    why: 'M002/S04 cited Kippen at the theka claim; the row says he is in Further Reading only, which is stale',
  },
  {
    row: 'F49',
    file: '07-balkan.mdx',
    anchor: 'fr-silverman-2007',
    near: 'svatbarska muzika',
    why: "Rice is cited there for the tempos; Silverman is the source for the tradition's own history and politics",
  },
  {
    row: 'F50',
    file: '08-minimalism.mdx',
    anchor: 'fr-scherzinger-2010',
    near: 'West African and Indonesian musicians already knew',
    why: 'the entry and the theory page already cite Scherzinger; the chapter making the claim did not',
  },
  {
    row: 'F51',
    file: '14-synthesis.mdx',
    anchor: 'fr-born-hesmondhalgh-2000',
    near: 'Cross-Pollination: Why It Works',
    why: "the volume is about musical borrowing and the representation of difference, which is this section's subject",
  },
  {
    row: 'F53',
    file: '10-brazilian.mdx',
    anchor: 'fr-crook-2009',
    near: 'Maracatu: Weight and Repetition',
    why: "Crook's Afro-Brazilian traditions chapter covers maracatu directly",
  },
  {
    row: 'B06',
    file: '09-electronic.mdx',
    anchor: 'fr-linn-attack-2020',
    near: 'reduces to one parameter: swing',
    why: "the marker says Brett's blog is not Linn speaking and names this interview as the primary source",
  },
  {
    row: 'B06-theory',
    file: 'theory-electronic-breakbeat.mdx',
    anchor: 'fr-linn-attack-2020',
    near: 'descended from hardware sequencers',
    why: 'same source and same defect as 09-electronic, which the marker states outright',
  },
  {
    row: 'B07',
    file: '13-drum-and-bass.mdx',
    anchor: 'fr-harrison-2025',
    near: 'widely described as the most sampled recording',
    why: "Harrison's chapter documents the break's history; the hedge stays, because he grounds the history rather than counting the samples",
  },
];

for (const c of CITATIONS) {
  test(`M005 ${c.row}: ${c.file} cites ${c.anchor} at its claim`, async () => {
    const src = await readFile(join(DOCS, c.file), 'utf8');
    const i = src.indexOf(c.near);
    assert.notEqual(i, -1, `${c.file}: the passage phrase ${JSON.stringify(c.near)} is gone, so this case can no longer bind to it`);
    // 600 characters is roughly a paragraph: wide enough for a citation at the
    // end of the sentence or the next, narrow enough that a mention elsewhere
    // on the page cannot satisfy it.
    const window = src.slice(i, i + 600);
    assert.ok(
      window.includes(`#${c.anchor}`),
      `${c.file} [${c.row}]: ${c.anchor} is not cited near ${JSON.stringify(c.near)} — ${c.why}`,
    );
  });
}

// Peycheva & Dimov is the one entry whose contents nobody here has been able to
// read: the work is in Bulgarian. Rather than let a tier-B rating stand in for a
// caveat nobody wrote, the entry says so in a fixed phrase, so
// `grep -rn "contents unverified" site/` enumerates every entry in that state.
test('M005/S04: the unread source says so in its own entry', async () => {
  const src = await readFile(APPENDIX, 'utf8');
  const i = src.indexOf('id="fr-peycheva-dimov-2002"');
  assert.notEqual(i, -1, 'fr-peycheva-dimov-2002 is missing from the appendix');
  const entry = src.slice(i, src.indexOf('</span>', i));
  assert.ok(
    entry.includes('contents unverified'),
    'the Peycheva & Dimov entry does not carry the phrase "contents unverified". Its bibliographic ' +
      'details were confirmed but the work is in Bulgarian and has not been read here; the entry has to ' +
      'say that, because no tier value can.',
  );
});

// The maracatu ensemble's parts, as F53 scopes them. Named as a set so dropping
// one fails by name rather than leaving the section quietly thinner than the row
// says it should be. What verification established is the instruments and their
// roles; a per-part account of each pattern is not something any source here
// supports, and the section does not attempt one.
const MARACATU_PARTS = ['caixa', 'alfaia', 'mineiro', 'agbê', 'gonguê'];

test('M005/S06: the maracatu section names the ensemble it describes', async () => {
  const src = await readFile(join(DOCS, '10-brazilian.mdx'), 'utf8');
  const i = src.indexOf('## Maracatu: Weight and Repetition');
  assert.notEqual(i, -1, '10-brazilian.mdx: the Maracatu section heading is gone');
  const section = src.slice(i, src.indexOf('\n## ', i + 5));
  const missing = MARACATU_PARTS.filter((part) => !section.toLowerCase().includes(part.toLowerCase()));
  assert.deepEqual(
    missing,
    [],
    `the maracatu section does not name ${missing.join(', ')} — F53 asks it to describe the ` +
      'ensemble rather than only its density and weight',
  );
});

// --- M006/S01: the citation-tier suppressions ------------------------------

// M002 shipped ten `citation-tier-ok` markers, each naming the M006 row that
// owns its upgrade. This asserts the rows this slice closes have none left.
//
// The upgrade pattern is M002's own: a named-theory claim is a <sup>[N]</sup>,
// a bibliographic listing is a plain link, so moving a claim onto a Tier-A
// Further Reading source means removing the superscript and adding an inline
// author-year link. Both halves are asserted, because removing the superscript
// alone would take the claim out of the tier check's scope rather than source
// it — an evasion that would look exactly like a fix.
// Keyed on the files this task clears, not on row ids in the marker text: the
// two ref-42 markers predate B17 and say "No M006 row owns ref-42 yet", so a
// row-id match silently misses exactly the pair the row was created for.
const FILES_CLEARED = [
  '01-foundations.mdx', // B01
  '03-afro-cuban.mdx', // B02
  '05-gamelan.mdx', // B03
  '08-minimalism.mdx', // B04 and B05
  '10-brazilian.mdx', // B17
  'appendix-euclidean-reference.mdx', // B17
];

test('M006/S01: no suppression remains in the files this slice clears', async () => {
  const live = [];
  for (const f of FILES_CLEARED) {
    const src = await readFile(join(DOCS, f), 'utf8');
    const n = [...src.matchAll(/citation-tier-ok:/g)].length;
    if (n) live.push(`${f} (${n})`);
  }
  assert.deepEqual(
    live,
    [],
    `${live.length} citation-tier-ok suppression(s) still name a row M006/S01 closes:\n  ` +
      live.join('\n  '),
  );
});

// --- M006/S02: no entry is cited by nothing --------------------------------

// B08 called these orphans "either dead weight or a source nobody checked".
// Measuring found that most were neither: seventeen of twenty-one sat inside
// range listings — "See also refs [21]–[25]" — which hyperlinked only their
// endpoints, so the interior entries were pointed at in prose and unreachable
// by anchor. M006/S02 expanded all ten ranges into explicit links rather than
// teaching this check to parse a prose convention that could drift.
//
// The one exception is an entry that says its own contents are unread. That
// phrase is fixed and in-band so the set stays greppable; see M005/S04.
test('M006/S02: every appendix entry is cited by a page, or says it is unread', async () => {
  const app = await readFile(APPENDIX, 'utf8');
  const entries = [...app.matchAll(/id="((?:ref|fr)-[A-Za-z0-9-]+)"/g)].map((m) => m[1]);

  const cited = new Set();
  for (const f of (await readdir(DOCS)).filter((f) => f.endsWith('.mdx'))) {
    if (f === 'appendix-references.mdx') continue;
    const src = await readFile(join(DOCS, f), 'utf8');
    for (const m of src.matchAll(/#((?:ref|fr)-[A-Za-z0-9-]+)/g)) cited.add(m[1]);
  }

  const unread = new Set(
    [...app.matchAll(/id="((?:ref|fr)-[A-Za-z0-9-]+)"[\s\S]*?<\/span>/g)]
      .filter((m) => m[0].includes('contents unverified'))
      .map((m) => m[1]),
  );

  const orphans = entries.filter((e) => !cited.has(e) && !unread.has(e));
  assert.deepEqual(
    orphans,
    [],
    `${orphans.length} appendix entr(ies) are cited by no page and do not declare their contents ` +
      `unverified:\n  ${orphans.join('\n  ')}\n` +
      'Either cite it where it genuinely supports a claim, list it in the relevant page\'s Sources ' +
      'section, or retire it with the reason recorded in the ledger row.',
  );
});

// --- M006/S03: no work is listed twice -------------------------------------

// ref-1 and fr-toussaint-2005 were the same 2005 BRIDGES paper, listed twice in
// one appendix at different weights. Nothing cited the duplicate, so they could
// not disagree — the defect was that nothing stopped a later citation picking
// the wrong one.
//
// Normalised rather than compared literally: the two entries phrased the same
// paper differently, which is precisely why it went unnoticed. An exact-string
// check would have found nothing and reported success.
const normaliseWork = (s) =>
  s
    .toLowerCase()
    .replace(/<[^>]*>/g, ' ')
    .replace(/[^a-z0-9]+/g, ' ')
    .trim();

test('M006/S03: no two appendix entries name the same work', async () => {
  const app = await readFile(APPENDIX, 'utf8');
  const seen = new Map();
  const clashes = [];
  // Per line, not per span: a numbered entry wraps only its bracketed number in
  // the span and carries its bibliography on the rest of the line, while a
  // Further Reading entry wraps the lot. A span-spanning regex reads across
  // neighbouring entries and invents collisions.
  for (const line of app.split('\n')) {
    const anchor = line.match(/id="((?:ref|fr)-[A-Za-z0-9-]+)"/)?.[1];
    if (!anchor) continue;
    const text = normaliseWork(line.replace(/id="[^"]*"/, ''));
    const year = text.match(/\b(19|20)\d{2}\b/)?.[0];
    if (!year) continue;
    // Title words, not the whole entry: publisher and URL differ between a DOI
    // and a PDF of the same paper, which is how the Toussaint duplicate hid.
    const words = text
      .split(' ')
      .filter((w) => w.length > 4 && !/^\d+$/.test(w) && !/^(19|20)\d{2}$/.test(w));
    if (words.length < 5) continue;
    const key = `${year}|${words.slice(0, 6).join(' ')}`;
    if (seen.has(key)) clashes.push(`${seen.get(key)} and ${anchor}: ${key}`);
    else seen.set(key, anchor);
  }
  assert.deepEqual(
    clashes,
    [],
    `${clashes.length} pair(s) of appendix entries name the same work:\n  ${clashes.join('\n  ')}\n` +
      'Keep the cited one and retire the other; two entries for one work let a later citation pick the wrong.',
  );
});
