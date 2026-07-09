import { test } from 'node:test';
import assert from 'node:assert/strict';

import {
  isDumpModeEnabled,
  slugifyPresetName,
} from '../src/audio/dump-mode.ts';

test('isDumpModeEnabled: dump=1 enables', () => {
  assert.equal(isDumpModeEnabled('?dump=1'), true);
  assert.equal(isDumpModeEnabled('?foo=bar&dump=1'), true);
});

test('isDumpModeEnabled: diag=1 alias enables', () => {
  assert.equal(isDumpModeEnabled('?diag=1'), true);
});

test('isDumpModeEnabled: absent / wrong value disables', () => {
  assert.equal(isDumpModeEnabled(''), false);
  assert.equal(isDumpModeEnabled('?dump=0'), false);
  assert.equal(isDumpModeEnabled('?dump=true'), false);
  assert.equal(isDumpModeEnabled('?foo=bar'), false);
});

test('isDumpModeEnabled: no window in Node returns false', () => {
  // The scheduler resolves dump-mode at start; when called from tests without
  // a `searchOverride` and no window global, it must default off so the
  // capture branch stays dead in offline suites that do not opt in.
  assert.equal(isDumpModeEnabled(), false);
});

test('slugifyPresetName: kebab-lowercase-hyphen', () => {
  assert.equal(slugifyPresetName('Four on the Floor'), 'four-on-the-floor');
  assert.equal(slugifyPresetName('Afrobeat 12/8'), 'afrobeat-12-8');
  assert.equal(slugifyPresetName('  Sparse   Pulse  '), 'sparse-pulse');
});

test('slugifyPresetName: pathological input falls back to "preset"', () => {
  assert.equal(slugifyPresetName(''), 'preset');
  assert.equal(slugifyPresetName('///'), 'preset');
});
