#!/usr/bin/env node
// check-workflow-hygiene.mjs — every workflow says what its token may do, a
// superseded PR push is cancelled, and nothing third-party runs at a moving ref.
//
// open-source-launch M002/S01, OS06/OS07/OS08. ci.yml and sanitizers.yml
// declared no top-level `permissions:`, so every job's token fell back to a
// repository default the tree cannot see; ci.yml had no `concurrency:` group,
// so every push to an open PR ran the full macOS and Windows matrix to
// completion; and pr-af-review.yml checked out Agent-Field/pr-af at whatever
// its default branch held that day and ran it with a secret in its
// environment. The three jk-standards workflow checks pass on all of that —
// workflow-permissions judges only reusable-workflow calls, workflow-concurrency
// judges only groups that exist, and action-pinning reads `uses:`, not a
// checkout's `repository:` input — so this guard is the one that fails.
//
// Each rule is a pure function over a workflow's text, asserted twice: on an
// inline fixture that must produce a finding (the red proof, built into every
// run) and on every file under .github/workflows/ (the tree). No YAML parser:
// targeted structural matches, as check-release-workflow.mjs does.
//
// Run: `node --test scripts/check-workflow-hygiene.mjs` (wired into
// check-guards.sh and the code-quality CI job).
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync, readdirSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const REPO = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const WORKFLOWS = resolve(REPO, '.github', 'workflows');
const FIRST_PARTY_PREFIX = 'JimAKennedy/';

// --- The rules -------------------------------------------------------------

// True when no `permissions:` key sits at column 0. A job-level block is
// indented and does not count: it narrows or widens one job, and says nothing
// about the token the other jobs receive.
function missingTopLevelPermissions(text) {
  return !/^permissions:/m.test(text);
}

