// M031 S06: proof for the site-README template-boilerplate OSS-maturity guard.
//
// Two things must hold and stay held (anti-drift seam, MEM110):
//
//   A. WIRING — scripts/check-site-readme.sh exists, is executable, and is
//      invoked BY NAME in ci.yml so template boilerplate creeping back fails the
//      build. If a future edit drops the guard or unwires it from CI, these fail
//      long before CI runs. (The CI wiring lands in S06 T03; until then that one
//      assertion is expected red — every behavior assertion below passes now.)
//
//   B. BEHAVIOR — the guard is not a rubber stamp:
//        * run over the real tree (no args) exits 0 — the three site docs are
//          project-specific today and the README carries the project marker
//          (green-now);
//        * a doc containing any Starlight/Astro template phrase
//          ("Starlight Starter Kit", "Seasoned astronaut", "npm create astro",
//          "docs.astro.build") fails with exit 1 and names the offending file
//          (red-on-regression);
//        * the framework names "Starlight"/"Astro" and the real
//          starlight.astro.build link are NOT flagged (guard forbids template
//          phrases, not the framework);
//        * a project-specific doc with none of the forbidden phrases passes.
//
// Dependency-free by design: tests/ci and scripts/ carry no package.json, so we
// assert against the raw workflow text (matching check-personal-paths.mjs)
// rather than pulling in a YAML dependency. Fixtures are written to a tmp dir so
// they are never git-tracked and never scanned by the CI-mode run.
//
// Run: `node --test scripts/check-site-readme.mjs`

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
const guardPath = join(repoRoot, 'scripts', 'check-site-readme.sh');

const runGuard = (args) =>
  spawnSync('bash', [guardPath, ...args], { cwd: repoRoot, encoding: 'utf8' });

const writeFixture = (prefix, body) => {
  const dir = mkdtempSync(join(tmpdir(), prefix));
  const file = join(dir, 'fixture.md');
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

test('ci.yml invokes the site-readme guard by name so boilerplate blocks the build', () => {
  assert.ok(existsSync(ciPath), `ci.yml not found at ${ciPath}`);
  const ci = readFileSync(ciPath, 'utf8');
  assert.match(
    ci,
    /scripts\/check-site-readme\.sh/,
    'ci.yml must invoke scripts/check-site-readme.sh',
  );
});

// --- B. Behavior -------------------------------------------------------------

test('real tree is clean — guard exits 0 with no args (green-now)', () => {
  const r = runGuard([]);
  const out = `${r.stdout || ''}${r.stderr || ''}`;
  assert.equal(r.status, 0, `guard should pass on the real site docs:\n${out}`);
  assert.match(out, /no template boilerplate/, 'guard should report the clean scan');
});

test('"Starlight Starter Kit" boilerplate FAILS with exit 1 and names the file', () => {
  const bad = writeFixture('sr-title-', '# Starlight Starter Kit: Basics\n');
  const r = runGuard([bad]);
  const out = `${r.stdout || ''}${r.stderr || ''}`;
  assert.equal(r.status, 1, `template title must fail:\n${out}`);
  assert.match(out, /fixture\.md/, 'guard must name the offending file');
});

test('"Seasoned astronaut" boilerplate FAILS with exit 1 (red-on-regression)', () => {
  const bad = writeFixture('sr-astro-', 'Seasoned astronaut? Delete this file. Have fun!\n');
  const r = runGuard([bad]);
  assert.equal(r.status, 1, `template prose must fail:\n${r.stdout}${r.stderr}`);
});

test('"npm create astro" scaffold command FAILS with exit 1 (red-on-regression)', () => {
  const bad = writeFixture('sr-cmd-', 'npm create astro@latest -- --template starlight\n');
  const r = runGuard([bad]);
  assert.equal(r.status, 1, `scaffold command must fail:\n${r.stdout}${r.stderr}`);
});

test('"docs.astro.build" stub link FAILS with exit 1 (red-on-regression)', () => {
  const bad = writeFixture('sr-link-', 'Full documentation: https://docs.astro.build\n');
  const r = runGuard([bad]);
  assert.equal(r.status, 1, `stub doc link must fail:\n${r.stdout}${r.stderr}`);
});

test('legitimate framework mentions are NOT flagged', () => {
  // Mirrors the real README: names the framework and links its own site, with
  // none of the forbidden template phrases.
  const ok = writeFixture(
    'sr-ok-',
    [
      '# Poly Guide Site',
      'The source for poly.jk.digital.',
      'This is a [Starlight](https://starlight.astro.build/) (Astro) documentation site.',
      '',
    ].join('\n'),
  );
  const r = runGuard([ok]);
  const out = `${r.stdout || ''}${r.stderr || ''}`;
  assert.equal(r.status, 0, `project-specific doc must pass:\n${out}`);
});
