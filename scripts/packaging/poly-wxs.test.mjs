// poly-wxs.test.mjs — the Windows installer's WiX source says what the
// installer does (open-source-launch M008/S01, OS37).
//
// WiX builds MSIs only on Windows, so the package-windows CI job is where the
// MSI is really built, signed, installed and removed. This test runs anywhere
// and holds the source to the decisions in M008-decisions.md: one per-machine
// MSI, installing the bundle to Common Files\VST3, recognising older Poly
// installs by a fixed UpgradeCode, with nothing that Windows Installer would
// not undo on uninstall.
//
// Run: `node --test scripts/packaging/poly-wxs.test.mjs` (check-guards.sh).

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const WXS = join(dirname(fileURLToPath(import.meta.url)), 'poly.wxs');
const UPGRADE_CODE = 'CAC811BE-A55D-452F-A377-9442360217CD';

const src = () => readFileSync(WXS, 'utf8');
const pkg = () => /<Package\b[^>]*>/.exec(src())?.[0] ?? '';

test('a WiX v4+ source for one per-machine package named Poly by jk.digital', () => {
  assert.match(src(), /xmlns="http:\/\/wixtoolset\.org\/schemas\/v4\/wxs"/, 'not a WiX v4+ source');
  const p = pkg();
  assert.ok(p, 'no <Package> element');
  assert.match(p, /\bName="Poly"/);
  assert.match(p, /\bManufacturer="jk\.digital"/);
  assert.match(p, /\bScope="perMachine"/, 'the VST3 folder is per-machine');
  assert.match(p, /\bVersion="\$\(Version\)"/, 'the version must come from the build, not the source');
});

test('the UpgradeCode is the fixed one, and a newer install replaces an older one', () => {
  assert.match(pkg(), new RegExp(`\\bUpgradeCode="${UPGRADE_CODE}"`), 'the UpgradeCode must never change');
  assert.match(src(), /<MajorUpgrade\b[^>]*\bDowngradeErrorMessage="[^"]+"/, 'no MajorUpgrade with a downgrade message');
});

test('one artifact: the cabinet is embedded in the MSI', () => {
  assert.match(src(), /<MediaTemplate\b[^>]*\bEmbedCab="yes"/);
});

test('the bundle installs to Common Files\\VST3\\poly_plugin.vst3, harvested whole', () => {
  const s = src();
  const m = /<StandardDirectory\s+Id="CommonFiles64Folder"\s*>([\s\S]*?)<\/StandardDirectory>/.exec(s);
  assert.ok(m, 'no CommonFiles64Folder');
  assert.match(m[1], /<Directory\s+Id="[^"]+"\s+Name="VST3"\s*>[\s\S]*<Directory\s+Id="INSTALLFOLDER"\s+Name="poly_plugin\.vst3"/,
    'VST3 > poly_plugin.vst3 must sit under Common Files');
  assert.match(s, /<Files\s+Include="\$\(BundleDir\)\\\*\*"\s+Directory="INSTALLFOLDER"/, 'the whole bundle must be harvested into INSTALLFOLDER');
});

test('nothing runs that Windows Installer would not undo', () => {
  assert.doesNotMatch(src(), /<CustomAction\b/, 'a custom action is code the uninstall cannot reverse');
});