// The `repository:` values of checkout steps that name a third-party
// repository and carry no `ref:` that is a 40-character commit SHA. A step is
// the text from one `- uses:` / `- name:` line to the next. The repository's
// own checkouts (`${{ github.repository }}` or anything under the owner's
// account) move with the tree and are exempt, as action-pinning exempts local
// `./` actions.
function unpinnedThirdPartyCheckouts(text) {
  const steps = text.split(/^(?=[ \t]*-[ \t]+(?:uses|name):)/m);
  const findings = [];
  for (const step of steps) {
    const repo = step.match(/^[ \t]*repository:[ \t]*(.+?)[ \t]*(?:#.*)?$/m);
    if (!repo) continue;
    const value = repo[1].replace(/^['"]|['"]$/g, '');
    if (value === '${{ github.repository }}' || value.startsWith(FIRST_PARTY_PREFIX)) continue;
    const pinned = /^[ \t]*ref:[ \t]*[0-9a-fA-F]{40}[ \t]*(?:#.*)?$/m.test(step);
    if (!pinned) findings.push(value);
  }
  return findings;
}

// The top-level `concurrency:` block as `{ group, cancel }`, or null when the
// workflow declares none. Values are the raw expressions, untouched.
function concurrencyShape(text) {
  const block = text.match(/^concurrency:\n((?:[ \t]+\S.*\n?)+)/m);
  if (!block) return null;
  const group = block[1].match(/^[ \t]*group:[ \t]*(.+?)[ \t]*$/m);
  const cancel = block[1].match(/^[ \t]*cancel-in-progress:[ \t]*(.+?)[ \t]*$/m);
  return { group: group ? group[1] : null, cancel: cancel ? cancel[1] : null };
}

// The indented lines of the top-level `permissions:` block, trimmed.
function topLevelGrant(text) {
  const block = text.match(/^permissions:\n((?:[ \t]+\S.*\n?)+)/m);
  return block ? block[1].trim().split('\n').map((l) => l.trim()) : [];
}

// --- Red fixtures: each rule fires on a workflow that violates it -----------

test('missingTopLevelPermissions fires on a job-level-only block and not on a top-level one', () => {
  const jobOnly = 'name: x\non: push\njobs:\n  a:\n    permissions:\n      contents: read\n    runs-on: ubuntu-latest\n';
  assert.equal(missingTopLevelPermissions(jobOnly), true, 'a job-level block must not satisfy the rule');
  const topLevel = 'name: x\non: push\npermissions:\n  contents: read\njobs:\n  a:\n    runs-on: ubuntu-latest\n';
  assert.equal(missingTopLevelPermissions(topLevel), false);
});

test('unpinnedThirdPartyCheckouts names a third-party repository: with no SHA ref, and only that', () => {
  const unpinned =
    'steps:\n      - name: Checkout PR-AF\n        uses: actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1 # v5\n' +
    '        with:\n          repository: Agent-Field/pr-af\n          path: pr-af\n' +
    '      - name: Next\n        run: echo\n';
  assert.deepEqual(unpinnedThirdPartyCheckouts(unpinned), ['Agent-Field/pr-af']);
  const pinned = unpinned.replace(
    '          path: pr-af\n',
    '          ref: 421fbd23bf1c5a2c3916d7046c97b5273958e1f4 # upstream main\n          path: pr-af\n',
  );
  assert.deepEqual(unpinnedThirdPartyCheckouts(pinned), []);
  const tagRef = unpinned.replace('          path: pr-af\n', '          ref: v1.2.3\n          path: pr-af\n');
  assert.deepEqual(unpinnedThirdPartyCheckouts(tagRef), ['Agent-Field/pr-af'], 'a tag is a moving ref');
  const self = unpinned.replace('Agent-Field/pr-af', '${{ github.repository }}');
  assert.deepEqual(unpinnedThirdPartyCheckouts(self), []);
  const owner = unpinned.replace('Agent-Field/pr-af', 'JimAKennedy/audio-meta');
  assert.deepEqual(unpinnedThirdPartyCheckouts(owner), []);
});

test('concurrencyShape is null without a block and reads group and cancel with one', () => {
  assert.equal(concurrencyShape('name: x\non: push\njobs: {}\n'), null);
  const withBlock =
    'name: x\non: push\nconcurrency:\n  group: ${{ github.workflow }}-${{ github.ref }}\n' +
    "  cancel-in-progress: ${{ github.event_name == 'pull_request' }}\njobs: {}\n";
  assert.deepEqual(concurrencyShape(withBlock), {
    group: '${{ github.workflow }}-${{ github.ref }}',
    cancel: "${{ github.event_name == 'pull_request' }}",
  });
});

// --- The tree ---------------------------------------------------------------

const files = readdirSync(WORKFLOWS)
  .filter((f) => f.endsWith('.yml') || f.endsWith('.yaml'))
  .sort();
const read = (f) => readFileSync(resolve(WORKFLOWS, f), 'utf8');

test('there are workflows to check', () => {
  assert.ok(files.length > 0, `no workflow files under ${WORKFLOWS}`);
});

for (const f of files) {
  test(`${f} declares a top-level permissions block`, () => {
    assert.equal(
      missingTopLevelPermissions(read(f)),
      false,
      `${f} has no top-level permissions: — every job's token falls back to a repository default the tree cannot see (OS06)`,
    );
  });
}

for (const f of files) {
  test(`${f} pins every third-party checkout to a commit`, () => {
    assert.deepEqual(
      unpinnedThirdPartyCheckouts(read(f)),
      [],
      `${f} checks out a third-party repository without a 40-character SHA ref — whatever its default branch holds today runs here (OS08)`,
    );
  });
}

test('ci.yml cancels superseded pull-request runs and never a push to main', () => {
  assert.deepEqual(
    concurrencyShape(read('ci.yml')),
    {
      group: '${{ github.workflow }}-${{ github.ref }}',
      cancel: "${{ github.event_name == 'pull_request' }}",
    },
    'ci.yml must declare a ref-scoped concurrency group that cancels in progress for pull requests only (OS07)',
  );
});

test("ci.yml's top-level grant is exactly contents: read, and secrets-scan keeps its own pull-requests: write", () => {
  const ci = read('ci.yml');
  assert.deepEqual(topLevelGrant(ci), ['contents: read'], 'ci.yml must grant contents: read and nothing else at the top level');
  const secretsScan = ci.match(/^  secrets-scan:\n((?:    .*\n)+)/m);
  assert.ok(secretsScan, 'ci.yml has no secrets-scan job');
  assert.match(secretsScan[1], /^    permissions:\n(?:      .*\n)*?      pull-requests: write\n/m, 'secrets-scan lost its job-local pull-requests: write');
});
