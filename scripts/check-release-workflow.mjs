#!/usr/bin/env node
// check-release-workflow.mjs — contract test for .github/workflows/release.yml
// (M030 S01). The release workflow cannot be exercised by a real tag push inside
// CI/the sandbox, so this committed node:test script locks its SHAPE and turns
// regressions into a red build:
//   - someone re-adding a Linux/ubuntu build leg (banned by D029/D030),
//   - dropping the macOS dual-arch (universal) flag,
//   - breaking the tag trigger (e.g. adding branch/PR triggers),
//   - renaming/mis-encoding the platform zip assets, or
//   - unwiring the release-publish job (permissions, gh-release, notes body).
//
// It ALSO proves scripts/gen-release-notes.mjs emits a non-empty body for the
// shipping CHANGELOG sections (0.1.0 / Unreleased), because that step runs at
// tag time and would otherwise fail the real Release.
//
// Node has no built-in YAML parser and this repo carries no YAML dependency, so
// assertions parse release.yml with targeted structural matches rather than a
// full YAML load — matching how the other scripts/check-*.mjs contract checks
// assert on focused structure. Run: `node --test scripts/check-release-workflow.mjs`.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { execFileSync } from 'node:child_process';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = resolve(HERE, '..');
const WF_PATH = resolve(REPO, '.github', 'workflows', 'release.yml');
const GEN = resolve(REPO, 'scripts', 'gen-release-notes.mjs');

const wf = readFileSync(WF_PATH, 'utf8');

// The build matrix legs are the ONLY lines shaped `- os: <runner>`. The release
// job selects its runner with `runs-on: ubuntu-latest`, which uses `runs-on:`,
// not `- os:`, so this cleanly isolates the shipping build legs from the
// (legitimately ubuntu) release-publish job.
function matrixLegs(src) {
  return [...src.matchAll(/^\s*-\s*os:\s*(\S+)/gm)].map((m) => m[1]);
}

test('workflow file exists and is non-empty', () => {
  assert.ok(wf.trim().length > 0, 'release.yml is empty');
});

test('triggers ONLY on push of v*.*.* tags — not branches or PRs', () => {
  // The semver tag glob must be present under on.push.tags.
  assert.match(
    wf,
    /tags:\s*\n\s*-\s*'v\*\.\*\.\*'/,
    'missing on.push.tags: v*.*.* trigger',
  );
  // Guard against accidental broadening of the trigger surface.
  assert.doesNotMatch(wf, /pull_request/, 'release must not trigger on pull_request');
  assert.doesNotMatch(
    wf,
    /^\s*branches:/m,
    'release must not trigger on branch pushes (no on.push.branches)',
  );
});

test('build matrix has exactly two shipping legs: macos-14 and windows-2022', () => {
  const legs = matrixLegs(wf);
  assert.equal(legs.length, 2, `expected 2 build legs, found ${legs.length}: ${legs}`);
  assert.deepEqual(
    [...legs].sort(),
    ['macos-14', 'windows-2022'],
    `unexpected build legs: ${legs}`,
  );
});

test('NO linux/ubuntu build leg (D029/D030: Poly ships no Linux VST3)', () => {
  const legs = matrixLegs(wf);
  const banned = legs.filter((os) => /ubuntu|linux/i.test(os));
  assert.equal(banned.length, 0, `banned Linux build leg present: ${banned}`);
});

test('macOS leg builds a UNIVERSAL binary (both arches, quoted so ; survives)', () => {
  // Anchor to the cmake-args LINE, not just anywhere in the file — the header
  // comment also mentions the flag, so a file-wide match would pass even if the
  // real build arg dropped it.
  assert.match(
    wf,
    /cmake-args:.*CMAKE_OSX_ARCHITECTURES="arm64;x86_64"/,
    'macOS leg must set -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" on its cmake-args (quotes load-bearing)',
  );
});

test('assets are named to encode platform (macos-universal / windows-x64 zips)', () => {
  assert.match(wf, /asset:\s*macos-universal/, 'missing macos-universal asset label');
  assert.match(wf, /asset:\s*windows-x64/, 'missing windows-x64 asset label');
  // Each packaging step names the zip poly-<tag>-<asset>.zip via matrix.asset.
  assert.match(
    wf,
    /poly-\$\{GITHUB_REF_NAME\}-\$\{\{\s*matrix\.asset\s*\}\}\.zip/,
    'macOS packaging must name the zip poly-<ref>-<asset>.zip',
  );
  assert.match(
    wf,
    /poly-\$env:GITHUB_REF_NAME-\$\{\{\s*matrix\.asset\s*\}\}\.zip/,
    'Windows packaging must name the zip poly-<ref>-<asset>.zip',
  );
});

