import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile, readdir } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

import { resolvePresetName } from '../src/audio/preset-patterns.ts';

const HERE = dirname(fileURLToPath(import.meta.url));
const CHAPTERS_DIR = join(HERE, '..', 'src', 'content', 'docs');

// <PolyPatch title="..." preset="..."> — the preset attribute renders only as
// an italic display sub-title (see src/components/PolyPatch.astro); it resolves
// nothing at runtime. This guard closes that conformance hole: a PolyPatch
// caption must NOT read like a loadable factory preset unless it actually is
// one. See M071 S04 T04.
const POLYPATCH_PRESET_RE = /<PolyPatch\b[^>]*\bpreset="([^"]+)"/g;

// The resolution contract for a <PolyPatch preset="X"> caption:
//   (a) X resolves via resolvePresetName(X) — it names a real, loadable factory
//       preset in presets.json (the reader can load it, typically via the
//       companion <PolyPreviewCard> that follows the patch), OR
//   (b) X is an explicit non-loadable descriptor, marked by the 'Custom:'
//       prefix — a hand-built configuration with no factory equivalent.
// Anything else is a caption that reads like a preset name but resolves to
// nothing, which S06's preset-name guardrail (P6.2) must be able to reject.
const NON_LOADABLE_PREFIX = 'Custom:';

function isContractValid(preset) {
  if (preset.startsWith(NON_LOADABLE_PREFIX)) return true;
  return resolvePresetName(preset) !== null;
}

async function collectPolyPatchPresets() {
  const files = (await readdir(CHAPTERS_DIR)).filter((f) => f.endsWith('.mdx'));
  const refs = [];
  for (const f of files) {
    const src = await readFile(join(CHAPTERS_DIR, f), 'utf8');
    const lines = src.split('\n');
    lines.forEach((line, i) => {
      for (const m of line.matchAll(POLYPATCH_PRESET_RE)) {
        refs.push({ file: f, line: i + 1, preset: m[1] });
      }
    });
  }
  return { files, refs };
}

test('every PolyPatch preset caption resolves or is an explicit Custom label', async () => {
  const { files, refs } = await collectPolyPatchPresets();
  assert.ok(files.length >= 15, `expected >= 15 chapter files, got ${files.length}`);
  assert.ok(refs.length >= 15, `expected >= 15 PolyPatch preset captions, got ${refs.length}`);

  const offenders = refs
    .filter((r) => !isContractValid(r.preset))
    .map((r) => `${r.file}:${r.line} preset="${r.preset}"`);

  assert.deepEqual(
    offenders,
    [],
    `PolyPatch preset captions that neither resolve nor carry the '${NON_LOADABLE_PREFIX}' marker:\n${offenders.join('\n')}`,
  );
});

test('the Custom: marker is reserved for genuinely non-loadable patches', async () => {
  // A caption marked 'Custom:' must NOT collide with a real factory preset name —
  // otherwise the marker would hide a resolvable preset behind the escape hatch.
  const { refs } = await collectPolyPatchPresets();
  const shadowed = refs
    .filter((r) => r.preset.startsWith(NON_LOADABLE_PREFIX))
    .filter((r) => resolvePresetName(r.preset) !== null)
    .map((r) => `${r.file}:${r.line} preset="${r.preset}"`);

  assert.deepEqual(
    shadowed,
    [],
    `Custom-marked captions that are actually resolvable factory presets:\n${shadowed.join('\n')}`,
  );
});
