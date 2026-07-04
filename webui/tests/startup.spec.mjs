import { test, expect } from '@playwright/test';
import { pageUrl, setupWithActionLog } from './test-helpers.mjs';

// ---------------------------------------------------------------------------
// Startup & initialization tests
//
// These verify the assumptions that broke when the plugin opened empty in
// Cubase: host selection, ready signaling, state push parsing, field
// completeness, and the boot sequence under both standalone and embedded modes.
// ---------------------------------------------------------------------------

test.describe('host selection (standalone)', () => {
  test('PolyHost is PolyMockHost when not embedded', async ({ page }) => {
    await page.goto(pageUrl);
    const isMock = await page.evaluate(() => window.PolyHost === window.PolyMockHost);
    expect(isMock).toBe(true);
  });

  test('PolyHost is NOT PolyPluginHost when not embedded', async ({ page }) => {
    await page.goto(pageUrl);
    const isPlugin = await page.evaluate(() => window.PolyHost === window.PolyPluginHost);
    expect(isPlugin).toBe(false);
  });

  test('__POLY_EMBEDDED__ is falsy in standalone mode', async ({ page }) => {
    await page.goto(pageUrl);
    const embedded = await page.evaluate(() => !!window.__POLY_EMBEDDED__);
    expect(embedded).toBe(false);
  });
});

test.describe('host selection (embedded)', () => {
  test('PolyHost is PolyPluginHost when __POLY_EMBEDDED__ is set before load', async ({ page }) => {
    await page.addInitScript(() => { window.__POLY_EMBEDDED__ = true; });
    await page.goto(pageUrl);
    const isPlugin = await page.evaluate(() => window.PolyHost === window.PolyPluginHost);
    expect(isPlugin).toBe(true);
  });

  test('PolyHost is NOT PolyMockHost when embedded', async ({ page }) => {
    await page.addInitScript(() => { window.__POLY_EMBEDDED__ = true; });
    await page.goto(pageUrl);
    const isMock = await page.evaluate(() => window.PolyHost === window.PolyMockHost);
    expect(isMock).toBe(false);
  });
});

test.describe('schema version', () => {
  test('POLY_SCHEMA_VERSION is 1', async ({ page }) => {
    await page.goto(pageUrl);
    const ver = await page.evaluate(() => window.POLY_SCHEMA_VERSION);
    expect(ver).toBe(1);
  });

  test('PolyPluginHost.schemaVersion matches POLY_SCHEMA_VERSION', async ({ page }) => {
    await page.goto(pageUrl);
    const match = await page.evaluate(
      () => window.PolyPluginHost.schemaVersion === window.POLY_SCHEMA_VERSION
    );
    expect(match).toBe(true);
  });
});

test.describe('ready signal (embedded)', () => {
  test('sends {type:"ready"} when __POLY_EMBEDDED__ is set', async ({ page }) => {
    await page.addInitScript(() => {
      window.__POLY_EMBEDDED__ = true;
      window.__readyCalls = [];
      window.polyHostCall = (json) => { window.__readyCalls.push(JSON.parse(json)); };
    });
    await page.goto(pageUrl);
    const calls = await page.evaluate(() => window.__readyCalls);
    const readyMsg = calls.find((m) => m.type === 'ready');
    expect(readyMsg).toBeDefined();
    expect(readyMsg.v).toBe(1);
  });

  test('does NOT send ready when not embedded', async ({ page }) => {
    await page.addInitScript(() => {
      window.__readyCalls = [];
      window.polyHostCall = (json) => { window.__readyCalls.push(JSON.parse(json)); };
    });
    await page.goto(pageUrl);
    const calls = await page.evaluate(() => window.__readyCalls);
    const readyMsg = calls.find((m) => m.type === 'ready');
    expect(readyMsg).toBeUndefined();
  });
});

