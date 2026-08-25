// Shared prose-claim assertion helpers for M001 chapter-conformance tests.
//
// M001 slices S02-S06 each lock a corrected claim in a chapter/guide by
// asserting that a forbidden (pre-correction) phrase is ABSENT and a
// corrective phrase is PRESENT. Per D003 those cases must all build on one
// mechanism, and the mechanism must:
//
//   1. survive editorial tweaks that do not change the claim - added/removed
//      commas, curly vs straight quotes, extra whitespace, case; and
//   2. still fail when the claim reverses (a word like "no" appearing or
//      disappearing, a number flipping, an attribution swapping).
//
// The mechanism is deliberately word-level, not sentence-level: the shape
// of a claim is a bag of ordered words the reader must encounter in that
// order, ignoring purely typographic punctuation between them. Regex escape
// hatches (forbiddenRegex / presentRegex) exist for structured spots such
// as a table cell where a numeric token is the load-bearing part.
//
// Every failure names the chapter file, the claim id, which side of the
// assertion moved (forbidden reappeared vs corrective went missing), and
// the authority (rule) the claim agrees with, so a red run is diagnosable
// without opening the test.

// Normalize prose so trivial editorial edits do not move a claim.
// Kept characters: letters, digits, intra-word apostrophes and hyphens.
// Everything else (commas, periods, semicolons, quotes, whitespace runs)
// collapses to a single space. Case is folded.
export function normalizeProse(text) {
  return String(text)
    .replace(/[‘’]/g, "'")
    .replace(/[“”]/g, '"')
    .toLowerCase()
    .replace(/[^\p{L}\p{N}'\-]+/gu, ' ')
    .replace(/\s+/g, ' ')
    .trim();
}

// True iff `phrase` appears in `source` after normalization on both sides.
// Substring semantics: word order is preserved, punctuation between words
// is not.
export function containsClaim(source, phrase) {
  const s = normalizeProse(source);
  const p = normalizeProse(phrase);
  if (p === '') return true;
  return s.includes(p);
}

// Assert one claim against one source. Throws via the caller's assert on
// the first violation, with a message that names file, id, side, and rule.
//
// Claim shape:
//   id             stable identifier (e.g. "S02-F3")
//   file           source path being asserted (for the error message)
//   rule           one-line authority the claim agrees with
//   forbidden      string phrases that must be ABSENT under normalization
//   forbiddenRegex RegExp variants that must not match the raw source
//   present        string phrases that must be PRESENT under normalization
//   presentRegex   RegExp variants that must match the raw source
export function assertClaim(assert, claim, source) {
  const { id, file } = claim;
  if (!id) throw new Error('assertClaim: claim.id is required');
  if (!file) throw new Error('assertClaim: claim.file is required');
  const rule = claim.rule ?? '(no rule cited)';
  const where = `${file} [${id}]`;
  const authority = `Authority: ${rule}.`;

  for (const phrase of claim.forbidden ?? []) {
    if (containsClaim(source, phrase)) {
      assert.fail(
        `${where}: forbidden phrase reappeared: ${JSON.stringify(phrase)}. ${authority}`,
      );
    }
  }
  for (const re of claim.forbiddenRegex ?? []) {
    if (re.test(source)) {
      assert.fail(
        `${where}: forbidden pattern reappeared: ${re}. ${authority}`,
      );
    }
  }
  for (const phrase of claim.present ?? []) {
    if (!containsClaim(source, phrase)) {
      assert.fail(
        `${where}: corrective phrase went missing: ${JSON.stringify(phrase)}. ${authority}`,
      );
    }
  }
  for (const re of claim.presentRegex ?? []) {
    if (!re.test(source)) {
      assert.fail(
        `${where}: corrective pattern went missing: ${re}. ${authority}`,
      );
    }
  }
}

// Register one `node:test` case per claim. `loadSource(file)` returns the
// raw text of that file (async). Used by chapter guards to avoid repeating
// the per-file boilerplate.
export function registerClaimTests({ test, assert, claims, loadSource, label = 'forbidden absent, correction present' }) {
  for (const claim of claims) {
    test(`${claim.id} (${claim.file}): ${label}`, async () => {
      const src = await loadSource(claim.file);
      assertClaim(assert, claim, src);
    });
  }
}
