// AU bundle-identity smoke test (host-independent, no Xcode required).
//
// The Steinberg AUv2 wrapper (wired in S03-T01) re-exposes Poly's VST3 as an
// `aumu` AudioUnit. The wrapper's AUWrapperFactory reads the plugin's identity
// — the four-char OSType codes and the factory function — from the
// `AudioComponents` array in plugin/resource/au-info.plist. That identity is
// exactly what `auval -v aumu Poly JkDl` (S03-T03 CI job) loads and validates.
//
// This test is the always-on, committed assertion of that contract: if the
// plist identity drifts from the `aumu Poly JkDl` tokens the CI auval depends
// on, this fails on Linux/dev-without-Xcode long before the live auval would.
// When a built .component is present (POLY_AU_COMPONENT env or a build dir),
// it also asserts the bundle structure and that the bundle's embedded
// Info.plist carries the same identity.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync, existsSync, statSync, readdirSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join, resolve } from 'node:path';

const __dirname = dirname(fileURLToPath(import.meta.url));
const repoRoot = resolve(__dirname, '..', '..');
const plistPath = join(repoRoot, 'plugin', 'resource', 'au-info.plist');

// The single source of truth for what the AU must advertise. These are the
// same tokens as the T03 CI invocation `auval -v aumu Poly JkDl` and the
// SDK factory entry point.
const EXPECTED = {
  type: 'aumu', // music device / instrument
  subtype: 'Poly',
  manufacturer: 'JkDl',
  factoryFunction: 'AUWrapperFactory',
};

// Minimal dependency-free plist reader: extracts the first <dict> inside the
// AudioComponents <array> and returns its <key>/<string|integer> pairs. There
// is no root package.json, so we cannot pull in a plist library.
function parseAudioComponent(xml) {
  const acKey = xml.indexOf('<key>AudioComponents</key>');
  assert.notEqual(acKey, -1, 'plist has no AudioComponents key');
  const arrStart = xml.indexOf('<array>', acKey);
  const arrEnd = xml.indexOf('</array>', arrStart);
  assert.ok(arrStart !== -1 && arrEnd !== -1, 'AudioComponents array is malformed');
  const arr = xml.slice(arrStart, arrEnd);

  const dictStart = arr.indexOf('<dict>');
  const dictEnd = arr.indexOf('</dict>');
  assert.ok(dictStart !== -1 && dictEnd !== -1, 'AudioComponents has no dict entry');
  const dict = arr.slice(dictStart + '<dict>'.length, dictEnd);

  const out = {};
  const pairRe = /<key>([^<]+)<\/key>\s*<(string|integer)>([^<]*)<\/\2>/g;
  let m;
  while ((m = pairRe.exec(dict)) !== null) {
    out[m[1]] = m[3];
  }
  return out;
}

function loadPlistIdentity() {
  assert.ok(existsSync(plistPath), `au-info.plist not found at ${plistPath}`);
  const xml = readFileSync(plistPath, 'utf8');
  // Parity with the T01 negative grep: an AU identity plist must never carry
  // the VST3 marker string.
  assert.ok(!xml.includes('VST3 Plug-in'), 'au-info.plist must not contain "VST3 Plug-in"');
  return { xml, comp: parseAudioComponent(xml) };
}

test('au-info.plist advertises the aumu Poly JkDl identity auval loads', () => {
  const { comp } = loadPlistIdentity();
  assert.equal(comp.type, EXPECTED.type, 'AU type must be aumu (music device)');
  assert.equal(comp.subtype, EXPECTED.subtype, 'AU subtype must match auval arg');
  assert.equal(comp.manufacturer, EXPECTED.manufacturer, 'AU manufacturer must match auval arg');
  assert.equal(
    comp.factoryFunction,
    EXPECTED.factoryFunction,
    'factoryFunction must be the Steinberg AU wrapper entry point',
  );
  assert.ok(comp.name && comp.name.length > 0, 'AudioComponent name must be non-empty');
});

// Negative surface: OSType codes are exactly four ASCII bytes. A truncated,
// padded, or mistyped code (e.g. "aum", "aumuu", non-ASCII) is silently
// accepted by the plist XML but rejected by the real AU host — auval would
// simply fail to find the component. Assert the four-byte contract here.
test('AU OSType codes are exactly four ASCII characters', () => {
  const { comp } = loadPlistIdentity();
  for (const key of ['type', 'subtype', 'manufacturer']) {
    const code = comp[key];
    assert.equal(typeof code, 'string', `${key} missing from AudioComponent`);
    assert.equal(code.length, 4, `${key} "${code}" must be exactly 4 chars`);
    assert.match(code, /^[\x20-\x7e]{4}$/, `${key} "${code}" must be 4 printable ASCII bytes`);
  }
});

// Locate a built .component without requiring one. Order: explicit env, then a
// shallow scan of likely build dirs. Returns null when nothing is built (the
// normal case on Linux/dev-without-Xcode), so the bundle test skips cleanly.
function findBuiltComponent() {
  const envPath = process.env.POLY_AU_COMPONENT;
  if (envPath) {
    return existsSync(envPath) ? envPath : null;
  }
  for (const dir of ['build-au', 'build', 'build-au-cfgtest']) {
    const root = join(repoRoot, dir);
    if (!existsSync(root)) continue;
    const hit = shallowFindComponent(root, 4);
    if (hit) return hit;
  }
  return null;
}

function shallowFindComponent(dir, depth) {
  if (depth < 0) return null;
  let entries;
  try {
    entries = readdirSync(dir, { withFileTypes: true });
  } catch {
    return null;
  }
  for (const e of entries) {
    if (!e.isDirectory()) continue;
    const full = join(dir, e.name);
    if (e.name.endsWith('.component')) return full;
    const nested = shallowFindComponent(full, depth - 1);
    if (nested) return nested;
  }
  return null;
}

test('built .component bundle structure matches the plist identity', (t) => {
  const bundle = findBuiltComponent();
  if (!bundle) {
    t.skip('no built .component present (expected without an Xcode AU build)');
    return;
  }

  assert.ok(statSync(bundle).isDirectory(), '.component must be a bundle directory');
  const contents = join(bundle, 'Contents');
  const infoPlist = join(contents, 'Info.plist');
  const macos = join(contents, 'MacOS');
  assert.ok(existsSync(infoPlist), 'bundle is missing Contents/Info.plist');
  assert.ok(existsSync(macos) && statSync(macos).isDirectory(), 'bundle is missing Contents/MacOS');

  // The macho executable exists and is non-empty.
  const bins = readdirSync(macos).filter((f) => statSync(join(macos, f)).size > 0);
  assert.ok(bins.length > 0, 'bundle Contents/MacOS has no executable');

  // The bundle's embedded Info.plist must carry the same identity as the
  // source template (after CMake substitution the AudioComponents dict is
  // copied verbatim).
  const embedded = parseAudioComponent(readFileSync(infoPlist, 'utf8'));
  assert.equal(embedded.type, EXPECTED.type, 'built bundle type drifted from source plist');
  assert.equal(embedded.subtype, EXPECTED.subtype, 'built bundle subtype drifted from source plist');
  assert.equal(
    embedded.manufacturer,
    EXPECTED.manufacturer,
    'built bundle manufacturer drifted from source plist',
  );
});
