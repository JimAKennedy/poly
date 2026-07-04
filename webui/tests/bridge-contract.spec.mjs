import { test, expect } from '@playwright/test';
import { createRequire } from 'node:module';
import { setupWithActionLog, getEdits, clearEdits, pageUrl } from './test-helpers.mjs';

const require = createRequire(import.meta.url);
const Ajv = require('ajv');

const schema = require('../bridge.schema.json');
const editFixtures = require('./fixtures/edits.json');
const actionFixtures = require('./fixtures/actions.json');
const stateInitFixture = require('./fixtures/state-init.json');
const frameFixture = require('./fixtures/frame.json');

function makeValidator() {
  const ajv = new Ajv({ allErrors: true });
  return ajv.compile(schema);
}

test.describe('bridge schema validation', () => {
  let validate;
  test.beforeAll(() => { validate = makeValidator(); });

  test('edit fixture messages validate against schema', () => {
    for (const msg of editFixtures.messages) {
      const { label, ...m } = msg;
      expect(validate(m), `edit "${label}": ${JSON.stringify(validate.errors)}`).toBe(true);
    }
  });

  test('action fixture messages validate against schema', () => {
    for (const msg of actionFixtures.messages) {
      const { label, ...m } = msg;
      expect(validate(m), `action "${label}": ${JSON.stringify(validate.errors)}`).toBe(true);
    }
  });

  test('state-init fixture validates against schema', () => {
    const r = validate({ type: stateInitFixture.type, state: stateInitFixture.state });
    expect(r, JSON.stringify(validate.errors)).toBe(true);
  });

  test('frame fixture validates against schema', () => {
    const r = validate({ type: frameFixture.type, frame: frameFixture.frame });
    expect(r, JSON.stringify(validate.errors)).toBe(true);
  });
});

test.describe('outbound edit contract', () => {
  let validate;
  test.beforeAll(() => { validate = makeValidator(); });

  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
  });

  test('macro slider drag produces schema-valid edit message', async ({ page }) => {
    await page.evaluate(() => {
      window.PolyHost.edit('macro.complexity', 0.75, 'begin');
      window.PolyHost.edit('macro.complexity', 0.75, 'perform');
      window.PolyHost.edit('macro.complexity', 0.75, 'end');
    });
    const edits = await getEdits(page);
    expect(edits.length).toBe(3);
    for (const e of edits) {
      const msg = { type: 'edit', v: 1, ...e };
      expect(validate(msg), `edit schema fail: ${JSON.stringify(validate.errors)}`).toBe(true);
    }
    expect(edits[0].gesture).toBe('begin');
    expect(edits[1].gesture).toBe('perform');
    expect(edits[2].gesture).toBe('end');
    expect(edits[1].paramId).toBe('macro.complexity');
    expect(edits[1].value).toBe(0.75);
  });

  test('lane parameter edit produces schema-valid message', async ({ page }) => {
    await page.evaluate(() => {
      window.PolyHost.edit('lane.0.velocity', 0.5, 'perform');
    });
    const edits = await getEdits(page);
    expect(edits.length).toBe(1);
    const msg = { type: 'edit', v: 1, ...edits[0] };
    expect(validate(msg), JSON.stringify(validate.errors)).toBe(true);
    expect(edits[0].paramId).toBe('lane.0.velocity');
  });

  test('chain parameter edit produces schema-valid message', async ({ page }) => {
    await page.evaluate(() => {
      window.PolyHost.edit('chain.enabled', 1.0, 'perform');
    });
    const edits = await getEdits(page);
    expect(edits.length).toBe(1);
    const msg = { type: 'edit', v: 1, ...edits[0] };
    expect(validate(msg), JSON.stringify(validate.errors)).toBe(true);
  });
});

