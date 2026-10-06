// build-macos-pkg.test.mjs — the macOS installer package installs the VST3
// where a DAW looks, offers a per-user install, and carries nothing else
// (open-source-launch M007/S01, OS35).
//
// The package is built from a fixture bundle and taken apart with
// `pkgutil --expand`, never installed: installing needs sudo and changes the
// machine, which is the CI job's work (test-macos-pkg.sh). What this proves is
// what the installer will do — identifier, version, install location, the
// domain choice, that the bundle is not relocatable, and that the payload is
// the bundle and only the bundle.
//
// Run: `node --test scripts/packaging/build-macos-pkg.test.mjs`
// (check-guards.sh, and the package-macos CI job). macOS only: pkgbuild and
// productbuild exist nowhere else, so other platforms skip with that reason.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { mkdtempSync, mkdirSync, writeFileSync, readFileSync, readdirSync, rmSync, existsSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const SCRIPT = join(HERE, 'build-macos-pkg.sh');
const MACOS = process.platform === 'darwin';
const SKIP = MACOS ? false : 'pkgbuild and productbuild are macOS-only';

const INFO_PLIST = `<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
<key>CFBundleIdentifier</key><string>digital.jk.poly.fixture</string>
<key>CFBundleName</key><string>poly_plugin</string>
<key>CFBundlePackageType</key><string>BNDL</string>
</dict></plist>
`;

function run(cmd, args, opts = {}) {
  return spawnSync(cmd, args, { encoding: 'utf8', ...opts });
}

function buildFixture() {
  const work = mkdtempSync(join(tmpdir(), 'poly-pkg-'));
  const bundle = join(work, 'poly_plugin.vst3');
  mkdirSync(join(bundle, 'Contents'), { recursive: true });
  writeFileSync(join(bundle, 'Contents', 'Info.plist'), INFO_PLIST);
  // A downloaded or freshly built file can carry quarantine; the package must
  // never pass it on, or the installer would re-quarantine what it installs.
  if (MACOS) run('xattr', ['-w', 'com.apple.quarantine', '0081;00000000;Safari;', join(bundle, 'Contents', 'Info.plist')]);
  return { work, bundle };
}

test('the package installs the VST3 to the system VST3 folder, or per-user, and nothing else', { skip: SKIP }, () => {
  const { work, bundle } = buildFixture();
  try {
    const out = join(work, 'poly.pkg');
    const r = run('bash', [SCRIPT, bundle, '9.9.9', out]);
    assert.equal(r.status, 0, `build failed: ${r.stderr}`);
    assert.ok(existsSync(out), 'no package written');

    const expanded = join(work, 'expanded');
    const x = run('pkgutil', ['--expand', out, expanded]);
    assert.equal(x.status, 0, x.stderr);

    const dist = readFileSync(join(expanded, 'Distribution'), 'utf8');
    assert.match(dist, /<title>Poly<\/title>/);
    assert.match(dist, /enable_localSystem="true"/, 'no all-users install');
    assert.match(dist, /enable_currentUserHome="true"/, 'no per-user install');
    assert.match(dist, /enable_anywhere="false"/, 'the installer must not offer an arbitrary volume');

    const components = readdirSync(expanded).filter((f) => f.endsWith('.pkg'));
    assert.equal(components.length, 1, `expected one component package, got ${components.join(', ')}`);
    const info = readFileSync(join(expanded, components[0], 'PackageInfo'), 'utf8');
    assert.match(info, /identifier="digital\.jk\.poly\.vst3"/);
    assert.match(info, /version="9\.9\.9"/);
    assert.match(info, /install-location="\/Library\/Audio\/Plug-Ins\/VST3"/);
    assert.doesNotMatch(info, /<relocate>/, 'the bundle must not be relocatable');

    const payload = run('tar', ['-tf', join(expanded, components[0], 'Payload')]);
    assert.equal(payload.status, 0, payload.stderr);
    // `._name` entries are AppleDouble metadata: how pkgbuild carries a file's
    // extended attributes, restored by the installer as attributes, not files.
    // macOS stamps com.apple.provenance on every file a process writes and no
    // process can remove it, so they are expected; quarantine is not (below).
    const files = payload.stdout
      .split('\n')
      .filter(Boolean)
      .filter((p) => p !== '.' && p !== './')
      .filter((p) => !p.split('/').pop().startsWith('._'));
    assert.ok(files.includes('./poly_plugin.vst3/Contents/Info.plist'), `payload lacks the bundle: ${files.join(', ')}`);
    const stray = files.filter((p) => !p.startsWith('./poly_plugin.vst3'));
    assert.deepEqual(stray, [], 'the payload carries something other than the bundle');
    assert.ok(!files.some((p) => p.endsWith('.component')), 'no AU ships (OS17 declined it)');

    const extracted = join(work, 'extracted');
    mkdirSync(extracted);
    const ex = run('tar', ['-xf', join(expanded, components[0], 'Payload'), '-C', extracted]);
    assert.equal(ex.status, 0, ex.stderr);
    const attrs = run('xattr', ['-lr', extracted]);
    assert.doesNotMatch(attrs.stdout, /com\.apple\.quarantine/, 'the payload carries a quarantine attribute');
  } finally {
    rmSync(work, { recursive: true, force: true });
  }
});

test('a missing bundle or an empty version fails with a reason', { skip: SKIP }, () => {
  const { work, bundle } = buildFixture();
  try {
    const missing = run('bash', [SCRIPT, join(work, 'nope.vst3'), '1.0.0', join(work, 'a.pkg')]);
    assert.notEqual(missing.status, 0);
    assert.match(missing.stderr, /bundle/i);
    const noVersion = run('bash', [SCRIPT, bundle, '', join(work, 'b.pkg')]);
    assert.notEqual(noVersion.status, 0);
    assert.match(noVersion.stderr, /version/i);
  } finally {
    rmSync(work, { recursive: true, force: true });
  }
});
