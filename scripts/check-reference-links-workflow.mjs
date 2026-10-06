// check-reference-links-workflow.mjs — the dead-reference workflow keeps the
// shape that makes it advisory (verifiable-references M006/S01, VR16).
//
// The workflow is only trustworthy while three things hold: it runs on a
// schedule (so link rot is found by a job, not a reader); it can never fail a
// pull request (a flaky gate on somebody else's web server gets disabled); and
// only scheduled and manual runs may write an issue (a PR must never open
// one). Each rule is a pure function over the workflow's text, asserted on an
// inline fixture that must produce a finding and on the real file, as
// check-workflow-hygiene.mjs does. No YAML parser: targeted matches.
//
// Run: `node --test scripts/check-reference-links-workflow.mjs`
// (check-guards.sh and the code-quality CI job).

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const REPO = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const WORKFLOW = resolve(REPO, '.github', 'workflows', 'reference-links.yml');

const PR_PATHS = [
  'site/src/content/docs/appendix-references.mdx',
  'site/src/content/theory/theory-references.mdx',
  'scripts/reference-links.mjs',
  '.github/workflows/reference-links.yml',
];

// The `jobs:` block split into { name: text } by its two-space-indented keys.
export function jobs(text) {
  const start = text.search(/^jobs:\s*$/m);
  if (start < 0) return {};
  const body = text.slice(start).split('\n').slice(1);
  const out = {};
  let name = null;
  for (const line of body) {
    const m = /^ {2}([A-Za-z0-9_-]+):\s*$/.exec(line);
    if (m) { name = m[1]; out[name] = ''; continue; }
    if (/^\S/.test(line)) break;
    if (name) out[name] += line + '\n';
  }
  return out;
}

export function triggerProblems(text) {
  const p = [];
  if (!/^\s+schedule:\s*\n\s+- cron: '\d+ \d+ \* \* [0-6]'/m.test(text)) p.push('no weekly schedule (a cron with a single day of the week)');
  if (!/^\s+workflow_dispatch:/m.test(text) || !/^\s+extra_url:/m.test(text)) p.push('no workflow_dispatch with an extra_url input');
  if (!/^\s+pull_request:/m.test(text)) p.push('no pull_request trigger');
  for (const path of PR_PATHS) if (!text.includes(`- '${path}'`)) p.push(`pull_request paths omit ${path}`);
  return p;
}

export function permissionProblems(text) {
  const p = [];
  if (!/^permissions:\s*\n\s+contents: read\s*$/m.test(text)) p.push('top-level permissions are not exactly contents: read');
  const writes = (text.match(/issues: write/g) ?? []).length;
  const j = jobs(text);
  if (writes !== 1) p.push(`issues: write appears ${writes} times; it belongs on the report job alone`);
  if (!j.report || !/issues: write/.test(j.report)) p.push('the report job does not hold issues: write');
  if (!j.report || !/if: .*github\.event_name != 'pull_request'/.test(j.report)) p.push("the report job does not exclude pull_request runs");
  return p;
}

export function reportingProblems(text) {
  const p = [];
  const check = jobs(text).check ?? '';
  if (!/node scripts\/reference-links\.mjs[^\n]*--json[^\n]*--summary|node scripts\/reference-links\.mjs[^\n]*--summary[^\n]*--json/.test(check)) {
    p.push('the check job does not run the checker with --json and --summary');
  }
  if (!/GITHUB_STEP_SUMMARY/.test(check)) p.push('the summary is not written to the job summary');
  if (!/uses: actions\/upload-artifact@/.test(check)) p.push('the JSON report is not uploaded as an artifact');
  if (/--strict|\bexit 1\b|continue-on-error: false/.test(text)) p.push('something can fail the job on findings');
  return p;
}

const FIXTURE_BAD = `name: Reference links
on:
  schedule:
    - cron: '0 4 * * *'
  pull_request:
permissions:
  contents: write
jobs:
  check:
    runs-on: ubuntu-latest
    permissions:
      issues: write
    steps:
      - run: node scripts/reference-links.mjs || exit 1
`;

test('the fixture breaks every rule (the rules can fail)', () => {
  assert.ok(triggerProblems(FIXTURE_BAD).length >= 3, 'trigger rules found nothing in a bad fixture');
  assert.ok(permissionProblems(FIXTURE_BAD).length >= 2, 'permission rules found nothing in a bad fixture');
  assert.ok(reportingProblems(FIXTURE_BAD).length >= 3, 'reporting rules found nothing in a bad fixture');
});

const real = () => readFileSync(WORKFLOW, 'utf8');

test('runs weekly, on demand with an injected URL, and on PRs that touch what it checks', () => {
  assert.deepEqual(triggerProblems(real()), []);
});

test('reads by default, and only scheduled or manual runs may write an issue', () => {
  assert.deepEqual(permissionProblems(real()), []);
});

test('reports to the job summary and an artifact, and never fails on findings', () => {
  assert.deepEqual(reportingProblems(real()), []);
});