test.describe('outbound action contract', () => {
  let validate;
  test.beforeAll(() => { validate = makeValidator(); });

  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
  });

  test('toggleStep action has correct shape', async ({ page }) => {
    const actions = await page.evaluate(() => {
      window.PolyHost.action('toggleStep', { lane: 1, step: 0 });
      return window.__actionLog.slice();
    });
    expect(actions.length).toBe(1);
    expect(actions[0].name).toBe('toggleStep');
    expect(actions[0].payload).toEqual({ lane: 1, step: 0 });
    const msg = { type: 'action', v: 1, ...actions[0] };
    expect(validate(msg), JSON.stringify(validate.errors)).toBe(true);
  });

  test('setEuclid action has correct shape', async ({ page }) => {
    const actions = await page.evaluate(() => {
      window.PolyHost.action('setEuclid', { lane: 1, steps: 8, hits: 5, rotation: 2 });
      return window.__actionLog.slice();
    });
    expect(actions.length).toBe(1);
    expect(actions[0].name).toBe('setEuclid');
    expect(actions[0].payload.lane).toBe(1);
    expect(actions[0].payload.steps).toBe(8);
    expect(actions[0].payload.hits).toBe(5);
    const msg = { type: 'action', v: 1, ...actions[0] };
    expect(validate(msg), JSON.stringify(validate.errors)).toBe(true);
  });

  test('applyPreset action has correct shape', async ({ page }) => {
    const actions = await page.evaluate(() => {
      window.PolyHost.action('applyPreset', { index: 2 });
      return window.__actionLog.slice();
    });
    expect(actions.length).toBe(1);
    expect(actions[0].name).toBe('applyPreset');
    expect(actions[0].payload.index).toBe(2);
    const msg = { type: 'action', v: 1, ...actions[0] };
    expect(validate(msg), JSON.stringify(validate.errors)).toBe(true);
  });

  test('chainAddEntry/chainRemoveEntry actions have correct shape', async ({ page }) => {
    const actions = await page.evaluate(() => {
      window.PolyHost.action('chainAddEntry', {});
      window.PolyHost.action('chainRemoveEntry', { index: 0 });
      return window.__actionLog.slice();
    });
    expect(actions.length).toBe(2);
    expect(actions[0].name).toBe('chainAddEntry');
    expect(actions[1].name).toBe('chainRemoveEntry');
    expect(actions[1].payload.index).toBe(0);
    for (const a of actions) {
      const msg = { type: 'action', v: 1, ...a };
      expect(validate(msg), JSON.stringify(validate.errors)).toBe(true);
    }
  });

  test('setAccent action has correct shape', async ({ page }) => {
    const actions = await page.evaluate(() => {
      window.PolyHost.action('setAccent', { lane: 0, step: 2, value: 0.8 });
      return window.__actionLog.slice();
    });
    expect(actions.length).toBe(1);
    expect(actions[0].payload).toEqual({ lane: 0, step: 2, value: 0.8 });
    const msg = { type: 'action', v: 1, ...actions[0] };
    expect(validate(msg), JSON.stringify(validate.errors)).toBe(true);
  });

  test('noteMap actions have correct shape', async ({ page }) => {
    const actions = await page.evaluate(() => {
      window.PolyHost.action('setNoteMap', { note: 36, output: 48 });
      window.PolyHost.action('resetNoteMap', {});
      return window.__actionLog.slice();
    });
    expect(actions.length).toBe(2);
    expect(actions[0].name).toBe('setNoteMap');
    expect(actions[0].payload.note).toBe(36);
    expect(actions[1].name).toBe('resetNoteMap');
    for (const a of actions) {
      const msg = { type: 'action', v: 1, ...a };
      expect(validate(msg), JSON.stringify(validate.errors)).toBe(true);
    }
  });
});

test.describe('inbound state contract', () => {
  let validate;
  test.beforeAll(() => { validate = makeValidator(); });

  test.beforeEach(async ({ page }) => {
    await page.goto(pageUrl);
  });

  test('mock-host default state validates against schema', async ({ page }) => {
    const state = await page.evaluate(() => window.PolyMockHost.getState());
    const msg = { type: 'state', state };
    expect(validate(msg), `default state: ${JSON.stringify(validate.errors?.slice(0, 5))}`).toBe(true);
  });

  test('every factory preset state validates against schema', async ({ page }) => {
    const presetCount = await page.evaluate(() => window.PolyMockHost.getState().presets.length);
    for (let i = 0; i < presetCount; i++) {
      await page.evaluate((idx) => window.PolyMockHost.action('applyPreset', { index: idx }), i);
      const state = await page.evaluate(() => window.PolyMockHost.getState());
      const msg = { type: 'state', state };
      expect(validate(msg), `preset ${i} (${state.preset}): ${JSON.stringify(validate.errors?.slice(0, 3))}`).toBe(true);
    }
  });

  test('init preset state validates against schema', async ({ page }) => {
    await page.evaluate(() => window.PolyMockHost.action('applyPreset', { index: -1 }));
    const state = await page.evaluate(() => window.PolyMockHost.getState());
    const msg = { type: 'state', state };
    expect(validate(msg), `init: ${JSON.stringify(validate.errors?.slice(0, 3))}`).toBe(true);
    expect(state.preset).toBe('Init');
    expect(state.macros.complexity).toBe(0);
  });

  test('state after setEuclid validates against schema', async ({ page }) => {
    await page.evaluate(() => {
      window.PolyMockHost.action('setEuclid', { lane: 1, steps: 8, hits: 5, rotation: 2 });
    });
    const state = await page.evaluate(() => window.PolyMockHost.getState());
    const msg = { type: 'state', state };
    expect(validate(msg), JSON.stringify(validate.errors?.slice(0, 3))).toBe(true);
    expect(state.lanes[1].steps).toBe(8);
    expect(state.lanes[1].hits).toBe(5);
    expect(state.lanes[1].rot).toBe(2);
  });

  test('state after chain operations validates against schema', async ({ page }) => {
    await page.evaluate(() => {
      window.PolyMockHost.action('chainAddEntry', {});
      window.PolyMockHost.action('chainAddEntry', {});
    });
    const state = await page.evaluate(() => window.PolyMockHost.getState());
    const msg = { type: 'state', state };
    expect(validate(msg), JSON.stringify(validate.errors?.slice(0, 3))).toBe(true);
    expect(state.chain.entryCount).toBe(2);
    expect(state.chain.entries.length).toBe(2);
  });
});