test('least-privilege permissions: default read, release job elevates to write', () => {
  assert.match(wf, /permissions:\s*\n\s*contents:\s*read/, 'missing default contents: read');
  assert.match(wf, /permissions:\s*\n\s*contents:\s*write/, 'release job must elevate to contents: write');
});

test('release job is wired: needs build, generates notes, publishes via gh-release', () => {
  assert.match(wf, /needs:\s*build/, 'release job must depend on the build job');
  assert.match(
    wf,
    /node scripts\/gen-release-notes\.mjs/,
    'release job must generate notes from CHANGELOG via gen-release-notes.mjs',
  );
  assert.match(wf, /softprops\/action-gh-release/, 'release job must publish via softprops/action-gh-release');
  assert.match(wf, /body_path:\s*release-notes\.md/, 'gh-release must use the generated notes as body');
  assert.match(wf, /fail_on_unmatched_files:\s*true/, 'gh-release must fail if no zip assets matched');
});

// --- S02 gate: VST3 validator + pluginval must run on the exact release
// binaries BEFORE any zip asset is packaged. These lock the gate against a
// future edit silently dropping the validator flag, unpinning pluginval,
// weakening strictness, or reordering pluginval after packaging. ---

// Every build-leg's cmake invocation is on a `cmake-args:` line (the header
// comment mentions the flag too, so match the arg LINES, not the whole file).
function cmakeArgsLines(src) {
  return [...src.matchAll(/^\s*cmake-args:.*/gm)].map((m) => m[0]);
}

// File-order index of the first step whose `name:` matches the pattern (-1 if
// absent). Used to assert pluginval precedes packaging in the leg.
function stepIndex(src, pattern) {
  const m = src.match(new RegExp(`^\\s*-\\s*name:\\s*${pattern}.*$`, 'm'));
  return m ? m.index : -1;
}

test('BOTH build legs pass -DSMTG_RUN_VST_VALIDATOR=ON to cmake', () => {
  const lines = cmakeArgsLines(wf);
  assert.equal(lines.length, 2, `expected 2 cmake-args lines, found ${lines.length}`);
  for (const line of lines) {
    assert.match(
      line,
      /-DSMTG_RUN_VST_VALIDATOR=ON/,
      `cmake-args leg missing SMTG_RUN_VST_VALIDATOR=ON: ${line.trim()}`,
    );
  }
});

test('pluginval runs on BOTH legs (macOS + Windows steps present)', () => {
  const legs = [...wf.matchAll(/^\s*-\s*name:\s*Run pluginval/gm)];
  assert.equal(legs.length, 2, `expected 2 "Run pluginval" steps, found ${legs.length}`);
  assert.match(wf, /name:\s*Run pluginval \(macOS\)/, 'missing macOS pluginval step');
  assert.match(wf, /name:\s*Run pluginval \(Windows\)/, 'missing Windows pluginval step');
});

test('pluginval is PINNED to v1.0.4 (no floating releases/latest)', () => {
  assert.match(
    wf,
    /releases\/download\/v1\.0\.4\/pluginval_macOS\.zip/,
    'macOS pluginval must download the pinned v1.0.4 macOS zip',
  );
  assert.match(
    wf,
    /releases\/download\/v1\.0\.4\/pluginval_Windows\.zip/,
    'Windows pluginval must download the pinned v1.0.4 Windows zip',
  );
  assert.doesNotMatch(
    wf,
    /pluginval\/releases\/latest/,
    'pluginval must be pinned to v1.0.4, not releases/latest',
  );
});

test('pluginval strictness/flags are locked on BOTH legs (level 8, skip-gui, 120s)', () => {
  const strictness = [...wf.matchAll(/--strictness-level\s+8\b/g)];
  assert.equal(strictness.length, 2, `expected --strictness-level 8 on both legs, found ${strictness.length}`);
  const skipGui = [...wf.matchAll(/--skip-gui-tests\b/g)];
  assert.equal(skipGui.length, 2, `expected --skip-gui-tests on both legs, found ${skipGui.length}`);
  const timeout = [...wf.matchAll(/--timeout-ms\s+120000\b/g)];
  assert.equal(timeout.length, 2, `expected --timeout-ms 120000 on both legs, found ${timeout.length}`);
});

