import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

// S11 T06: schema-shape sanity for the emitter output that both the card
// runtime and the WASM host consume. Catches shape drift (schemaVersion
// regressions, missing lanes, empty roleLabels) at Node-test speed before
// the Playwright cross-surface gate has to notice.

const HERE = dirname(fileURLToPath(import.meta.url));
const PRESETS_PATH = join(HERE, '..', 'src', 'generated', 'presets.json');

test('presets.json — schema shape', async () => {
  const raw = await readFile(PRESETS_PATH, 'utf8');
  const parsed = JSON.parse(raw);

  assert.equal(
    typeof parsed.schemaVersion,
    'number',
    'schemaVersion must be a number',
  );
  assert.ok(
    parsed.schemaVersion >= 1,
    `schemaVersion=${parsed.schemaVersion}, expected >= 1`,
  );

  assert.ok(Array.isArray(parsed.presets), 'presets must be an array');
  assert.equal(
    parsed.presets.length,
    43,
    `presets.length=${parsed.presets.length}, expected 43`,
  );

  parsed.presets.forEach((preset, pi) => {
    assert.equal(
      typeof preset.name,
      'string',
      `preset[${pi}].name is not a string`,
    );
    assert.ok(
      preset.name.length > 0,
      `preset[${pi}].name is empty`,
    );
    assert.ok(
      Number.isInteger(preset.notesInBar) && preset.notesInBar >= 1,
      `preset[${pi}] "${preset.name}".notesInBar=${preset.notesInBar}, expected integer >= 1`,
    );
    assert.ok(
      Array.isArray(preset.lanes) && preset.lanes.length >= 1,
      `preset[${pi}] "${preset.name}".lanes.length=${preset.lanes?.length}, expected >= 1`,
    );
    preset.lanes.forEach((lane, li) => {
      assert.ok(
        Number.isInteger(lane.noteNumber) &&
          lane.noteNumber >= 0 &&
          lane.noteNumber <= 127,
        `preset[${pi}] "${preset.name}" lane[${li}].noteNumber=${lane.noteNumber}, expected integer in [0,127]`,
      );
      assert.equal(
        typeof lane.roleLabel,
        'string',
        `preset[${pi}] "${preset.name}" lane[${li}].roleLabel is not a string`,
      );
      assert.ok(
        lane.roleLabel.length > 0,
        `preset[${pi}] "${preset.name}" lane[${li}].roleLabel is empty`,
      );
    });
  });
});
