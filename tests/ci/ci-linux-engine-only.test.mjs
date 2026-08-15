// D029 (M054 S02) contract test: the CI workflow pins Linux to an engine-only
// compile and carries no Linux plugin-validation surface.
//
// Poly ships no Linux VST3 (the native VSTGUI editor was removed in M053 S05),
// so the ubuntu-latest `build` matrix leg builds the portable engine only
// (-DPOLY_ENGINE_ONLY=ON, no VST3 SDK) and the `pluginval-linux` job — which
// validated a full-but-editorless Linux plugin that shipped to nobody — is
// gone. This test is the always-on assertion of that shape: if a future edit
// re-adds a Linux plugin build/validation, or leaves the aggregation gate
// pointing at a deleted job, it fails here long before CI runs.
//
// Dependency-free by design: tests/ci has no package.json, so we assert against
// the raw workflow text rather than pulling in a YAML parser (T03 does the
// separate `node yaml load` parse-validity check).

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync, existsSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join, resolve } from 'node:path';

const __dirname = dirname(fileURLToPath(import.meta.url));
const repoRoot = resolve(__dirname, '..', '..');
const ciPath = join(repoRoot, '.github', 'workflows', 'ci.yml');

function loadCi() {
  assert.ok(existsSync(ciPath), `ci.yml not found at ${ciPath}`);
  return readFileSync(ciPath, 'utf8');
}

// Extract the cmake-args line for the ubuntu-latest entry in the `build`
// matrix. The matrix `include` lists `- os: <name>` immediately followed by a
// `cmake-args:` line for each leg.
function ubuntuBuildArgs(ci) {
  const re = /- os:\s*ubuntu-latest\s*\n\s*cmake-args:\s*(.+)/;
  const m = ci.match(re);
  assert.ok(m, 'no ubuntu-latest build matrix leg with cmake-args found');
  return m[1].trim();
}

test('ubuntu-latest build leg configures engine-only', () => {
  const args = ubuntuBuildArgs(loadCi());
  assert.match(args, /-DPOLY_ENGINE_ONLY=ON/, 'Linux build leg must set POLY_ENGINE_ONLY=ON');
  assert.match(args, /-DCMAKE_BUILD_TYPE=Release/, 'Linux build leg must be a Release build');
  assert.match(args, /-G Ninja/, 'Linux build leg must use the Ninja generator');
  assert.doesNotMatch(
    args,
    /SMTG_RUN_VST_VALIDATOR/,
    'engine-only build has no plugin — SMTG_RUN_VST_VALIDATOR must be dropped',
  );
});

test('Linux build leg installs only engine deps (no UI libs)', () => {
  const ci = loadCi();
  // The Linux dependency install step under the `build` job.
  const m = ci.match(/Install dependencies \(Linux\)[\s\S]*?run:\s*(sudo apt-get install[^\n]*)/);
  assert.ok(m, 'no Linux dependency install step found in build job');
  const installLine = m[1];
  assert.match(installLine, /ninja-build/, 'engine build still needs Ninja');
  // No VSTGUI/GTK/X11 UI libraries — those existed only for the editor build.
  for (const uiLib of ['libgtkmm', 'libx11', 'libxcb', 'libcairo', 'libfontconfig', 'libxkbcommon']) {
    assert.ok(
      !installLine.includes(uiLib),
      `engine-only Linux install must not pull UI lib "${uiLib}"`,
    );
  }
});

test('pluginval-linux job is fully removed', () => {
  const ci = loadCi();
  // No job definition. Job keys are two-space-indented top-level map entries.
  assert.doesNotMatch(ci, /^\s{2}pluginval-linux:/m, 'pluginval-linux job must not be defined');
  // No reference anywhere (needs list, result check, artifact name).
  assert.ok(!ci.includes('pluginval-linux'), 'no reference to pluginval-linux may remain');
  // The sibling plugin-validation jobs for the shipping targets stay.
  assert.match(ci, /^\s{2}pluginval-macos:/m, 'pluginval-macos must remain');
  assert.match(ci, /^\s{2}pluginval-windows:/m, 'pluginval-windows must remain');
});

test('ci-complete aggregation gate references no undefined job', () => {
  const ci = loadCi();

  // Collect the defined job names (2-space-indented keys under `jobs:`).
  const jobNames = new Set();
  for (const line of ci.split('\n')) {
    const m = line.match(/^ {2}([A-Za-z][\w-]*):\s*$/);
    if (m) jobNames.add(m[1]);
  }
  assert.ok(jobNames.has('ci-complete'), 'ci-complete job must exist');
  assert.ok(!jobNames.has('pluginval-linux'), 'pluginval-linux must not be a defined job');

  // Every entry in ci-complete's `needs: [...]` list must be a defined job.
  const needsM = ci.match(/ci-complete:\s*\n\s*needs:\s*\[([^\]]*)\]/);
  assert.ok(needsM, 'ci-complete needs list not found');
  const needs = needsM[1].split(',').map((s) => s.trim()).filter(Boolean);
  assert.ok(!needs.includes('pluginval-linux'), 'pluginval-linux must be gone from needs list');
  for (const dep of needs) {
    assert.ok(jobNames.has(dep), `ci-complete needs undefined job "${dep}"`);
  }

  // Every live `${{ needs.<job>.result }}` check in the gate body must resolve
  // to a real job. The `${{ }}` wrapper distinguishes an evaluated expression
  // from historical prose in job comments (e.g. a "needs.pluginval.result"
  // reference in the M048 S13 note above).
  const resultRe = /\$\{\{\s*needs\.([\w-]+)\.result/g;
  let rm;
  while ((rm = resultRe.exec(ci)) !== null) {
    assert.ok(jobNames.has(rm[1]), `gate checks result of undefined job "${rm[1]}"`);
  }
});