test.describe('polyHostPush: state parsing', () => {
  test.beforeEach(async ({ page }) => {
    await page.addInitScript(() => {
      window.__POLY_EMBEDDED__ = true;
      window.polyHostCall = () => {};
    });
    await page.goto(pageUrl);
  });

  test('accepts valid JSON string and delivers state via onState', async ({ page }) => {
    const received = await page.evaluate(() => {
      return new Promise((resolve) => {
        window.PolyPluginHost.onState((s) => resolve(s.preset));
        const state = window.PolyMockHost.getState();
        window.polyHostPush(JSON.stringify({ type: 'state', state }));
      });
    });
    expect(received).toBeTruthy();
  });

  test('accepts pre-parsed object', async ({ page }) => {
    const received = await page.evaluate(() => {
      return new Promise((resolve) => {
        window.PolyPluginHost.onState((s) => resolve(s.preset));
        const state = window.PolyMockHost.getState();
        window.polyHostPush({ type: 'state', state });
      });
    });
    expect(received).toBeTruthy();
  });

  test('silently ignores malformed JSON', async ({ page }) => {
    const errors = [];
    page.on('console', (msg) => {
      if (msg.type() === 'error') errors.push(msg.text());
    });
    await page.evaluate(() => { window.polyHostPush('not json {{{'); });
    expect(errors.some((e) => e.includes('bad push payload'))).toBe(true);
    const state = await page.evaluate(() => window.PolyPluginHost.getState());
    expect(state).toBeNull();
  });

  test('getState returns null before any push', async ({ page }) => {
    const state = await page.evaluate(() => window.PolyPluginHost.getState());
    expect(state).toBeNull();
  });

  test('getState returns last pushed state', async ({ page }) => {
    await page.evaluate(() => {
      const state = window.PolyMockHost.getState();
      window.polyHostPush(JSON.stringify({ type: 'state', state }));
    });
    const preset = await page.evaluate(() => window.PolyPluginHost.getState()?.preset);
    expect(preset).toBeTruthy();
  });
});

test.describe('polyHostPush: frame handling', () => {
  test.beforeEach(async ({ page }) => {
    await page.addInitScript(() => {
      window.__POLY_EMBEDDED__ = true;
      window.polyHostCall = () => {};
    });
    await page.goto(pageUrl);
  });

  test('delivers frame data via onFrame', async ({ page }) => {
    const frame = await page.evaluate(() => {
      return new Promise((resolve) => {
        window.PolyPluginHost.onFrame((f) => resolve(f));
        window.polyHostPush(JSON.stringify({
          type: 'frame',
          frame: { t8: 4.5, playing: true, convLeft: 24, lanes: [] }
        }));
      });
    });
    expect(frame.t8).toBe(4.5);
    expect(frame.playing).toBe(true);
  });

  test('ignores unknown message types', async ({ page }) => {
    await page.evaluate(() => {
      window.polyHostPush(JSON.stringify({ type: 'unknown', data: 123 }));
    });
    const state = await page.evaluate(() => window.PolyPluginHost.getState());
    expect(state).toBeNull();
  });
});

test.describe('embedded boot sequence', () => {
  test('UI does not render until state is pushed', async ({ page }) => {
    await page.addInitScript(() => {
      window.__POLY_EMBEDDED__ = true;
      window.polyHostCall = () => {};
    });
    await page.goto(pageUrl);
    const strips = await page.locator('.strip').count();
    expect(strips).toBe(0);
  });

  test('UI boots after first state push via polyHostPush', async ({ page }) => {
    await page.addInitScript(() => {
      window.__POLY_EMBEDDED__ = true;
      window.polyHostCall = () => {};
    });
    await page.goto(pageUrl);
    await page.evaluate(() => {
      const state = window.PolyMockHost.getState();
      window.polyHostPush(JSON.stringify({ type: 'state', state }));
    });
    await expect(page.locator('.strip').first()).toBeVisible();
    const count = await page.locator('.strip').count();
    expect(count).toBeGreaterThanOrEqual(4);
  });

  test('preset name shows after state push', async ({ page }) => {
    await page.addInitScript(() => {
      window.__POLY_EMBEDDED__ = true;
      window.polyHostCall = () => {};
    });
    await page.goto(pageUrl);
    await page.evaluate(() => {
      const state = window.PolyMockHost.getState();
      window.polyHostPush(JSON.stringify({ type: 'state', state }));
    });
    await expect(page.locator('#presetName')).not.toHaveText('—');
  });

  test('macro sliders render after state push', async ({ page }) => {
    await page.addInitScript(() => {
      window.__POLY_EMBEDDED__ = true;
      window.polyHostCall = () => {};
    });
    await page.goto(pageUrl);
    await page.evaluate(() => {
      const state = window.PolyMockHost.getState();
      window.polyHostPush(JSON.stringify({ type: 'state', state }));
    });
    await expect(page.locator('#master .macro')).toHaveCount(6);
  });
});