test('pluginval step PRECEDES packaging on both legs (validate before we ship)', () => {
  const pvMac = stepIndex(wf, 'Run pluginval \\(macOS\\)');
  const pkgMac = stepIndex(wf, 'Locate and package VST3 \\(macOS\\)');
  assert.ok(pvMac >= 0, 'missing macOS pluginval step');
  assert.ok(pkgMac >= 0, 'missing macOS packaging step');
  assert.ok(pvMac < pkgMac, 'macOS pluginval must run BEFORE packaging (else unvalidated zips ship)');

  const pvWin = stepIndex(wf, 'Run pluginval \\(Windows\\)');
  const pkgWin = stepIndex(wf, 'Locate and package VST3 \\(Windows\\)');
  assert.ok(pvWin >= 0, 'missing Windows pluginval step');
  assert.ok(pkgWin >= 0, 'missing Windows packaging step');
  assert.ok(pvWin < pkgWin, 'Windows pluginval must run BEFORE packaging (else unvalidated zips ship)');
});

// --- S03 gate: macOS Developer ID codesign + Apple notarization + stapling.
// These steps run AFTER pluginval and BEFORE packaging, and each is gated on its
// signing secrets being non-empty so absent secrets SKIP (never fail) the step
// and the leg still ships an unsigned zip. These assertions lock that shape
// against a future edit dropping the hardened-runtime flag, un-gating the secret
// `if:`, removing notarytool --wait, dropping stapling, or reordering signing
// after packaging. (M030 S03, D031.) ---

test('signing secrets are mapped to job-level env (step if: reads env, not secrets)', () => {
  // GitHub does not expose the `secrets` context in step `if:` expressions —
  // only `env` — so the gated steps require these job-level env mappings to
  // exist for their `env.MACOS_* != ''` gates to resolve.
  for (const name of [
    'MACOS_CERTIFICATE_P12_BASE64',
    'MACOS_CERTIFICATE_PASSWORD',
    'MACOS_SIGNING_IDENTITY',
    'MACOS_NOTARY_APPLE_ID',
    'MACOS_NOTARY_PASSWORD',
    'MACOS_NOTARY_TEAM_ID',
  ]) {
    assert.match(
      wf,
      new RegExp(`${name}:\\s*\\$\\{\\{\\s*secrets\\.${name}\\s*\\}\\}`),
      `signing secret ${name} must be mapped to job-level env from the secrets context`,
    );
  }
});

test('codesign uses the hardened runtime + secure timestamp (both required to notarize)', () => {
  assert.match(
    wf,
    /codesign\b[^\n]*--options runtime/,
    'codesign must pass --options runtime (hardened runtime is a notarization prerequisite)',
  );
  assert.match(
    wf,
    /codesign\b[^\n]*--timestamp/,
    'codesign must pass --timestamp (secure Apple timestamp is a notarization prerequisite)',
  );
});

test('codesign step is GATED on the signing-cert secrets (absent -> skip, not fail)', () => {
  assert.match(
    wf,
    /if:\s*runner\.os == 'macOS' && env\.MACOS_CERTIFICATE_P12_BASE64 != '' && env\.MACOS_SIGNING_IDENTITY != ''/,
    'Codesign step must gate on env.MACOS_CERTIFICATE_P12_BASE64 and env.MACOS_SIGNING_IDENTITY being non-empty',
  );
});

test('notarize runs notarytool submit --wait (fails the leg on a non-Accepted status)', () => {
  assert.match(wf, /notarytool submit/, 'missing xcrun notarytool submit');
  assert.match(
    wf,
    /notarytool submit[\s\S]*?--wait/,
    'notarytool submit must use --wait so a rejected submission fails the leg instead of shipping',
  );
});

test('notarize + staple steps are GATED on the notary secrets (absent -> skip, not fail)', () => {
  const notaryGate =
    /if:\s*runner\.os == 'macOS' && env\.MACOS_NOTARY_APPLE_ID != '' && env\.MACOS_NOTARY_PASSWORD != '' && env\.MACOS_NOTARY_TEAM_ID != ''/g;
  const gates = [...wf.matchAll(notaryGate)];
  assert.equal(
    gates.length,
    2,
    `expected the notary-secret gate on BOTH the notarize and staple steps, found ${gates.length}`,
  );
});

test('notarization ticket is stapled INTO the bundle (offline Gatekeeper acceptance)', () => {
  assert.match(wf, /stapler staple/, 'missing xcrun stapler staple');
  assert.match(wf, /stapler validate/, 'missing xcrun stapler validate');
});

