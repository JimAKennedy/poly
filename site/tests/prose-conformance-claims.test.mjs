// Prose-conformance regression guard for M071 S03.
//
// S02's Euclidean corrections were locked with arithmetic guards
// (prose-pattern-claims / chapter-euclidean-guardrail). S03's corrections are
// *semantic* prose contradictions — a chapter/guide sentence asserting something
// its paired theory deep-dive page (the M069-repaired authority) or the chapter's
// own text contradicts. Those can't be re-derived arithmetically, so they need a
// different lock: for each of the eleven recon-confirmed findings, assert that the
// exact forbidden phrase is ABSENT and (where applicable) the corrective
// phrase/value is PRESENT. A reintroduced contradiction fails `node --test`.
//
// Each finding cites the theory-page rule (or internal statement) it now agrees
// with, so a failure message names the file, the finding id, the offending
// forbidden phrase (reappeared) or corrective phrase (went missing), and the
// authority the claim must agree with.
//
// Scope: prose contradictions only. Euclidean pattern/onset/gap/EuclideanDiagram
// spellings (S02) and Experiment/ListenFor idiom-break framing (S05) are out of
// scope and are NOT asserted here.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const CHAPTERS_DIR = join(HERE, '..', 'src', 'content', 'docs');

// Each finding:
//   id        - stable S03 finding identifier
//   file      - chapter/guide .mdx under src/content/docs
//   rule      - the theory-page rule (or internal statement) the claim agrees with
//   forbidden - literal phrases that MUST be absent (the reintroduced contradiction)
//   present   - literal phrases that MUST be present (the corrective phrase/value)
//   forbiddenRegex / presentRegex - regex variants for table-cell / structured checks
const FINDINGS = [
  {
    id: 'S03-F1',
    file: '01-foundations.mdx',
    rule: 'line 57 rotation-preserves-onsets + line 41 cinquillo = E(5,8): rotating E(7,12) cannot become the E(5,8) cinquillo',
    forbidden: ['becomes the Cuban cinquillo variant'],
    present: ['rotating it does not change the seven onsets'],
  },
  {
    id: 'S03-F2',
    file: '01-foundations.mdx',
    rule: 'reference [1] (appendix-references.mdx): Toussaint 2005',
    forbidden: ['In 2004, Godfried Toussaint'],
    present: ['In 2005, Godfried Toussaint'],
  },
  {
    id: 'S03-F3',
    file: '03-afro-cuban.mdx',
    rule: 'theory-afro-cuban Rule 1: the clave takes no ornamentation, no dynamics contour, no mutation',
    forbidden: ['participate in Poly’s macro modulation', "participate in Poly's macro modulation"],
    present: ['no ornamentation, no dynamics contour, and no mutation'],
  },
  {
    id: 'S03-F4',
    file: '05-gamelan.mdx',
    rule: 'theory-gamelan Rule 4: parts overlap at structural tones; a never-overlapping pair sounds mechanical',
    forbidden: ['their intersection is empty'],
    present: ['strike together at structural tones', 'sounds mechanical'],
  },
  {
    id: 'S03-F5',
    file: '06-indian-classical.mdx',
    rule: "theory-indian-classical Rule 4 + chapter lines 43/45: 1/16 against 1/4 is chaugun (4x), not tigun (3x)",
    forbidden: ['Tigun ornament'],
    present: ['Chaugun ornament'],
  },
  {
    id: 'S03-F6',
    file: '08-minimalism.mdx',
    rule: 'theory-minimalism Rules 6 & 8: additive process via deterministic Complexity/Density hit-count change; Mutation/probability have no role',
    forbidden: [
      'Poly’s Mutation parameter offers an analogous process',
      "Poly's Mutation parameter offers an analogous process",
    ],
    present: ['Complexity and Density macros offer an analogous process'],
  },
  {
    id: 'S03-F7',
    file: '09-electronic.mdx',
    rule: 'theory-electronic-breakbeat Rule 1: the four-on-the-floor anchor takes no density modulation below 1.0',
    forbidden: ['scales effective hit counts across all lanes'],
    present: ['scales effective hit counts across the syncopating lanes', 'without ever thinning the kick'],
  },
  {
    id: 'S03-F8',
    file: '11-funk-soul.mdx',
    rule: 'theory-funk-soul Rule 4 / What-Breaks-the-Idiom: an even carpet of uniform ghosts breaks the idiom; ghosts cluster toward the accents',
    forbidden: ['every sixteenth-note subdivision', 'almost every remaining sixteenth'],
    present: ['cluster toward the backbeat'],
  },
  {
    id: 'S03-F9',
    file: '12-jazz.mdx',
    rule: "theory-jazz Rule 1 + chapter lines 56/89: timekeeping ride lane has zero mutation",
    // The Elvin Jones "Ride (steady)" table row must carry 0% Mutation, not 10%.
    // Match within the single table line (. does not cross newlines).
    forbiddenRegex: [/Ride \(steady\).*\b10%\s*\|/],
    presentRegex: [/Ride \(steady\).*\b0%\s*\|/],
  },
  {
    id: 'S03-F10',
    file: '12-jazz.mdx',
    rule: "theory-jazz Rule 7 (5- and 7-groupings) + chapter's own 3/4/5/7 patch",
    forbidden: ['6-over-4'],
    present: ['7-over-4'],
  },
  {
    id: 'S03-F11',
    file: '13-drum-and-bass.mdx',
    rule: 'attribution spelling: Gregory Sylvester Coleman (Amen break drummer)',
    forbidden: ['Gregory Cylvester Coleman'],
    present: ['Gregory Sylvester Coleman'],
  },
];

const srcCache = new Map();
async function chapterSource(file) {
  if (!srcCache.has(file)) {
    srcCache.set(file, await readFile(join(CHAPTERS_DIR, file), 'utf8'));
  }
  return srcCache.get(file);
}

test('all eleven S03 findings are covered by this guard', () => {
  assert.equal(
    FINDINGS.length,
    11,
    `expected 11 recon-confirmed contradiction-class findings, guard declares ${FINDINGS.length}`,
  );
});

for (const f of FINDINGS) {
  test(`${f.id} (${f.file}): forbidden contradiction absent, correction present`, async () => {
    const src = await chapterSource(f.file);

    for (const phrase of f.forbidden ?? []) {
      assert.ok(
        !src.includes(phrase),
        `${f.file} [${f.id}]: forbidden phrase reappeared: "${phrase}". ` +
          `Prose must agree with authority: ${f.rule}.`,
      );
    }
    for (const re of f.forbiddenRegex ?? []) {
      assert.ok(
        !re.test(src),
        `${f.file} [${f.id}]: forbidden pattern reappeared: ${re}. ` +
          `Prose must agree with authority: ${f.rule}.`,
      );
    }
    for (const phrase of f.present ?? []) {
      assert.ok(
        src.includes(phrase),
        `${f.file} [${f.id}]: corrective phrase went missing: "${phrase}". ` +
          `Prose must agree with authority: ${f.rule}.`,
      );
    }
    for (const re of f.presentRegex ?? []) {
      assert.ok(
        re.test(src),
        `${f.file} [${f.id}]: corrective pattern went missing: ${re}. ` +
          `Prose must agree with authority: ${f.rule}.`,
      );
    }
  });
}
