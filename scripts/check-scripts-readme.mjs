// Proof for the scripts-inventory README sync guard.
//
// Two things must hold and stay held (same anti-drift seam as
// check-site-readme.mjs / check-personal-paths.mjs):
//
//   A. WIRING — scripts/check-scripts-readme.sh exists, is executable, and is
//      invoked BY NAME in ci.yml, so a README that drifts from the directory
//      fails the build. If a future edit drops the guard or unwires it from
//      CI, these fail long before CI runs.
//
//   B. BEHAVIOR — the guard is not a rubber stamp:
//        * run over the real tree (no args) exits 0 — scripts/README.md lists
//          every top-level script and subdirectory today (green-now);
//        * a script present on disk but absent from the README fails with
//          exit 1 and names the script (red-on-regression, coverage arm);
//        * a subdirectory absent from the README fails the same way;
//        * a README entry naming a script that does not exist fails with
//          exit 1 and names the stale entry (red-on-regression, stale arm);
//        * backticked tokens that are not bare script/dir names (paths with
//          interior slashes, other file types) are NOT flagged;
//        * a missing README is itself a failure, not a silent pass.
//
// Dependency-free by design: scripts/ carries no package.json, so we assert
// against the raw workflow text rather than pulling in a YAML dependency.
// Fixtures are written to a tmp dir so they are never git-tracked and never
// scanned by the CI-mode run.
//
// Run: `node --test scripts/check-scripts-readme.mjs`

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync, existsSync, writeFileSync, mkdtempSync, mkdirSync, statSync } from 'node:fs';
import { spawnSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import { dirname, join, resolve } from 'node:path';
import { tmpdir } from 'node:os';
import { constants as fsConstants, accessSync } from 'node:fs';

const __dirname = dirname(fileURLToPath(import.meta.url));
const repoRoot = resolve(__dirname, '..');
const ciPath = join(repoRoot, '.github', 'workflows', 'ci.yml');
const guardPath = join(repoRoot, 'scripts', 'check-scripts-readme.sh');

const runGuard = (args) =>
  spawnSync('bash', [guardPath, ...args], { cwd: repoRoot, encoding: 'utf8' });

// Build a fixture scripts dir: `files` are created empty at the top level,
// `dirs` as empty subdirectories, and `readme` (when non-null) becomes
// README.md. Returns [scriptsDir, readmePath].
const makeFixture = (prefix, { files = [], dirs = [], readme = null }) => {
  const dir = mkdtempSync(join(tmpdir(), prefix));
  for (const f of files) writeFileSync(join(dir, f), '');
  for (const d of dirs) mkdirSync(join(dir, d));
  const readmePath = join(dir, 'README.md');
  if (readme !== null) writeFileSync(readmePath, readme);
  return [dir, readmePath];
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

test('ci.yml invokes the scripts-readme guard by name so README drift blocks the build', () => {
  assert.ok(existsSync(ciPath), `ci.yml not found at ${ciPath}`);
  const ci = readFileSync(ciPath, 'utf8');
  assert.match(
    ci,
    /scripts\/check-scripts-readme\.sh/,
    'ci.yml must invoke scripts/check-scripts-readme.sh',
  );
});

// --- B. Behavior -------------------------------------------------------------

test('real tree is clean — guard exits 0 with no args (green-now)', () => {
  const r = runGuard([]);
  const out = `${r.stdout || ''}${r.stderr || ''}`;
  assert.equal(r.status, 0, `guard should pass on the real scripts/README.md:\n${out}`);
  assert.match(out, /no stale README entries/, 'guard should report the clean scan');
});

test('fully-listed fixture passes — files and subdir all have entries', () => {
  const [dir, readme] = makeFixture('sr-ok-', {
    files: ['alpha.sh', 'beta.mjs'],
    dirs: ['tools'],
    readme: '- `alpha.sh` — a\n- `beta.mjs` — b\n- `tools/` — helpers\n',
  });
  const r = runGuard([dir, readme]);
  const out = `${r.stdout || ''}${r.stderr || ''}`;
  assert.equal(r.status, 0, `in-sync fixture must pass:\n${out}`);
});

test('script on disk but missing from README FAILS and names the script (coverage arm)', () => {
  const [dir, readme] = makeFixture('sr-miss-', {
    files: ['alpha.sh', 'orphan.sh'],
    readme: '- `alpha.sh` — a\n',
  });
  const r = runGuard([dir, readme]);
  const out = `${r.stdout || ''}${r.stderr || ''}`;
  assert.equal(r.status, 1, `unlisted script must fail:\n${out}`);
  assert.match(out, /orphan\.sh/, 'guard must name the unlisted script');
});

test('subdirectory missing from README FAILS and names it (coverage arm)', () => {
  const [dir, readme] = makeFixture('sr-dir-', {
    files: ['alpha.sh'],
    dirs: ['winstuff'],
    readme: '- `alpha.sh` — a\n',
  });
  const r = runGuard([dir, readme]);
  const out = `${r.stdout || ''}${r.stderr || ''}`;
  assert.equal(r.status, 1, `unlisted subdirectory must fail:\n${out}`);
  assert.match(out, /winstuff/, 'guard must name the unlisted subdirectory');
});

test('README entry for a deleted script FAILS and names the stale entry (stale arm)', () => {
  const [dir, readme] = makeFixture('sr-stale-', {
    files: ['alpha.sh'],
    readme: '- `alpha.sh` — a\n- `ghost.sh` — deleted long ago\n',
  });
  const r = runGuard([dir, readme]);
  const out = `${r.stdout || ''}${r.stderr || ''}`;
  assert.equal(r.status, 1, `stale entry must fail:\n${out}`);
  assert.match(out, /ghost\.sh/, 'guard must name the stale entry');
});

test('non-script backticked tokens are NOT flagged (paths, other file types)', () => {
  const [dir, readme] = makeFixture('sr-tok-', {
    files: ['alpha.sh'],
    readme: [
      '- `alpha.sh` — wired into `ci.yml`, reads `webui/bridge.schema.json`,',
      '  emits `poly_engine.{js,wasm}`; see `lib/ensure-emsdk.sh` and',
      '  `cubase/README.md` for details. Run `pre-commit install -t pre-push`.',
      '',
    ].join('\n'),
  });
  const r = runGuard([dir, readme]);
  const out = `${r.stdout || ''}${r.stderr || ''}`;
  assert.equal(r.status, 0, `non-script tokens must be ignored:\n${out}`);
});

test('missing README is a failure, not a silent pass', () => {
  const [dir, readme] = makeFixture('sr-nordme-', { files: ['alpha.sh'] });
  const r = runGuard([dir, readme]);
  const out = `${r.stdout || ''}${r.stderr || ''}`;
  assert.equal(r.status, 1, `missing README must fail:\n${out}`);
  assert.match(out, /README not found/, 'guard must say the README is missing');
});
