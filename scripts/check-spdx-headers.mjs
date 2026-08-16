// M031 S05 T01: proof for the SPDX-header OSS-maturity guard.
//
// Two things must hold and stay held (anti-drift seam, MEM110):
//
//   A. WIRING — scripts/check-spdx-headers.sh exists, is executable, and is
//      invoked BY NAME in ci.yml so a missing header fails the build. If a
//      future edit drops the guard or unwires it from CI, these fail long
//      before CI runs.
//
//   B. BEHAVIOR — the guard is not a rubber stamp:
//        * run over the real tree (no args) exits 0 — every tracked
//          C/C++/ObjC source currently carries the header (green-now);
//        * a file whose first line IS the header passes;
//        * a file whose first line is NOT the header fails with exit 1 and
//          the offending path named in stderr (red-on-regression).
//
// Dependency-free by design: tests/ci and scripts/ carry no package.json, so we
// assert against the raw workflow text (matching check-release-workflow.mjs and
// clang-tidy-gate.test.mjs) rather than pulling in a YAML dependency.
//
// Run: `node --test scripts/check-spdx-headers.mjs`

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync, existsSync, writeFileSync, mkdtempSync, statSync } from 'node:fs';
import { spawnSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import { dirname, join, resolve } from 'node:path';
import { tmpdir } from 'node:os';
import { constants as fsConstants, accessSync } from 'node:fs';

const __dirname = dirname(fileURLToPath(import.meta.url));
const repoRoot = resolve(__dirname, '..');
const ciPath = join(repoRoot, '.github', 'workflows', 'ci.yml');
const guardPath = join(repoRoot, 'scripts', 'check-spdx-headers.sh');
const SPDX_LINE = '// SPDX-License-Identifier: GPL-3.0-or-later';

const runGuard = (args) =>
  spawnSync('bash', [guardPath, ...args], { cwd: repoRoot, encoding: 'utf8' });

// --- A. Wiring ---------------------------------------------------------------

test('guard script exists, is executable, and is a regular file', () => {
  assert.ok(existsSync(guardPath), `guard not found at ${guardPath}`);
  assert.ok(statSync(guardPath).isFile(), 'guard must be a regular file');
  assert.doesNotThrow(
    () => accessSync(guardPath, fsConstants.X_OK),
    'guard must be executable',
  );
});

test('ci.yml invokes the SPDX guard by name so a missing header blocks the build', () => {
  assert.ok(existsSync(ciPath), `ci.yml not found at ${ciPath}`);
  const ci = readFileSync(ciPath, 'utf8');
  assert.match(
    ci,
    /scripts\/check-spdx-headers\.sh/,
    'ci.yml must invoke scripts/check-spdx-headers.sh',
  );
});

// --- B. Behavior -------------------------------------------------------------

test('real tree is fully stamped — guard exits 0 with no args (green-now)', () => {
  const r = runGuard([]);
  const out = `${r.stdout || ''}${r.stderr || ''}`;
  assert.equal(r.status, 0, `guard should pass on the real tree:\n${out}`);
  assert.match(out, /carry the SPDX header/, 'guard should report the passing count');
});

test('a file whose first line is the header PASSES', () => {
  const dir = mkdtempSync(join(tmpdir(), 'spdx-good-'));
  const good = join(dir, 'good.cpp');
  writeFileSync(good, `${SPDX_LINE}\n// Copyright 2024-2026 Jim Kennedy\n#include <vector>\n`);
  const r = runGuard([good]);
  assert.equal(r.status, 0, `well-formed file should pass:\n${r.stdout}${r.stderr}`);
});

test('a file missing the header FAILS with exit 1 and names the file (red-on-regression)', () => {
  const dir = mkdtempSync(join(tmpdir(), 'spdx-bad-'));
  const bad = join(dir, 'bad.cpp');
  writeFileSync(bad, `#include <vector>\nint main() { return 0; }\n`);
  const r = runGuard([bad]);
  const out = `${r.stdout || ''}${r.stderr || ''}`;
  assert.equal(r.status, 1, `header-less file must fail:\n${out}`);
  assert.match(out, /bad\.cpp/, 'guard must name the offending file');
});