test('signing runs AFTER pluginval and BEFORE packaging on the macOS leg', () => {
  const pvMac = stepIndex(wf, 'Run pluginval \\(macOS\\)');
  const codesign = stepIndex(wf, 'Codesign VST3 \\(macOS\\)');
  const notarize = stepIndex(wf, 'Notarize VST3 \\(macOS\\)');
  const staple = stepIndex(wf, 'Staple notarization ticket \\(macOS\\)');
  const pkgMac = stepIndex(wf, 'Locate and package VST3 \\(macOS\\)');
  for (const [label, idx] of [
    ['pluginval', pvMac],
    ['codesign', codesign],
    ['notarize', notarize],
    ['staple', staple],
    ['package', pkgMac],
  ]) {
    assert.ok(idx >= 0, `missing macOS ${label} step`);
  }
  assert.ok(pvMac < codesign, 'codesign must run AFTER pluginval');
  assert.ok(codesign < notarize, 'notarize must run AFTER codesign (only a signed bundle can be notarized)');
  assert.ok(notarize < staple, 'staple must run AFTER notarize (it staples the notarization ticket)');
  assert.ok(
    staple < pkgMac,
    'codesign/notarize/staple must all run BEFORE packaging (else the zipped asset ships unsigned)',
  );
});

// --- S04 gate: Windows zip VST3 directory layout. The Windows packaging step
// must Compress-Archive the .vst3 bundle DIRECTORY (via $vst3.FullName) so the
// archive keeps poly_plugin.vst3/ as its top-level entry — mirroring the macOS
// leg's `ditto -c -k --keepParent`, which also roots the bundle at the zip. A
// future edit that globs the bundle contents (`-Path "$vst3.FullName/*"`) or
// switches to a flat file list would silently ship a zip that extracts loose
// files instead of poly_plugin.vst3/, breaking install with no CI signal.
// (M030 S04.) ---

test('Windows packaging locates the .vst3 as a DIRECTORY (a bundle, not a stray file)', () => {
  // A .vst3 bundle is a directory; Get-ChildItem -Directory guarantees $vst3 is
  // that directory (so $vst3.FullName below is the bundle dir, not a file).
  assert.match(
    wf,
    /Get-ChildItem[^\n]*-Directory[^\n]*-Filter \*\.vst3/,
    'Windows packaging must find the .vst3 bundle with Get-ChildItem -Directory -Filter *.vst3',
  );
});

test('Windows packaging zips the bundle DIRECTORY ($vst3.FullName), keeping poly_plugin.vst3/ at the zip root', () => {
  assert.match(
    wf,
    /Compress-Archive -Path \$vst3\.FullName -DestinationPath \$asset/,
    'Windows packaging must Compress-Archive -Path $vst3.FullName (the bundle dir) so poly_plugin.vst3/ stays the archive top-level entry (mirrors macOS ditto --keepParent)',
  );
});

test('Windows packaging does NOT glob the bundle contents (a flattened archive breaks install)', () => {
  // `-Path "$vst3.FullName/*"` (or `...\*`) would zip the bundle's CONTENTS loose
  // at the archive root instead of the poly_plugin.vst3/ directory, so extracting
  // it would NOT yield the bundle path a DAW loads. Ban that shape outright.
  assert.doesNotMatch(
    wf,
    /Compress-Archive[^\n]*\$vst3\.FullName[\\/]\*/,
    'Windows packaging must not glob $vst3.FullName/* — that flattens the archive (ships loose files, not poly_plugin.vst3/)',
  );
});

// --- gen-release-notes.mjs behaviour (runs at tag time; must not fail loud) ---

test('gen-release-notes emits a non-empty body for shipping version 0.1.0', () => {
  // The workflow strips the leading v (v0.1.0 -> 0.1.0) before invoking this.
  const out = execFileSync('node', [GEN, '0.1.0'], { encoding: 'utf8' });
  assert.ok(out.trim().length > 0, 'gen-release-notes produced an empty body for 0.1.0');
});

test('gen-release-notes emits a non-empty body for Unreleased', () => {
  const out = execFileSync('node', [GEN, 'Unreleased'], { encoding: 'utf8' });
  assert.ok(out.trim().length > 0, 'gen-release-notes produced an empty body for Unreleased');
});

test('gen-release-notes fails loud (exit 1) on a missing CHANGELOG section', () => {
  // A mistagged/undocumented version must halt the release, not publish an empty body.
  assert.throws(
    () => execFileSync('node', [GEN, '99.99.99'], { encoding: 'utf8', stdio: 'pipe' }),
    (err) => err.status === 1,
    'expected exit 1 for an absent CHANGELOG section',
  );
});

test('gen-release-notes rejects a missing version argument (exit 2)', () => {
  assert.throws(
    () => execFileSync('node', [GEN], { encoding: 'utf8', stdio: 'pipe' }),
    (err) => err.status === 2,
    'expected exit 2 when no version argument is given',
  );
});
