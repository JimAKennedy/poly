#!/usr/bin/env node
// verify-good-first-issue-count.test.js — contract test for M031 S03 T02.
//
// The onboarding slice requires that at least 5 well-scoped open GitHub issues
// carry the "good first issue" label so a newcomer landing on the repo can find
// something to pick up (see ROADMAP.md and the good-first-issue filter linked
// from README.md / CONTRIBUTING.md).
//
// The label set lives on GitHub, not in the working tree, so this test queries
// the live repo via the `gh` CLI. In environments without `gh`, without network,
// or without an authenticated token (e.g. some CI/sandbox contexts), the query
// cannot run — there the test SKIPS rather than fails, matching how the other
// scripts/check-*.mjs contract checks degrade when their external surface is
// unavailable. When `gh` IS available and authenticated, the assertion is hard:
// fewer than MIN_GOOD_FIRST_ISSUES labeled open issues turns the build red.
//
// Run: `node --test scripts/verify-good-first-issue-count.test.js`

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { execFileSync } from 'node:child_process';

const LABEL = 'good first issue';
const MIN_GOOD_FIRST_ISSUES = 5;

// Query open issues carrying the label. Returns an array of issue numbers, or
// `null` when the external `gh` surface is unavailable (missing binary, no auth,
// or no network) so the caller can skip rather than fail.
function fetchLabeledOpenIssues() {
  let stdout;
  try {
    stdout = execFileSync(
      'gh',
      [
        'issue',
        'list',
        '--state',
        'open',
        '--label',
        LABEL,
        '--limit',
        '200',
        '--json',
        'number',
      ],
      { encoding: 'utf8', stdio: ['ignore', 'pipe', 'pipe'] },
    );
  } catch (err) {
    // ENOENT => gh not installed; non-zero exit => not authenticated / offline.
    return null;
  }

  let parsed;
  try {
    parsed = JSON.parse(stdout);
  } catch {
    return null;
  }
  if (!Array.isArray(parsed)) return null;
  return parsed.map((issue) => issue.number);
}

test('at least 5 open issues carry the "good first issue" label', (t) => {
  const numbers = fetchLabeledOpenIssues();

  if (numbers === null) {
    t.skip('gh CLI unavailable, unauthenticated, or offline — cannot verify label set');
    return;
  }

  assert.ok(
    numbers.length >= MIN_GOOD_FIRST_ISSUES,
    `expected >= ${MIN_GOOD_FIRST_ISSUES} open issues labeled "${LABEL}", ` +
      `found ${numbers.length} (issues: ${numbers.join(', ') || 'none'}). ` +
      `Label more well-scoped issues with: gh issue edit <n> --add-label "${LABEL}"`,
  );
});