test.describe('standalone boot sequence', () => {
  test('UI renders immediately from mock-host state', async ({ page }) => {
    await page.goto(pageUrl);
    await expect(page.locator('.strip').first()).toBeVisible();
    const count = await page.locator('.strip').count();
    expect(count).toBeGreaterThanOrEqual(4);
  });

  test('mock-host getState is available synchronously', async ({ page }) => {
    await page.goto(pageUrl);
    const hasState = await page.evaluate(() => window.PolyMockHost.getState() !== null);
    expect(hasState).toBe(true);
  });
});

test.describe('outgoing message format (embedded)', () => {
  let sentMessages;

  test.beforeEach(async ({ page }) => {
    await page.addInitScript(() => {
      window.__POLY_EMBEDDED__ = true;
      window.__sentMessages = [];
      window.polyHostCall = (json) => { window.__sentMessages.push(JSON.parse(json)); };
    });
    await page.goto(pageUrl);
    await page.evaluate(() => {
      const state = window.PolyMockHost.getState();
      window.polyHostPush(JSON.stringify({ type: 'state', state }));
    });
  });

  test('action messages include v, type, name, payload', async ({ page }) => {
    await page.evaluate(() => { window.__sentMessages.length = 0; });
    await page.click('#scB');
    const msgs = await page.evaluate(() => window.__sentMessages);
    const action = msgs.find((m) => m.type === 'action');
    expect(action).toBeDefined();
    expect(action.v).toBe(1);
    expect(action.name).toBe('selectScene');
    expect(action.payload).toEqual({ scene: 'B' });
  });

  test('edit messages include v, type, paramId, value, gesture', async ({ page }) => {
    await page.evaluate(() => { window.__sentMessages.length = 0; });
    await page.evaluate(() => {
      window.PolyHost.edit('macro.complexity', 0.75, 'perform');
    });
    const msgs = await page.evaluate(() => window.__sentMessages);
    const edit = msgs.find((m) => m.type === 'edit');
    expect(edit).toBeDefined();
    expect(edit.v).toBe(1);
    expect(edit.paramId).toBe('macro.complexity');
    expect(edit.value).toBe(0.75);
    expect(edit.gesture).toBe('perform');
  });
});

test.describe('missing polyHostCall binding (embedded)', () => {
  test('logs error when polyHostCall is not a function', async ({ page }) => {
    const errors = [];
    page.on('console', (msg) => {
      if (msg.type() === 'error') errors.push(msg.text());
    });
    await page.addInitScript(() => { window.__POLY_EMBEDDED__ = true; });
    await page.goto(pageUrl);
    expect(errors.some((e) => e.includes('polyHostCall missing'))).toBe(true);
  });
});

