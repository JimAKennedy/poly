// M031 S04 T02: proof for the WarningsAsErrors-gated clang-tidy CI gate.
//
// Two things must hold and stay held (anti-drift seam, MEM110):
//
//   A. WIRING — the shared runner (scripts/run-clang-tidy-changed.sh) exists, is
//      executable, carries its region markers, is invoked BY NAME in ci.yml, and
//      the `clang-tidy` job is referenced in BOTH ci-complete's `needs:` list and
//      its pass-check body, so a red gate blocks merge. `.clang-tidy` (the single
//      source of truth for the gated set) stays tracked. If a future edit adds the
//      job without wiring it into ci-complete — or drops the runner — these fail
//      long before CI runs.
//
//   B. BEHAVIOR — the gate is not a rubber stamp. run-clang-tidy-changed.behavior.test.sh
//      drives the real runner: a clean file exits 0, a modernize-use-nullptr defect
//      exits 1 with a file:line:check location. That check is one of the 8 gated
//      WarningsAsErrors checks, so proving it fires proves the gate enforces the set.
//      The .sh skips (exit 77) when no clang-tidy binary is present, so this stays
//      green on LLVM-less machines; CI installs clang-tidy and exercises it for real.
//
// Dependency-free by design: tests/ci has no package.json, so we assert against the
// raw workflow text (matching ci-linux-engine-only.test.mjs) rather than a YAML dep.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync, existsSync, statSync } from 'node:fs';
import { spawnSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import { dirname, join, resolve } from 'node:path';
import { constants as fsConstants, accessSync } from 'node:fs';

const __dirname = dirname(fileURLToPath(import.meta.url));
const repoRoot = resolve(__dirname, '..', '..');
const ciPath = join(repoRoot, '.github', 'workflows', 'ci.yml');
const runnerPath = join(repoRoot, 'scripts', 'run-clang-tidy-changed.sh');
const clangTidyConfig = join(repoRoot, '.clang-tidy');
const behaviorTest = join(__dirname, 'run-clang-tidy-changed.behavior.test.sh');

const ci = () => {
  assert.ok(existsSync(ciPath), `ci.yml not found at ${ciPath}`);
  return readFileSync(ciPath, 'utf8');
};

// --- A. Wiring ---------------------------------------------------------------

test('shared runner exists, is executable, and carries its region markers', () => {
  assert.ok(existsSync(runnerPath), `runner not found at ${runnerPath}`);
  // Executable bit (any-exec) — the CI job and callers invoke it as a script.
  assert.doesNotThrow(
    () => accessSync(runnerPath, fsConstants.X_OK),
    'runner must be executable',
  );
  assert.ok(statSync(runnerPath).isFile(), 'runner must be a regular file');
  const src = readFileSync(runnerPath, 'utf8');
  assert.match(src, /# region:clang-tidy-runner/, 'runner must open its CodeSnippet region');
  assert.match(src, /# endregion:clang-tidy-runner/, 'runner must close its CodeSnippet region');
});

test('.clang-tidy config (single source of truth for gated set) stays tracked', () => {
  assert.ok(existsSync(clangTidyConfig), '.clang-tidy must remain in the repo');
  const cfg = readFileSync(clangTidyConfig, 'utf8');
  assert.match(cfg, /^WarningsAsErrors:/m, '.clang-tidy must keep its WarningsAsErrors key');
});

test('ci.yml defines a clang-tidy job that invokes the runner by name', () => {
  const src = ci();
  assert.match(src, /^\s{2}clang-tidy:/m, 'a top-level clang-tidy job must be defined');
  assert.match(
    src,
    /scripts\/run-clang-tidy-changed\.sh/,
    'the clang-tidy job must invoke scripts/run-clang-tidy-changed.sh by name',
  );
  // The job must produce a compile DB for tidy (engine-only, export compile commands).
  assert.match(src, /-DCMAKE_EXPORT_COMPILE_COMMANDS=ON/, 'job must export a compile DB for clang-tidy');
});

test('ci-complete needs list AND pass-check both gate on clang-tidy', () => {
  const src = ci();
  // needs: [...] for ci-complete must list clang-tidy.
  const needsM = src.match(/ci-complete:\s*\n\s*needs:\s*\[([^\]]*)\]/);
  assert.ok(needsM, 'ci-complete needs list not found');
  const needs = needsM[1].split(',').map((s) => s.trim()).filter(Boolean);
  assert.ok(
    needs.includes('clang-tidy'),
    'ci-complete needs list must include clang-tidy so the gate is required',
  );
  // The pass-check body must evaluate needs.clang-tidy.result — a red gate blocks.
  assert.match(
    src,
    /\$\{\{\s*needs\.clang-tidy\.result\s*\}\}"?\s*!=\s*"success"/,
    "ci-complete pass-check must fail when needs.clang-tidy.result != 'success'",
  );
});

// --- B. Behavior (drives the real runner via the .sh proof) ------------------

test('behavior test proves green-on-clean and red-on-nullptr-defect', (t) => {
  assert.ok(existsSync(behaviorTest), `behavior test not found at ${behaviorTest}`);
  const r = spawnSync('bash', [behaviorTest], { cwd: repoRoot, encoding: 'utf8' });
  const out = `${r.stdout || ''}${r.stderr || ''}`;
  if (r.status === 77) {
    // No clang-tidy binary on this host — the .sh self-skips. Wiring assertions
    // above still ran; the behavior proof runs for real in CI (clang-tidy installed).
    t.skip(`clang-tidy unavailable — behavior proof skipped:\n${out}`);
    return;
  }
  assert.equal(r.status, 0, `behavior test failed (exit ${r.status}):\n${out}`);
  assert.match(out, /GREEN case passed/, 'behavior test must confirm the clean-file GREEN case');
  assert.match(out, /RED case passed/, 'behavior test must confirm the defect RED case');
});
