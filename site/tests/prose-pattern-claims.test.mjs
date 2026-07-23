// Verify chapter-level patternClaims front-matter blocks against the
// reference bjorklund/euclidean-claims library.
//
// Each chapter with a specific pattern-string assertion in prose can declare
// a fixture in front-matter:
//
//   patternClaims:
//     - label: "Reich Clapping Music (1972)"
//       pattern: "x x x . x x . x . x x ."
//       isEuclidean: false                 # or { k, n, rotation }
//       gapSequence: [1, 1, 2, 1, 2, 2, 1, 2]   # optional cross-check
//       reference: "line N prose ..."      # optional but recommended
//
// The verifier catches D1-shape errors (claim isEuclidean where pattern isn't)
// and D3-shape errors (typo in pattern string). Locks in M047's D1/D3
// corrections so a future edit can't silently un-fix them.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile, readdir } from 'node:fs/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import yaml from 'js-yaml';

import {
  parsePattern,
  onsetCount,
  gapSequence,
  findEuclideanRotation,
  findAnyEuclideanFit,
} from '../src/lib/euclidean-claims.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const CHAPTERS_DIR = join(HERE, '..', 'src', 'content', 'docs');

function parseFrontmatter(src) {
  if (!src.startsWith('---\n')) return null;
  const end = src.indexOf('\n---', 4);
  if (end < 0) return null;
  const yamlSrc = src.slice(4, end);
  try {
    return yaml.load(yamlSrc);
  } catch (e) {
    throw new Error(`YAML parse error: ${e.message}`);
  }
}

async function loadClaims() {
  const files = (await readdir(CHAPTERS_DIR)).filter((f) => f.endsWith('.mdx'));
  const out = [];
  for (const file of files) {
    const src = await readFile(join(CHAPTERS_DIR, file), 'utf8');
    const fm = parseFrontmatter(src);
    if (!fm || !Array.isArray(fm.patternClaims)) continue;
    for (let i = 0; i < fm.patternClaims.length; i++) {
      out.push({ file, index: i, claim: fm.patternClaims[i] });
    }
  }
  return out;
}

test('at least the expected chapters carry patternClaims fixtures', async () => {
  const claims = await loadClaims();
  const files = new Set(claims.map((c) => c.file));
  assert.ok(files.has('08-minimalism.mdx'), 'expected Reich claim in 08-minimalism');
  assert.ok(files.has('03-afro-cuban.mdx'), 'expected clave claims in 03-afro-cuban');
  assert.ok(claims.length >= 5, `expected >= 5 claims across chapters, got ${claims.length}`);
});

test('every claim: pattern parses cleanly', async () => {
  const claims = await loadClaims();
  const failures = [];
  for (const c of claims) {
    try {
      parsePattern(c.claim.pattern);
    } catch (e) {
      failures.push(`${c.file}#patternClaims[${c.index}] "${c.claim.label}": ${e.message}`);
    }
  }
  assert.equal(failures.length, 0, `\n${failures.join('\n')}`);
});

test('every claim: gapSequence (if declared) matches parsed pattern', async () => {
  const claims = await loadClaims();
  const failures = [];
  for (const c of claims) {
    if (!Array.isArray(c.claim.gapSequence)) continue;
    const p = parsePattern(c.claim.pattern);
    const actual = gapSequence(p);
    if (actual.length !== c.claim.gapSequence.length ||
        !actual.every((v, i) => v === c.claim.gapSequence[i])) {
      failures.push(
        `${c.file}#patternClaims[${c.index}] "${c.claim.label}": ` +
          `declared gaps ${c.claim.gapSequence.join('-')} but pattern produces ${actual.join('-')}`,
      );
    }
  }
  assert.equal(failures.length, 0, `\n${failures.join('\n')}`);
});

test('every claim: isEuclidean assertion matches actual Bjorklund fit', async () => {
  const claims = await loadClaims();
  const failures = [];
  for (const c of claims) {
    const p = parsePattern(c.claim.pattern);
    if (c.claim.isEuclidean === false) {
      // Prove NO Euclidean fit at any (k, n) for this pattern's length.
      // Only checks (onsetCount(p), pattern.length) which is the meaningful
      // question. Other (k, n) with different lengths are irrelevant.
      const fit = findAnyEuclideanFit(p);
      if (fit !== null) {
        failures.push(
          `${c.file}#patternClaims[${c.index}] "${c.claim.label}": ` +
            `declared isEuclidean:false but pattern is E(${fit.k},${fit.n}) at rotation ${fit.rotation}`,
        );
      }
    } else if (typeof c.claim.isEuclidean === 'object' && c.claim.isEuclidean !== null) {
      const { k, n, rotation } = c.claim.isEuclidean;
      const actualRot = findEuclideanRotation(p, k, n);
      if (actualRot === null) {
        failures.push(
          `${c.file}#patternClaims[${c.index}] "${c.claim.label}": ` +
            `declared isEuclidean:{k:${k},n:${n}} but pattern is not Euclidean at any rotation`,
        );
      } else if (typeof rotation === 'number' && actualRot !== rotation) {
        failures.push(
          `${c.file}#patternClaims[${c.index}] "${c.claim.label}": ` +
            `declared rotation ${rotation} but pattern matches E(${k},${n}) at rotation ${actualRot}`,
        );
      }
    } else {
      failures.push(
        `${c.file}#patternClaims[${c.index}] "${c.claim.label}": ` +
          `isEuclidean must be false or {k, n, rotation?}, got ${JSON.stringify(c.claim.isEuclidean)}`,
      );
    }
  }
  assert.equal(failures.length, 0, `\n${failures.join('\n')}`);
});
