// Non-vacuity wiring test for the shared doc-conformance guardrail runner
// (originally M071 S06 T02; extended by M001 S01 T05).
//
// The shared runner scripts/check-doc-conformance.sh is the single source of
// truth for WHICH guardrail tests run; the CI site-lint job and the local
// pre-push hook both invoke it so their lists cannot drift apart. This test
// guards that seam from three failure modes it would otherwise rot into:
//
//   1. Coverage drop — a required guardrail file is quietly removed from the
//      runner's TESTS array (the suite still passes, but no longer guards that
//      file). REQUIRED below is the canonical contract; every entry must be
//      named in the runner.
//   2. Phantom entry — the runner names a test file that does not exist on
//      disk, so `node --test` silently runs nothing for it (a green vacuous
//      pass). Every file the runner names must exist.
//   3. Wiring rot — CI or pre-push stops invoking the runner and instead runs
//      some ad-hoc subset, so the two lists diverge. Both must invoke the
//      runner by name.
//
// Failure messages name the exact file / caller so the fix is unambiguous.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync, existsSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const SITE = join(HERE, '..');
const REPO = join(SITE, '..');

const RUNNER = join(REPO, 'scripts', 'check-doc-conformance.sh');
const CI = join(REPO, '.github', 'workflows', 'ci.yml');
const PRE_PUSH = join(REPO, 'scripts', 'pre-push-check.sh');

// The canonical guardrail contract. Adding a guardrail means adding it here
// AND to the runner; this list is what the "coverage drop" assertion checks
// the runner against. Paths are repo-relative (the runner cd's to REPO_ROOT).
const REQUIRED = [
  // M071 doc-conformance guardrails.
  'site/tests/euclidean-claims.test.mjs',
  'site/tests/appendix-euclidean-claims.test.mjs',
  'site/tests/prose-pattern-claims.test.mjs',
  'site/tests/chapter-euclidean-guardrail.test.mjs',
  'site/tests/polypatch-preset-resolution.test.mjs',
  'site/tests/preset-table-conformance.test.mjs',
  'site/tests/preset-taxonomy-conformance.test.mjs',
  'site/tests/prose-conformance-claims.test.mjs',
  'site/tests/idiom-break-framing.test.mjs',
  'site/tests/theory-euclidean-guardrail.test.mjs',
  'site/tests/theory-patch-conformance.test.mjs',
  // M001 S01 shared prose-claim helper self-test.
  'site/tests/prose-claim-helpers.test.mjs',
  // M001 S01 audit-ledger completeness gates.
  'docs/audits/theory-audit-remediation.test.mjs',
  'docs/audits/parity-matrix.test.mjs',
  'docs/audits/gap-closure-plan.test.mjs',
];

// Extract every `<dir>/<name>.test.mjs` token the runner script names. The
// runner keeps them in a bash TESTS=() array, one per line, so a simple regex
// over the file body is a faithful read of what the runner actually runs.
function runnerTests() {
  const body = readFileSync(RUNNER, 'utf8');
  const matches =
    body.match(/(?:site\/tests|docs\/audits)\/[A-Za-z0-9._-]+\.test\.mjs/g) ??
    [];
  return [...new Set(matches)];
}

test('runner exists and is the single source of truth', () => {
  assert.ok(existsSync(RUNNER), `missing shared runner: ${RUNNER}`);
});

test('runner names at least the full REQUIRED guardrail set (no coverage drop)', () => {
  const named = new Set(runnerTests());
  assert.ok(named.size > 0, 'runner names no test files — non-vacuity failure');
  for (const req of REQUIRED) {
    assert.ok(
      named.has(req),
      `check-doc-conformance.sh dropped required guardrail: ${req}`,
    );
  }
});

test('every test file the runner names exists on disk (no phantom entry)', () => {
  for (const rel of runnerTests()) {
    const abs = join(REPO, rel);
    assert.ok(
      existsSync(abs),
      `check-doc-conformance.sh names a non-existent test file: ${rel}`,
    );
  }
});

test('CI site-lint job invokes the shared runner (no wiring rot)', () => {
  const ci = readFileSync(CI, 'utf8');
  assert.match(
    ci,
    /check-doc-conformance\.sh/,
    '.github/workflows/ci.yml no longer invokes scripts/check-doc-conformance.sh',
  );
});

test('pre-push hook invokes the shared runner (no wiring rot)', () => {
  const hook = readFileSync(PRE_PUSH, 'utf8');
  assert.match(
    hook,
    /check-doc-conformance\.sh/,
    'scripts/pre-push-check.sh no longer invokes scripts/check-doc-conformance.sh',
  );
});