test.describe('canned state push renders UI correctly', () => {
  test.beforeEach(async ({ page }) => {
    await page.goto(pageUrl);
  });

  async function pushCannedState(page, stateObj) {
    await page.evaluate((s) => {
      const host = window.PolyMockHost;
      const cur = host.getState();
      Object.keys(s).forEach(k => { cur[k] = s[k]; });
      host._pushState();
    }, stateObj);
  }

  test('pushing init state fixture renders 4 lanes', async ({ page }) => {
    await pushCannedState(page, stateInitFixture.state);
    await expect(page.locator('.strip')).toHaveCount(4);
    await expect(page.locator('#presetName')).toContainText('Init');
  });

  test('pushing state with modified macros renders correct values', async ({ page }) => {
    const modified = JSON.parse(JSON.stringify(stateInitFixture.state));
    modified.macros.complexity = 0.8;
    modified.macros.density = 0.6;
    await pushCannedState(page, modified);
    const complexity = page.locator('.macro', { has: page.locator('.t span', { hasText: 'Complexity' }) });
    await expect(complexity.locator('.t b')).toHaveText('80');
    const density = page.locator('.macro', { has: page.locator('.t span', { hasText: 'Density' }) });
    await expect(density.locator('.t b')).toHaveText('60');
  });

  test('pushing state with 5 lanes renders correct lane count and names', async ({ page }) => {
    const fiveLanes = JSON.parse(JSON.stringify(stateInitFixture.state));
    fiveLanes.lanes.push({
      name: 'Conga', role: 'Ghost', note: 63, ch: 5, steps: 5,
      subdivision: 8, stepLen: 1, vel: 65, prob: 0.8, spread: 0, ghost: 25,
      push: 0, hits: 3, rot: 0, timeline: false, active: true,
      pattern: [1, 0, 1, 0, 1], fixed: null, cells: null, cellCount: 0,
      mt: [0, 0, 0, 0, 0], envs: [], accents: [0, 0, 0, 0, 0],
      humanize: 0, swing: 0, duration: 0.5, emphasisProb: 0, timingOffset: 0,
      mutationRate: 0, driftRate: 0, phraseLength: 0, phraseGap: 0, phraseOffset: 0,
      tempoMultiplier: 1.0, kotekanSource: -1, hue: '#B48AE0'
    });
    await pushCannedState(page, fiveLanes);
    await expect(page.locator('.strip')).toHaveCount(5);
    await expect(page.locator('.strip[data-lane="4"] .head .nm b')).toContainText('Conga');
  });

  test('pushing frame message does not crash', async ({ page }) => {
    await page.evaluate((fixture) => {
      window.polyHostPush(JSON.stringify({ type: fixture.type, frame: fixture.frame }));
    }, frameFixture);
    await page.waitForTimeout(50);
    const strips = await page.locator('.strip').count();
    expect(strips).toBeGreaterThan(0);
  });
});

test.describe('plugin-host outbound message schema validation', () => {
  let validate;
  test.beforeAll(() => { validate = makeValidator(); });

  test('plugin-host send() wraps messages with v and type', async ({ page }) => {
    await page.goto(pageUrl);
    const captured = await page.evaluate(() => {
      const msgs = [];
      window.polyHostCall = (json) => msgs.push(JSON.parse(json));
      window.__POLY_EMBEDDED__ = true;

      const ph = window.PolyPluginHost;
      ph.edit('macro.complexity', 0.5, 'perform');
      ph.action('toggleStep', { lane: 0, step: 0 });
      return msgs;
    });
    expect(captured.length).toBe(2);
    for (const msg of captured) {
      expect(msg.v).toBe(1);
      expect(validate(msg), JSON.stringify(validate.errors)).toBe(true);
    }
    expect(captured[0].type).toBe('edit');
    expect(captured[1].type).toBe('action');
  });
});
