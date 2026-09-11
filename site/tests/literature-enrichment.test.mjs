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
import { readFile } from 'node:fs/promises';
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
