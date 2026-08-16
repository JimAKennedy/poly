// M031 S05 T03: proof for the personal-machine-path OSS-maturity guard.
//
// Two things must hold and stay held (anti-drift seam, MEM110):
//
//   A. WIRING — scripts/check-personal-paths.sh exists, is executable, and is
//      invoked BY NAME in ci.yml so a committed personal path fails the build.
//      If a future edit drops the guard or unwires it from CI, these fail long
//      before CI runs.
//
//   B. BEHAVIOR — the guard is not a rubber stamp:
//        * run over the real tree (no args) exits 0 — no tracked file carries a
//          non-allowlisted personal path today (green-now);
//        * a file embedding a real personal path (/Users/<name>/…, /home/<name>/…,
//          C:\Users\<name>\…) fails with exit 1 and names the offending file
//          (red-on-regression);
//        * the portable `~` home abbreviation is NOT flagged;
//        * each allowlisted benign form (freesound URL, the polyci CI account,
//          the /Users/jim/... ellipsis rationale) passes.
//
// Dependency-free by design: tests/ci and scripts/ carry no package.json, so we
// assert against the raw workflow text (matching check-spdx-headers.mjs) rather
// than pulling in a YAML dependency. Fixtures are written to a tmp dir so they
// are never git-tracked and never scanned by the CI-mode run.
//
// Run: `node --test scripts/check-personal-paths.mjs`

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
const guardPath = join(repoRoot, 'scripts', 'check-personal-paths.sh');

const runGuard = (args) =>
  spawnSync('bash', [guardPath, ...args], { cwd: repoRoot, encoding: 'utf8' });

// Assemble personal-path literals from fragments so this proof file does not
// itself carry a scannable personal path (it is excluded from CI-mode anyway,
// but this keeps the source honest under any future ad-hoc grep).
const U = 'Users';
const macPath = `/${U}/alice/dev/secret-project`;
const linuxPath = `/home/bob/.config/creds`;
const winPath = `C:\\${U}\\carol\\AppData`;

const writeFixture = (prefix, body) => {
  const dir = mkdtempSync(join(tmpdir(), prefix));
  const file = join(dir, 'fixture.txt');
  writeFileSync(file, body);
  return file;
};

// --- A. Wiring ---------------------------------------------------------------

test('guard script exists, is executable, and is a regular file', () => {
  assert.ok(existsSync(guardPath), `guard not found at ${guardPath}`);
  assert.ok(statSync(guardPath).isFile(), 'guard must be a regular file');
  assert.doesNotThrow(
    () => accessSync(guardPath, fsConstants.X_OK),
    'guard must be executable',
  );
});

test('ci.yml invokes the personal-path guard by name so a leak blocks the build', () => {
  assert.ok(existsSync(ciPath), `ci.yml not found at ${ciPath}`);
  const ci = readFileSync(ciPath, 'utf8');
  assert.match(
    ci,
    /scripts\/check-personal-paths\.sh/,
    'ci.yml must invoke scripts/check-personal-paths.sh',
  );
});

// --- B. Behavior -------------------------------------------------------------

test('real tree is clean — guard exits 0 with no args (green-now)', () => {
  const r = runGuard([]);
  const out = `${r.stdout || ''}${r.stderr || ''}`;
  assert.equal(r.status, 0, `guard should pass on the real tree:\n${out}`);
  assert.match(out, /no personal-machine paths/, 'guard should report the clean scan');
});

test('a macOS personal path FAILS with exit 1 and names the file (red-on-regression)', () => {
  const bad = writeFixture('pp-mac-', `notes\n${macPath}\nmore\n`);
  const r = runGuard([bad]);
  const out = `${r.stdout || ''}${r.stderr || ''}`;
  assert.equal(r.status, 1, `macOS personal path must fail:\n${out}`);
  assert.match(out, /fixture\.txt/, 'guard must name the offending file');
});

test('a Linux personal path FAILS with exit 1 (red-on-regression)', () => {
  const bad = writeFixture('pp-linux-', `${linuxPath}\n`);
  const r = runGuard([bad]);
  assert.equal(r.status, 1, `linux personal path must fail:\n${r.stdout}${r.stderr}`);
});

test('a Windows personal path FAILS with exit 1 (red-on-regression)', () => {
  const bad = writeFixture('pp-win-', `${winPath}\n`);
  const r = runGuard([bad]);
  assert.equal(r.status, 1, `windows personal path must fail:\n${r.stdout}${r.stderr}`);
});

test('the portable ~ home abbreviation is NOT flagged', () => {
  const ok = writeFixture('pp-tilde-', `install to ~/Library/Audio and ~/.cache\n`);
  const r = runGuard([ok]);
  assert.equal(r.status, 0, `~ abbreviation must pass:\n${r.stdout}${r.stderr}`);
});

test('allowlisted benign forms all PASS', () => {
  // Exact strings mirroring the three real tracked hits the allowlist exempts.
  const benign = writeFixture(
    'pp-allow-',
    [
      'see freesound.org/home/attribution/ for the list',
      'Out-File C:\\Users\\polyci\\cdp-probe.txt',
      'see /Users/jim/... prefixes. Not portable',
      '',
    ].join('\n'),
  );
  const r = runGuard([benign]);
  const out = `${r.stdout || ''}${r.stderr || ''}`;
  assert.equal(r.status, 0, `allowlisted benign forms must pass:\n${out}`);
});

test('a real /Users/jim path (no ellipsis) still FAILS despite the ellipsis allowlist', () => {
  const bad = writeFixture('pp-jim-', `/Users/jim/dev/poly/build\n`);
  const r = runGuard([bad]);
  assert.equal(r.status, 1, `a concrete /Users/jim path must still fail:\n${r.stdout}${r.stderr}`);
});