test.describe('state field completeness', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
  });

  test('top-level state has all required fields', async ({ page }) => {
    const fields = await page.evaluate(() => {
      const s = window.PolyHost.getState();
      return {
        hasPreset: typeof s.preset === 'string',
        hasSeed: typeof s.seed === 'number',
        hasTempo: typeof s.tempo === 'number',
        hasScene: typeof s.scene === 'string',
        hasMorph: typeof s.morph === 'number',
        hasMacros: typeof s.macros === 'object' && s.macros !== null,
        hasLanes: Array.isArray(s.lanes),
        hasPresets: Array.isArray(s.presets),
      };
    });
    for (const [k, v] of Object.entries(fields)) {
      expect(v, k).toBe(true);
    }
  });

  test('macros has all 6 fields', async ({ page }) => {
    const keys = await page.evaluate(() => Object.keys(window.PolyHost.getState().macros).sort());
    expect(keys).toEqual(['complexity', 'density', 'humanize', 'swing', 'syncopation', 'tension']);
  });

  test('each lane has all required fields', async ({ page }) => {
    const result = await page.evaluate(() => {
      const required = [
        'name', 'role', 'note', 'ch', 'steps', 'stepLen', 'vel', 'prob',
        'spread', 'ghost', 'push', 'hits', 'rot', 'timeline', 'pattern',
        'mt', 'envs', 'hue', 'active'
      ];
      const lanes = window.PolyHost.getState().lanes;
      const missing = [];
      lanes.forEach((l, i) => {
        required.forEach((f) => {
          if (l[f] === undefined) missing.push(`lane[${i}].${f}`);
        });
      });
      return { count: lanes.length, missing };
    });
    expect(result.count).toBeGreaterThanOrEqual(4);
    expect(result.missing).toEqual([]);
  });

  test('pattern array length matches steps', async ({ page }) => {
    const mismatches = await page.evaluate(() => {
      const lanes = window.PolyHost.getState().lanes;
      return lanes
        .map((l, i) => ({ i, steps: l.steps, patLen: l.pattern.length }))
        .filter((x) => x.patLen !== x.steps);
    });
    expect(mismatches).toEqual([]);
  });

  test('mt array length matches steps', async ({ page }) => {
    const mismatches = await page.evaluate(() => {
      const lanes = window.PolyHost.getState().lanes;
      return lanes
        .map((l, i) => ({ i, steps: l.steps, mtLen: l.mt.length }))
        .filter((x) => x.mtLen !== x.steps);
    });
    expect(mismatches).toEqual([]);
  });

  test('presets array has 14 factory presets', async ({ page }) => {
    const presets = await page.evaluate(() => window.PolyHost.getState().presets);
    expect(presets).toHaveLength(14);
    for (const p of presets) {
      expect(typeof p.name).toBe('string');
      expect(p.name.length).toBeGreaterThan(0);
    }
  });

  test('chain object has required fields', async ({ page }) => {
    const chain = await page.evaluate(() => {
      const s = window.PolyHost.getState();
      return s.chain;
    });
    expect(chain).toBeDefined();
    expect(typeof chain.enabled).toBe('boolean');
    expect(typeof chain.mode).toBe('number');
    expect(typeof chain.entryCount).toBe('number');
    expect(Array.isArray(chain.entries)).toBe(true);
  });

  test('noteMap has 128 entries', async ({ page }) => {
    const len = await page.evaluate(() => window.PolyHost.getState().noteMap?.length);
    expect(len).toBe(128);
  });
});

test.describe('state field completeness (embedded push)', () => {
  test('state pushed through polyHostPush retains all fields', async ({ page }) => {
    await page.addInitScript(() => {
      window.__POLY_EMBEDDED__ = true;
      window.polyHostCall = () => {};
    });
    await page.goto(pageUrl);
    const result = await page.evaluate(() => {
      const state = window.PolyMockHost.getState();
      window.polyHostPush(JSON.stringify({ type: 'state', state }));
      const s = window.PolyPluginHost.getState();
      const required = ['preset', 'seed', 'tempo', 'scene', 'morph', 'macros', 'lanes', 'presets', 'chain', 'noteMap'];
      return required.filter((f) => s[f] === undefined);
    });
    expect(result).toEqual([]);
  });

  test('lane fields survive JSON round-trip through polyHostPush', async ({ page }) => {
    await page.addInitScript(() => {
      window.__POLY_EMBEDDED__ = true;
      window.polyHostCall = () => {};
    });
    await page.goto(pageUrl);
    const result = await page.evaluate(() => {
      const state = window.PolyMockHost.getState();
      window.polyHostPush(JSON.stringify({ type: 'state', state }));
      const s = window.PolyPluginHost.getState();
      const required = [
        'name', 'role', 'note', 'ch', 'steps', 'stepLen', 'vel', 'prob',
        'spread', 'ghost', 'push', 'hits', 'rot', 'timeline', 'pattern',
        'mt', 'envs', 'hue', 'active', 'accents'
      ];
      const missing = [];
      s.lanes.forEach((l, i) => {
        required.forEach((f) => {
          if (l[f] === undefined) missing.push(`lane[${i}].${f}`);
        });
      });
      return missing;
    });
    expect(result).toEqual([]);
  });
});

