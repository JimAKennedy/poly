// Idiom-break-framing regression guard for M071 S05.
//
// S05 swept the remaining idiom-breaking Experiment/ListenFor suggestions in the
// chapters and appendices so that each knowing break either (a) carries a
// D6/D027 :::note[Bending the idiom] Starlight aside naming the broken tradition
// rule and linking to that tradition's theory-*.mdx #what-breaks-the-idiom
// section (FRAME), or (b) is replaced when the claim is factually wrong rather
// than a knowing break (REPLACE).
//
// That framing is prose, not machine-derivable: a reintroduced unframed
// idiom-break or a reverted factual correction would silently pass the build.
// So, mirroring S03's prose-conformance-claims.test.mjs, this guard locks each
// edit's exact literal signature — for REPLACE, the forbidden phrase is ABSENT
// and the corrective wording/link is PRESENT; for FRAME, the :::note[Bending the
// idiom] aside title and the tradition's #what-breaks-the-idiom anchor link are
// PRESENT. A reintroduced unframed break fails `node --test` with a message that
// names the file, the finding id, and the phrase/link that reappeared or went
// missing.
//
// Scope: S05 idiom-break framing only. Euclidean pattern spellings (S02) and
// semantic prose contradictions (S03) are out of scope and NOT asserted here.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const CHAPTERS_DIR = join(HERE, '..', 'src', 'content', 'docs');

// Each finding:
//   id        - stable S05 finding identifier
//   kind      - 'REPLACE' | 'FRAME' (documentation only)
//   file      - chapter/appendix .mdx under src/content/docs
//   rule      - the broken tradition rule the edit frames or the fact it fixes
//   forbidden - literal phrases that MUST be absent (the reintroduced break)
//   present   - literal phrases that MUST be present (aside title / anchor link /
//               corrective wording)
const FINDINGS = [
  {
    id: 'S05-F1',
    kind: 'REPLACE',
    file: 'appendix-presets.mdx',
    rule: 'E(5,16) is the bossa nova cell; the son clave is non-Euclidean (theory-afro-cuban Rule 1), so "E(5,16) for the full son clave pattern" is factually wrong and was replaced with a Timeline-mode experiment',
    forbidden: ['for the full son clave pattern'],
    present: [
      'the son clave is non-Euclidean',
      '/03-afro-cuban/#son-clave-rumba-clave-and-their-euclidean-neighbour',
    ],
  },
  {
    id: 'S05-F2',
    kind: 'FRAME',
    file: 'appendix-presets.mdx',
    rule: 'theory-gamelan Rule 7: gamelan colotomy is self-contained and binary, so a West African E(7,12) bell over Balinese kotekan is knowing fusion, not gamelan practice',
    present: [':::note[Bending the idiom]', '/theory-gamelan/#what-breaks-the-idiom'],
  },
  {
    id: 'S05-F3',
    kind: 'FRAME',
    file: 'appendix-presets.mdx',
    rule: 'theory-balkan Rule 5: a foreign cycle length erases the dance, so an E(4,11) kopanitsa lane against the 7/8 aksak is a deliberate polymetric hybrid, not idiomatic aksak',
    present: [':::note[Bending the idiom]', '/theory-balkan/#what-breaks-the-idiom'],
  },
  {
    id: 'S05-F4',
    kind: 'FRAME',
    file: '03-afro-cuban.mdx',
    rule: 'theory-afro-cuban Rule 2: a crossed part contradicts the matrix direction, so timba clave crossings are a knowing break for tension, not the son idiom',
    present: [':::note[Bending the idiom]', '/theory-afro-cuban/#what-breaks-the-idiom'],
  },
  {
    id: 'S05-F5',
    kind: 'FRAME',
    file: '14-synthesis.mdx',
    rule: "the chapter's fusions knowingly bend the Afro-Cuban, gamelan, and Balkan idioms; one reciprocal aside links back to each tradition's #what-breaks-the-idiom section",
    present: [
      ':::note[Bending the idiom]',
      '/theory-afro-cuban/#what-breaks-the-idiom',
      '/theory-gamelan/#what-breaks-the-idiom',
      '/theory-balkan/#what-breaks-the-idiom',
    ],
  },
];

// Per-file count of :::note[Bending the idiom] asides the sweep introduced, so
// removing one aside while another survives still fails even though the title
// string is shared within a file.
const ASIDE_COUNTS = [
  { file: 'appendix-presets.mdx', count: 2 }, // gamelan (S05-F2) + balkan (S05-F3)
  { file: '03-afro-cuban.mdx', count: 1 }, // timba crossings (S05-F4)
  { file: '14-synthesis.mdx', count: 1 }, // reciprocal cross-links (S05-F5)
];

const ASIDE_TITLE = ':::note[Bending the idiom]';

const srcCache = new Map();
async function chapterSource(file) {
  if (!srcCache.has(file)) {
    srcCache.set(file, await readFile(join(CHAPTERS_DIR, file), 'utf8'));
  }
  return srcCache.get(file);
}

test('the guard covers every S05 idiom-break edit (1 REPLACE + 4 FRAME)', () => {
  assert.equal(
    FINDINGS.length,
    5,
    `expected 5 recon-confirmed idiom-break edits, guard declares ${FINDINGS.length}`,
  );
  assert.equal(
    FINDINGS.filter((f) => f.kind === 'REPLACE').length,
    1,
    'expected exactly 1 REPLACE edit (the factually-wrong son-clave bullet)',
  );
  assert.equal(
    FINDINGS.filter((f) => f.kind === 'FRAME').length,
    4,
    'expected exactly 4 FRAME edits (the "Bending the idiom" asides)',
  );
});

for (const f of FINDINGS) {
  test(`${f.id} ${f.kind} (${f.file}): unframed break absent, framing present`, async () => {
    const src = await chapterSource(f.file);

    for (const phrase of f.forbidden ?? []) {
      assert.ok(
        !src.includes(phrase),
        `${f.file} [${f.id}]: forbidden idiom-break phrase reappeared: "${phrase}". ` +
          `That break is factually wrong / unframed — rule: ${f.rule}.`,
      );
    }
    for (const phrase of f.present ?? []) {
      assert.ok(
        src.includes(phrase),
        `${f.file} [${f.id}]: required framing went missing: "${phrase}". ` +
          `Every idiom-break must carry the "Bending the idiom" aside + anchor — rule: ${f.rule}.`,
      );
    }
  });
}

for (const { file, count } of ASIDE_COUNTS) {
  test(`${file}: exactly ${count} "Bending the idiom" aside(s) present`, async () => {
    const src = await chapterSource(file);
    const found = src.split(ASIDE_TITLE).length - 1;
    assert.equal(
      found,
      count,
      `${file}: expected ${count} ${ASIDE_TITLE} aside(s), found ${found}. ` +
        `A removed aside leaves its idiom-break unframed.`,
    );
  });
}
