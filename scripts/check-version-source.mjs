#!/usr/bin/env node
// check-version-source.mjs — the shipped version comes from one place.
//
// open-source-launch M001/S01, OS01. Five version strings disagreed: CMake said
// 0.1.0, plugids.h told every DAW 1.0.0, the two npm manifests carried numbers
// nobody published, and the AU plist a development sentinel. The plugin now
// takes its version from `project(poly VERSION …)` through the SDK's
// projectversion.h; this guard fails if a hand-typed version returns to
// plugids.h or a private npm manifest grows a version again. The AU plist is
// OS17's, because it matters only if the AU ships.
//
// Run: `node --test scripts/check-version-source.mjs` (wired into check-guards.sh).
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const REPO = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const read = (p) => readFileSync(resolve(REPO, p), 'utf8');

test('plugids.h takes its version from projectversion.h, not a literal', () => {
  const src = read('plugin/source/plugids.h');
  const literal = src.match(/kPolyVersionString\s*=\s*"(\d+\.\d+\.\d+)"/);
  assert.equal(
    literal,
    null,
    `plugids.h hand-types the version ${literal?.[1]} — it must be FULL_VERSION_STR from projectversion.h, ` +
      'so a DAW reports the version CMake built',
  );
  assert.match(src, /#include\s+"projectversion\.h"/, 'plugids.h does not include projectversion.h');
  assert.match(src, /kPolyVersionString\s*=\s*FULL_VERSION_STR/, 'kPolyVersionString is not FULL_VERSION_STR');
});

test('CMakeLists.txt declares the project version, the one place it is typed', () => {
  const m = read('CMakeLists.txt').match(/project\s*\(\s*poly[^)]*\bVERSION\s+(\d+\.\d+\.\d+)/s);
  assert.ok(m, 'CMakeLists.txt has no project(poly … VERSION x.y.z)');
});

for (const manifest of ['webui/package.json', 'site/package.json']) {
  test(`${manifest} is private and carries no version`, () => {
    const pkg = JSON.parse(read(manifest));
    assert.equal(pkg.private, true, `${manifest} must be "private": true — it is never published`);
    assert.equal(
      pkg.version,
      undefined,
      `${manifest} carries "version": "${pkg.version}" — a number nobody publishes and one more place to drift`,
    );
  });
}