test.describe('duplicate state suppression', () => {
  test('identical state push does not re-render', async ({ page }) => {
    await page.goto(pageUrl);
    await page.waitForSelector('.strip');
    const mutations = await page.evaluate(() => {
      let count = 0;
      const observer = new MutationObserver(() => { count++; });
      observer.observe(document.getElementById('desk'), { childList: true, subtree: true });
      window.PolyMockHost._pushState();
      return new Promise((resolve) => {
        setTimeout(() => {
          observer.disconnect();
          resolve(count);
        }, 100);
      });
    });
    expect(mutations).toBe(0);
  });
});

test.describe('onState late subscriber (embedded)', () => {
  test('late onState subscriber receives cached state', async ({ page }) => {
    await page.addInitScript(() => {
      window.__POLY_EMBEDDED__ = true;
      window.polyHostCall = () => {};
    });
    await page.goto(pageUrl);
    const received = await page.evaluate(() => {
      const state = window.PolyMockHost.getState();
      window.polyHostPush(JSON.stringify({ type: 'state', state }));
      return new Promise((resolve) => {
        window.PolyPluginHost.onState((s) => resolve(s.preset));
      });
    });
    expect(received).toBeTruthy();
  });
});

test.describe('all three hosts are defined', () => {
  test('PolyMockHost, PolyPluginHost, and PolyHost are all on window', async ({ page }) => {
    await page.goto(pageUrl);
    const defined = await page.evaluate(() => ({
      mock: typeof window.PolyMockHost === 'object',
      plugin: typeof window.PolyPluginHost === 'object',
      host: typeof window.PolyHost === 'object',
    }));
    expect(defined.mock).toBe(true);
    expect(defined.plugin).toBe(true);
    expect(defined.host).toBe(true);
  });

  test('polyHostPush is always defined (for C++ to call)', async ({ page }) => {
    await page.goto(pageUrl);
    const isFn = await page.evaluate(() => typeof window.polyHostPush === 'function');
    expect(isFn).toBe(true);
  });
});

test.describe('debug overlay (embedded)', () => {
  test('state push creates debug overlay element', async ({ page }) => {
    await page.addInitScript(() => {
      window.__POLY_EMBEDDED__ = true;
      window.polyHostCall = () => {};
    });
    await page.goto(pageUrl);
    await page.evaluate(() => {
      const state = window.PolyMockHost.getState();
      window.polyHostPush(JSON.stringify({ type: 'state', state }));
    });
    await expect(page.locator('#_dbg')).toBeVisible();
    const text = await page.locator('#_dbg').textContent();
    expect(text).toContain('SC:1');
    expect(text).toContain('FC:0');
  });
});

test.describe('scene selection (embedded)', () => {
  test.beforeEach(async ({ page }) => {
    await page.addInitScript(() => {
      window.__POLY_EMBEDDED__ = true;
      window.__sentMessages = [];
      window.polyHostCall = (json) => { window.__sentMessages.push(JSON.parse(json)); };
    });
    await page.goto(pageUrl);
    await page.evaluate(() => {
      const state = window.PolyMockHost.getState();
      window.polyHostPush(JSON.stringify({ type: 'state', state }));
    });
    await page.waitForSelector('.strip');
  });

  test('clicking B sends selectScene action with scene B', async ({ page }) => {
    await page.evaluate(() => { window.__sentMessages.length = 0; });
    await page.click('#scB');
    const msgs = await page.evaluate(() => window.__sentMessages);
    const action = msgs.find((m) => m.type === 'action' && m.name === 'selectScene');
    expect(action).toBeDefined();
    expect(action.payload.scene).toBe('B');
  });

  test('clicking Morph sends selectScene action with scene Morph', async ({ page }) => {
    await page.evaluate(() => { window.__sentMessages.length = 0; });
    await page.click('#scM');
    const msgs = await page.evaluate(() => window.__sentMessages);
    const action = msgs.find((m) => m.type === 'action' && m.name === 'selectScene');
    expect(action).toBeDefined();
    expect(action.payload.scene).toBe('Morph');
  });

  test('scene buttons reflect pushed state', async ({ page }) => {
    await page.evaluate(() => {
      const state = window.PolyMockHost.getState();
      state.scene = 'B';
      window.polyHostPush(JSON.stringify({ type: 'state', state }));
    });
    await expect(page.locator('#scB')).toHaveClass(/on/);
    await expect(page.locator('#scA')).not.toHaveClass(/on/);
  });

  test('morph slider appears when scene is Morph', async ({ page }) => {
    await page.evaluate(() => {
      const state = window.PolyMockHost.getState();
      state.scene = 'Morph';
      state.morph = 0.5;
      window.polyHostPush(JSON.stringify({ type: 'state', state }));
    });
    await expect(page.locator('#morphSlider')).toBeVisible();
  });
});

