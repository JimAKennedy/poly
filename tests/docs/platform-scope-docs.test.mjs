// D029 (M054 S03) docs-contract test: the user- and contributor-facing docs
// state the shipping-platform scope and cannot silently regress it.
//
// Decision D029 (M054): Poly ships as a plugin on macOS and Windows only;
// Linux is an engine/WASM-only target with no shipping VST3. This test reads
// the actual README.md, CONTRIBUTING.md, and CHANGELOG.md on disk and fails if
// the platform statement disappears, if a shippable-Linux-plugin claim
// reappears, or if the S02 CHANGELOG D029 entry is removed — so future doc
// edits cannot fork the docs from the decision.
//
// Dependency-free by design: asserts against the raw markdown text, no parser.

import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync, existsSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join, resolve } from 'node:path';

const __dirname = dirname(fileURLToPath(import.meta.url));
const repoRoot = resolve(__dirname, '..', '..');

function readDoc(name) {
  const p = join(repoRoot, name);
  assert.ok(existsSync(p), `${name} not found at ${p}`);
  return readFileSync(p, 'utf8');
}

test('README states the D029 shipping-platform scope', () => {
  const readme = readDoc('README.md');
  // Names the decision so the statement is traceable back to D029/M054.
  assert.match(readme, /\bD029\b/, 'README must reference decision D029');
  // macOS + Windows are the shipping plugin targets.
  assert.match(
    readme,
    /macOS and Windows/,
    'README must name macOS and Windows as the shipping targets',
  );
  // Linux is the engine/WASM-only target.
  assert.match(
    readme,
    /engine\/WASM-only/i,
    'README must state Linux is engine/WASM-only',
  );
});

test('CONTRIBUTING sets engine/WASM-only expectations for Linux', () => {
  const contributing = readDoc('CONTRIBUTING.md');
  assert.match(
    contributing,
    /engine\/WASM-only/i,
    'CONTRIBUTING must state the Linux build is engine/WASM-only',
  );
  // Points contributors at the engine-only configure flag.
  assert.match(
    contributing,
    /-DPOLY_ENGINE_ONLY=ON/,
    'CONTRIBUTING must direct Linux checkouts to -DPOLY_ENGINE_ONLY=ON',
  );
});

test('neither doc claims a shippable Linux plugin', () => {
  for (const name of ['README.md', 'CONTRIBUTING.md']) {
    // Collapse whitespace so line wrapping ("no\nshipping Linux VST3") does
    // not defeat the negated-form lookbehind.
    const doc = readDoc(name).replace(/\s+/g, ' ');
    // The exact "shipping Linux VST3" claim that D029 rules out — allowed only
    // in the negated form ("no shipping Linux VST3").
    const shippingVst3 = /(?<!no )shipping Linux VST3/i;
    assert.doesNotMatch(
      doc,
      shippingVst3,
      `${name} must not claim a shipping Linux VST3`,
    );
    assert.ok(
      !/\bLinux plugin\b/i.test(doc),
      `${name} must not claim a Linux plugin`,
    );
  }
});

test('CHANGELOG retains the S02 D029 shipping-target entry', () => {
  const changelog = readDoc('CHANGELOG.md');
  assert.match(
    changelog,
    /supported plugin targets are now explicitly macOS and Windows; Linux is engine\/WASM only/,
    'CHANGELOG must retain the S02 D029 shipping-target entry',
  );
  assert.match(changelog, /\bD029\b/, 'CHANGELOG D029 entry must name the decision');
});
