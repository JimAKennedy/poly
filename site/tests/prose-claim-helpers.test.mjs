// Green/red self-test for the shared prose-claim assertion helpers.
//
// M001/S01 T04 ships site/tests/helpers/prose-claims.mjs as the single
// mechanism S02-S06 will use to lock corrected chapter claims. Per the
// slice contract this self-test proves the mechanism in BOTH directions:
//
//   * green: helpers accept a corrected fixture even after editorial
//     tweaks (comma changes, curly quotes, extra whitespace, case);
//   * red: helpers reject a reverted fixture, and the failure message
//     names the chapter file, the claim id, and which side moved.
//
// No chapter file under src/content/docs is asserted here - those cases
// are owned by S02-S06. Only inline fixtures are exercised.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import {
  assertClaim,
  containsClaim,
  normalizeProse,
  registerClaimTests,
} from './helpers/prose-claims.mjs';

// --- normalizeProse ---------------------------------------------------------

test('normalizeProse collapses whitespace runs and trims edges', () => {
  assert.equal(normalizeProse('  foo\n\tbar   baz\n'), 'foo bar baz');
});

test('normalizeProse folds curly quotes and preserves intra-word apostrophes', () => {
  // Curly apostrophe survives as an intra-word straight apostrophe.
  assert.equal(normalizeProse('Poly’s'), "poly's");
  // Curly double quotes fold to straight, then strip as punctuation between words.
  assert.equal(normalizeProse('Poly’s “macro”'), "poly's macro");
});

test('normalizeProse strips punctuation between words but keeps word order', () => {
  assert.equal(
    normalizeProse('no ornamentation, no dynamics contour, and no mutation'),
    'no ornamentation no dynamics contour and no mutation',
  );
});

test('normalizeProse is case-insensitive', () => {
  assert.equal(normalizeProse('Chaugun Ornament'), 'chaugun ornament');
});

// --- containsClaim ----------------------------------------------------------

test('containsClaim survives editorial comma insertion', () => {
  const withoutComma = 'the clave takes no ornamentation and no mutation';
  const withComma = 'clave takes no ornamentation, and no mutation';
  assert.ok(containsClaim(withoutComma, withComma));
});

test('containsClaim survives curly vs straight quote mismatch', () => {
  assert.ok(containsClaim('Poly’s macro modulation is off', "Poly's macro"));
  assert.ok(containsClaim("Poly's macro modulation is off", 'Poly’s macro'));
});

test('containsClaim survives whitespace and case tweaks', () => {
  assert.ok(containsClaim('The  Amen\nBreak drummer', 'the amen break drummer'));
});

test('containsClaim STILL fails when a load-bearing word disappears', () => {
  // Remove the "no" and the claim reverses.
  assert.ok(!containsClaim('the clave takes ornamentation', 'clave takes no ornamentation'));
});

test('containsClaim STILL fails when a load-bearing word is swapped', () => {
  assert.ok(!containsClaim('Tigun ornament dominates', 'Chaugun ornament'));
});

// --- assertClaim: green paths ----------------------------------------------

const CORRECTED_CLAVE = `
  The clave takes no ornamentation, no dynamics contour, and no mutation;
  it is Poly’s macro-level constant.
`;
const CLAVE_CLAIM = {
  id: 'SELF-F1',
  file: 'fixture-afro-cuban.mdx',
  rule: 'theory-afro-cuban Rule 1: clave takes no ornamentation, no dynamics contour, no mutation',
  forbidden: ["participates in Poly's macro modulation"],
  present: ['no ornamentation, no dynamics contour, and no mutation'],
};

test('assertClaim green: corrected fixture with editorial tweaks passes', () => {
  // Reflow whitespace, swap curly for straight quote, drop a comma.
  const tweaked = CORRECTED_CLAVE
    .replace(/\s+/g, ' ')
    .replace(/’/g, "'")
    .replace('contour,', 'contour');
  assertClaim(assert, CLAVE_CLAIM, tweaked);
});

test('assertClaim green: empty claim (no forbidden, no present) is a no-op', () => {
  assertClaim(
    assert,
    { id: 'SELF-EMPTY', file: 'fixture.mdx', rule: 'trivial' },
    'anything at all',
  );
});

// --- assertClaim: red paths + message shape --------------------------------

const REVERTED_CLAVE = `
  The clave takes ornamentation and participates in Poly's macro modulation.
`;

// Capture the AssertionError message thrown by `fn`, or fail the calling
// test if `fn` did not throw. `assert.throws` in strict mode swallows the
// error object, so we can't use it when we need to inspect the message.
function capture(fn) {
  try {
    fn();
  } catch (err) {
    return String(err?.message ?? err);
  }
  assert.fail('expected callback to throw an assertion error, but it did not');
}