test.describe('preset application (embedded)', () => {
  test.beforeEach(async ({ page }) => {
    await page.addInitScript(() => {
      window.__POLY_EMBEDDED__ = true;
      window.__sentMessages = [];
      window.polyHostCall = (json) => { window.__sentMessages.push(JSON.parse(json)); };
    });
    await page.goto(pageUrl);
    await page.evaluate(() => {
      const state = window.PolyMockHost.getState();
      window.polyHostPush(JSON.stringify({ type: 'state', state }));
    });
    await page.waitForSelector('.strip');
  });

  test('clicking a factory preset sends applyPreset action', async ({ page }) => {
    await page.click('#presetName');
    await page.waitForSelector('.preset-menu.open');
    await page.evaluate(() => { window.__sentMessages.length = 0; });
    const opts = page.locator('.preset-menu [role="option"]');
    const count = await opts.count();
    expect(count).toBeGreaterThan(1);
    await opts.nth(2).click();
    const msgs = await page.evaluate(() => window.__sentMessages);
    const action = msgs.find((m) => m.type === 'action' && m.name === 'applyPreset');
    expect(action).toBeDefined();
    expect(typeof action.payload.index).toBe('number');
  });

  test('preset dropdown closes after selection', async ({ page }) => {
    await page.click('#presetName');
    await page.waitForSelector('.preset-menu.open');
    const opts = page.locator('.preset-menu [role="option"]');
    await opts.nth(2).click();
    await expect(page.locator('.preset-menu')).not.toHaveClass(/open/);
  });

  test('UI updates when state push follows preset change', async ({ page }) => {
    const origLaneName = await page.locator('.strip .nm b').first().textContent();
    await page.evaluate(() => {
      const state = window.PolyMockHost.getState();
      state.preset = 'Afro-Cuban';
      state.lanes[0].name = 'Clave';
      state.lanes[0].steps = 7;
      state.lanes[0].hits = 5;
      window.polyHostPush(JSON.stringify({ type: 'state', state }));
    });
    const newLaneName = await page.locator('.strip .nm b').first().textContent();
    expect(newLaneName).toBe('Clave');
    expect(newLaneName).not.toBe(origLaneName);
  });
});

test.describe('state push resilience (embedded)', () => {
  test('state push works after multiple frame pushes', async ({ page }) => {
    await page.addInitScript(() => {
      window.__POLY_EMBEDDED__ = true;
      window.polyHostCall = () => {};
    });
    await page.goto(pageUrl);
    await page.evaluate(() => {
      for (let i = 0; i < 10; i++) {
        window.polyHostPush(JSON.stringify({
          type: 'frame',
          frame: { t8: i * 0.5, playing: false, convLeft: 120, lanes: [] }
        }));
      }
    });
    const strips = await page.locator('.strip').count();
    expect(strips).toBe(0);
    await page.evaluate(() => {
      const state = window.PolyMockHost.getState();
      window.polyHostPush(JSON.stringify({ type: 'state', state }));
    });
    await expect(page.locator('.strip').first()).toBeVisible();
  });

  test('second state push with different data re-renders', async ({ page }) => {
    await page.addInitScript(() => {
      window.__POLY_EMBEDDED__ = true;
      window.polyHostCall = () => {};
    });
    await page.goto(pageUrl);
    await page.evaluate(() => {
      const state = window.PolyMockHost.getState();
      window.polyHostPush(JSON.stringify({ type: 'state', state }));
    });
    await page.waitForSelector('.strip');
    const origName = await page.locator('.strip .nm b').first().textContent();
    await page.evaluate(() => {
      const state = window.PolyMockHost.getState();
      state.lanes[0].name = 'NewLane';
      window.polyHostPush(JSON.stringify({ type: 'state', state }));
    });
    const newName = await page.locator('.strip .nm b').first().textContent();
    expect(newName).toBe('NewLane');
  });
});
