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
import { readFileSync, existsSync, readdirSync } from 'node:fs';
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
  // M001 S02+ shared theory-audit-remediation prose-claim host (D010).
  'site/tests/theory-audit-claims.test.mjs',
  // M003 S01 scope-and-framing gate: the About page's repositioning statement
  // and its declared scope exclusions.
  'site/tests/scope-framing.test.mjs',
  'site/tests/literature-enrichment.test.mjs',
  // M002 S06 citation-tier gate: every bibliography entry declares a tier,
  // and a named-theory claim may not cite below Tier A.
  'site/tests/citation-tier.test.mjs',
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

// M006/S02 (GAP02). The runner above names WHICH guardrail files run. This
// case guards the other half of the seam: that CI runs the whole site/tests
// directory, so a file named in no runner still cannot go unrun.
//
// Before M006/S02 six files ran nowhere in CI -- including
// presets-json-schema.test.mjs, which carries M005/S01's stale-presets.json
// guard, so the check that catches a stale generated file was itself unproven.
// Nothing noticed, because nothing was watching this seam.
//
// Asserts on the command rather than the step's `name`: a name is cosmetic and
// a command is what executes.
test('CI runs the whole site test suite, not only the named guardrails', () => {
  const workflow = readFileSync(join(REPO, '.github', 'workflows', 'ci.yml'), 'utf8');
  assert.match(
    workflow,
    /run:\s*npm --prefix site test\b/,
    'no CI step runs `npm --prefix site test`, so any site/tests file not named in ' +
      'check-doc-conformance.sh runs nowhere in CI — the gap M006/S02 closed',
  );
});

// M007/S02 (GAP04). Every gap in this family was found by CI going red after a
// green local run: doc-drift on M005's ship, the site suite in M006's planning,
// check-scripts-readme on M006's ship, generate-params-json on M002's. Each was
// fixed as an instance. This closes the class: a guard that no local command
// reaches fails here, so a twelfth cannot appear silently.
//
// Reachable means named in one of the four things a developer can actually
// invoke. A guard that genuinely cannot run from a clean checkout declares
// itself with an in-band `# local-unrunnable: <reason>` marker — greppable, so
// `grep -rn local-unrunnable scripts/` enumerates every exemption and that
// listing is the audit. An empty reason does not count: the hatch is for
// declaring an exemption, not for silencing the check.
test('every repo guard is reachable from a command a developer can run', () => {
  const sources = [
    join(REPO, '.jk', 'validations.yml'),
    join(REPO, 'scripts', 'pre-push-check.sh'),
    join(REPO, '.pre-commit-config.yaml'),
    RUNNER,
    // check-guards.sh is itself a declared token, so a guard it runs is
    // reachable through it.
    join(REPO, 'scripts', 'check-guards.sh'),
  ]
    .map((f) => readFileSync(f, 'utf8'))
    .join('\n');

  const scriptsDir = join(REPO, 'scripts');
  const guards = readdirSync(scriptsDir).filter(
    (f) => f.startsWith('check-') && (f.endsWith('.sh') || f.endsWith('.mjs')),
  );
  assert.ok(guards.length > 0, 'found no check-* guards in scripts/, which cannot be right');

  const unreachable = [];
  for (const guard of guards) {
    if (sources.includes(guard)) continue;
    const body = readFileSync(join(scriptsDir, guard), 'utf8');
    const marker = /^#\s*local-unrunnable:[ \t]*(.*)$/m.exec(body);
    if (marker && marker[1].trim().length > 0) continue;
    unreachable.push(
      marker
        ? `${guard} carries a local-unrunnable marker with no reason after the colon`
        : `${guard} is reachable from no declared token, the pre-push gate, pre-commit, or the doc-conformance runner`,
    );
  }

  assert.deepEqual(unreachable, [], unreachable.join('\n  '));
});

// M007/S03 (GAP05). GAP04 stops a guard becoming unreachable; it does not stop
// someone forgetting to run one. This asserts the gate itself: every token in
// .jk/validations.yml is either run by the pre-push hook or declared exempt
// with a reason, so the local gate stays CI minus what genuinely cannot run
// locally rather than drifting back into a subset of it.
//
// Neither side is inferred. Matching a token's command string against a shell
// line is brittle — `unit` is a compound cmake invocation, and `format`'s step
// deliberately runs a narrower arm than the token does — and a brittle check is
// one that gets deleted rather than fixed. So both sides declare themselves:
// `# pre-push-token: <name>` above the gate that runs one, and
// `# pre-push-exempt: <name> — <reason>` beside the token it exempts. Both are
// greppable, and an empty reason fails for the same reason GAP04's does.
test('every validation token is run by the pre-push gate or exempt with a reason', () => {
  const yaml = readFileSync(join(REPO, '.jk', 'validations.yml'), 'utf8');
  const hook = readFileSync(join(REPO, 'scripts', 'pre-push-check.sh'), 'utf8');

  const tokens = [...yaml.matchAll(/^([a-z][a-z0-9-]*):[ \t]/gm)].map((m) => m[1]);
  assert.ok(tokens.length > 0, 'parsed no tokens from .jk/validations.yml, which cannot be right');

  const run = new Set([...hook.matchAll(/^[ \t]*#[ \t]*pre-push-token:[ \t]*(\S+)/gm)].map((m) => m[1]));
  const exempt = new Map(
    [...yaml.matchAll(/^#[ \t]*pre-push-exempt:[ \t]*([a-z][a-z0-9-]*)[ \t]*(?:—|--)?[ \t]*(.*)$/gm)].map(
      (m) => [m[1], m[2].trim()],
    ),
  );

  const problems = [];
  for (const token of tokens) {
    const isRun = run.has(token);
    const isExempt = exempt.has(token);
    if (isRun && isExempt) {
      problems.push(`${token} is both run by the pre-push gate and declared exempt from it`);
    } else if (!isRun && !isExempt) {
      problems.push(
        `${token} is neither run by the pre-push gate nor declared exempt — add a ` +
          `'# pre-push-token: ${token}' marker above the gate that runs it, or a ` +
          `'# pre-push-exempt: ${token} — <reason>' beside it in .jk/validations.yml`,
      );
    } else if (isExempt && exempt.get(token).length === 0) {
      problems.push(`${token} carries a pre-push-exempt marker with no reason after the dash`);
    }
  }
  for (const name of run) {
    if (!tokens.includes(name)) {
      problems.push(`the pre-push gate declares '# pre-push-token: ${name}', which is not a declared token`);
    }
  }

  assert.deepEqual(problems, [], problems.join('\n  '));
});