test('assertClaim red: reverted fixture triggers "corrective phrase went missing"', () => {
  const msg = capture(() => assertClaim(assert, CLAVE_CLAIM, REVERTED_CLAVE));
  assert.match(msg, /fixture-afro-cuban\.mdx/, 'message names the file');
  assert.match(msg, /\[SELF-F1\]/, 'message names the claim id');
  assert.match(
    msg,
    /(corrective phrase went missing|forbidden phrase reappeared)/,
    'message names which side moved',
  );
  assert.match(msg, /Authority: theory-afro-cuban Rule 1/, 'message cites the rule');
});

test('assertClaim red: forbidden branch names side="forbidden reappeared"', () => {
  // Isolate the forbidden branch: no `present` requirement.
  const forbiddenOnly = { ...CLAVE_CLAIM, present: undefined };
  const msg = capture(() => assertClaim(assert, forbiddenOnly, REVERTED_CLAVE));
  assert.match(msg, /forbidden phrase reappeared/);
  assert.match(msg, /\[SELF-F1\]/);
});

test('assertClaim red: present branch names side="corrective went missing"', () => {
  // Isolate the present branch: no `forbidden` requirement.
  const presentOnly = { ...CLAVE_CLAIM, forbidden: undefined };
  const msg = capture(() => assertClaim(assert, presentOnly, REVERTED_CLAVE));
  assert.match(msg, /corrective phrase went missing/);
  assert.match(msg, /\[SELF-F1\]/);
});

test('assertClaim red: regex variants also name file, id, and side', () => {
  const claim = {
    id: 'SELF-F2',
    file: 'fixture-jazz.mdx',
    rule: 'jazz Rule 1: Ride (steady) carries 0% mutation',
    forbiddenRegex: [/Ride \(steady\).*\b10%/],
    presentRegex: [/Ride \(steady\).*\b0%/],
  };
  const revertedTableRow = '| Ride (steady) | 10% |';
  const msg = capture(() => assertClaim(assert, claim, revertedTableRow));
  assert.match(msg, /fixture-jazz\.mdx/);
  assert.match(msg, /\[SELF-F2\]/);
  assert.match(msg, /forbidden pattern reappeared/);
});

test('assertClaim red: presentRegex missing names "corrective pattern went missing"', () => {
  const claim = {
    id: 'SELF-F3',
    file: 'fixture-jazz.mdx',
    rule: 'jazz Rule 1: Ride (steady) carries 0% mutation',
    presentRegex: [/Ride \(steady\).*\b0%/],
  };
  const msg = capture(() => assertClaim(assert, claim, '| Ride (steady) | 25% |'));
  assert.match(msg, /corrective pattern went missing/);
  assert.match(msg, /\[SELF-F3\]/);
});

test('assertClaim: an editorial comma alone does NOT trip the guard', () => {
  // Confirms property 1 end-to-end: adding/removing a comma inside the
  // corrective phrase must not turn the guard red.
  const source = 'the clave takes no ornamentation no dynamics contour and no mutation';
  const claim = {
    id: 'SELF-F4',
    file: 'fixture.mdx',
    rule: 'comma insensitivity',
    present: ['no ornamentation, no dynamics contour, and no mutation'],
  };
  assertClaim(assert, claim, source);
});

test('assertClaim: dropping a load-bearing "no" DOES trip the guard (claim reversed)', () => {
  // Confirms property 2 end-to-end: even under normalization, removing
  // the word "no" must still fail.
  const source = 'the clave takes ornamentation dynamics contour and mutation';
  const claim = {
    id: 'SELF-F5',
    file: 'fixture.mdx',
    rule: 'load-bearing word survives normalization',
    present: ['no ornamentation, no dynamics contour, and no mutation'],
  };
  const msg = capture(() => assertClaim(assert, claim, source));
  assert.match(msg, /corrective phrase went missing/);
  assert.match(msg, /\[SELF-F5\]/);
});

// --- registerClaimTests: batch registration --------------------------------

test('registerClaimTests registers one subtest per claim and drives loadSource', async (t) => {
  const seen = [];
  const stubTest = (name, fn) => t.test(name, fn);
  const claims = [
    {
      id: 'BATCH-1',
      file: 'a.mdx',
      rule: 'a',
      present: ['hello world'],
    },
    {
      id: 'BATCH-2',
      file: 'b.mdx',
      rule: 'b',
      present: ['goodbye world'],
    },
  ];
  const loadSource = async (file) => {
    seen.push(file);
    return { 'a.mdx': 'hello world', 'b.mdx': 'goodbye world' }[file];
  };
  registerClaimTests({ test: stubTest, assert, claims, loadSource, label: 'batch' });
  // Give the registered subtests a chance to run and record the loads.
  await new Promise((r) => setImmediate(r));
  assert.deepEqual(seen.sort(), ['a.mdx', 'b.mdx']);
});
