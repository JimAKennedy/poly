import { test, expect } from '@playwright/test';
import { setupWithActionLog, getActions, clearActions, expandStrip, pageUrl } from './test-helpers.mjs';

test.describe('master macros section', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
  });

  test('renders all 6 macro sliders with labels', async ({ page }) => {
    const master = page.locator('#master');
    await expect(master).toBeVisible();
    await expect(master.locator('.macro')).toHaveCount(6);
    for (const name of ['Complexity', 'Density', 'Syncopation', 'Swing', 'Tension', 'Humanize']) {
      await expect(master.locator('.macro .t span', { hasText: name })).toBeVisible();
    }
  });

  test('macro values match initial state', async ({ page }) => {
    const expected = { Complexity: '45', Density: '50', Syncopation: '30', Swing: '10', Tension: '25', Humanize: '15' };
    for (const [name, val] of Object.entries(expected)) {
      const macro = page.locator('.macro', { has: page.locator('.t span', { hasText: name }) });
      await expect(macro.locator('.t b')).toHaveText(val);
    }
  });

  test('convergence meter is rendered', async ({ page }) => {
    await expect(page.locator('#cmeter')).toBeVisible();
    await expect(page.locator('#cmeter span')).toHaveText('Convergence');
  });
});

test.describe('embedded mode', () => {
  test('play button is visually disabled', async ({ page }) => {
    await page.addInitScript(() => { window.__POLY_EMBEDDED__ = true; });
    await page.goto(pageUrl);
    const playBtn = page.locator('#play');
    const opacity = await playBtn.evaluate((el) => el.style.opacity);
    const cursor = await playBtn.evaluate((el) => el.style.cursor);
    expect(opacity).toBe('0.35');
    expect(cursor).toBe('default');
  });

  test('play button click does not dispatch togglePlay', async ({ page }) => {
    await page.addInitScript(() => { window.__POLY_EMBEDDED__ = true; });
    await setupWithActionLog(page);
    await page.click('#play');
    const acts = await getActions(page);
    expect(acts.filter((a) => a.name === 'togglePlay')).toHaveLength(0);
  });

  test('Space key does not dispatch togglePlay', async ({ page }) => {
    await page.addInitScript(() => { window.__POLY_EMBEDDED__ = true; });
    await setupWithActionLog(page);
    await page.keyboard.press('Space');
    const acts = await getActions(page);
    expect(acts.filter((a) => a.name === 'togglePlay')).toHaveLength(0);
  });

  test('non-transport controls still work', async ({ page }) => {
    await page.addInitScript(() => { window.__POLY_EMBEDDED__ = true; });
    await page.goto(pageUrl);
    // In embedded mode, PolyHost = PolyPluginHost which calls polyHostCall().
    // Intercept PolyHost.action directly since mock-host isn't the active host.
    await page.evaluate(() => {
      window.__actionLog = [];
      const orig = window.PolyHost.action;
      window.PolyHost.action = (name, payload) => {
        window.__actionLog.push({ name, payload: JSON.parse(JSON.stringify(payload)) });
        orig.call(window.PolyHost, name, payload);
      };
    });
    // Push initial state so the UI boots (PolyPluginHost has no state until native push)
    await page.evaluate(() => {
      window.polyHostPush(JSON.stringify({ type: 'state', state: window.PolyMockHost.getState() }));
    });
    await page.click('#scB');
    const acts = await page.evaluate(() => window.__actionLog.slice());
    expect(acts).toContainEqual(
      expect.objectContaining({ name: 'selectScene', payload: { scene: 'B' } })
    );
  });
});

test.describe('state-change propagation', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
  });

  test('preset name updates on state push', async ({ page }) => {
    await expect(page.locator('#presetName')).toHaveText('Afrobeat 12/8');
    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.preset = 'West African 4/4';
      window.PolyMockHost._pushState();
    });
    await expect(page.locator('#presetName')).toHaveText('West African 4/4');
  });

  test('scene indicator updates on external state push', async ({ page }) => {
    await expect(page.locator('#scA')).toHaveClass(/on/);
    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.scene = 'B';
      window.PolyMockHost._pushState();
    });
    await expect(page.locator('#scB')).toHaveClass(/on/);
    await expect(page.locator('#scA')).not.toHaveClass(/on/);
  });

  test('lane count change rebuilds strips', async ({ page }) => {
    await expect(page.locator('.strip[data-lane]')).toHaveCount(5);
    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.lanes = s.lanes.slice(0, 3);
      window.PolyMockHost._pushState();
    });
    await expect(page.locator('.strip[data-lane]')).toHaveCount(3);
  });

  test('euclid param change updates expanded pattern display', async ({ page }) => {
    await expandStrip(page, 2);
    const strip = page.locator('.strip[data-lane="2"]');
    await expect(strip.locator('[data-pane="pattern"]')).toContainText('E(2,3)');
    await page.evaluate(() => {
      const { euclid, rotArr } = window.PolyGrooveMath;
      const s = window.PolyMockHost.getState();
      const l = s.lanes[2];
      l.hits = 5;
      l.steps = 8;
      l.pattern = rotArr(euclid(5, 8), l.rot);
      l.mt = new Array(8).fill(0);
      window.PolyMockHost._pushState();
    });
    await expect(strip.locator('[data-pane="pattern"]')).toContainText('E(5,8)');
  });

  test('macro value updates on state push', async ({ page }) => {
    const macro = page.locator('.macro', { has: page.locator('.t span', { hasText: 'Complexity' }) });
    await expect(macro.locator('.t b')).toHaveText('45');
    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.macros.complexity = 0.8;
      window.PolyMockHost._pushState();
    });
    await expect(macro.locator('.t b')).toHaveText('80');
  });

  test('tempo and seed update on state push', async ({ page }) => {
    await expect(page.locator('#tempoVal')).toHaveText('126.0');
    await expect(page.locator('#seedVal')).toHaveText('88');
    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.tempo = 140;
      s.seed = 42;
      window.PolyMockHost._pushState();
    });
    await expect(page.locator('#tempoVal')).toHaveText('140.0');
    await expect(page.locator('#seedVal')).toHaveText('42');
  });
});

test.describe('boundary values', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
  });

  test('steps clamp at minimum of 2', async ({ page }) => {
    await page.evaluate(() => {
      const { euclid, rotArr } = window.PolyGrooveMath;
      const s = window.PolyMockHost.getState();
      const l = s.lanes[2];
      l.steps = 3;
      l.hits = 2;
      l.pattern = rotArr(euclid(2, 3), 0);
      l.mt = new Array(3).fill(0);
      window.PolyMockHost._pushState();
    });
    await expandStrip(page, 2);
    const strip = page.locator('.strip[data-lane="2"]');

    await strip.locator('[data-st="-1"]').click();
    await expect(strip.locator('.stepper .v').first()).toContainText('2');

    await strip.locator('[data-st="-1"]').click();
    await expect(strip.locator('.stepper .v').first()).toContainText('2');
  });

  test('steps clamp at maximum of 16', async ({ page }) => {
    await page.evaluate(() => {
      const { euclid, rotArr } = window.PolyGrooveMath;
      const s = window.PolyMockHost.getState();
      const l = s.lanes[2];
      l.steps = 15;
      l.hits = 2;
      l.pattern = rotArr(euclid(2, 15), 0);
      l.mt = new Array(15).fill(0);
      window.PolyMockHost._pushState();
    });
    await expandStrip(page, 2);
    const strip = page.locator('.strip[data-lane="2"]');

    await strip.locator('[data-st="1"]').click();
    await expect(strip.locator('.stepper .v').first()).toContainText('16');

    await strip.locator('[data-st="1"]').click();
    await expect(strip.locator('.stepper .v').first()).toContainText('16');
  });

  test('hits clamp to step count', async ({ page }) => {
    await expandStrip(page, 2);
    const strip = page.locator('.strip[data-lane="2"]');

    await strip.locator('[data-ht="1"]').click();
    await expect(strip.locator('.stepper .v').nth(1)).toContainText('E(3,3)');

    await strip.locator('[data-ht="1"]').click();
    await expect(strip.locator('.stepper .v').nth(1)).toContainText('E(3,3)');
  });

  test('hits decrement to zero', async ({ page }) => {
    await page.evaluate(() => {
      const { euclid, rotArr } = window.PolyGrooveMath;
      const s = window.PolyMockHost.getState();
      const l = s.lanes[2];
      l.steps = 4;
      l.hits = 1;
      l.pattern = rotArr(euclid(1, 4), 0);
      l.mt = new Array(4).fill(0);
      window.PolyMockHost._pushState();
    });
    await expandStrip(page, 2);
    const strip = page.locator('.strip[data-lane="2"]');

    await strip.locator('[data-ht="-1"]').click();
    await expect(strip.locator('.stepper .v').nth(1)).toContainText('E(0,4)');
    await expect(strip.locator('.ladder button.hit')).toHaveCount(0);
  });

  test('cells array at 6 resets to 2 on add', async ({ page }) => {
    await expandStrip(page, 4);
    const strip = page.locator('.strip[data-lane="4"]');
    await strip.locator('[data-cl]').click();

    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.lanes[4].cells = [2, 3, 2, 3, 2, 3];
      s.lanes[4].mt = new Array(6).fill(0);
      window.PolyMockHost._pushState();
    });

    await page.keyboard.press('Escape');
    await expandStrip(page, 4);

    await clearActions(page);
    await strip.locator('[data-addcell]').click();

    const acts = await getActions(page);
    const cellAction = acts.filter((a) => a.name === 'setCells').pop();
    expect(cellAction.payload.cells).toHaveLength(2);
  });

  test('cell value cycles 2 → 3 → 4 → 2', async ({ page }) => {
    await expandStrip(page, 4);
    const strip = page.locator('.strip[data-lane="4"]');
    await strip.locator('[data-cl]').click();

    const cellBtn = strip.locator('[data-cells] button[data-i="0"]');
    await clearActions(page);

    await cellBtn.click();
    let acts = await getActions(page);
    let ca = acts.filter((a) => a.name === 'setCells').pop();
    expect(ca.payload.cells[0]).toBe(3);

    await clearActions(page);
    await cellBtn.click();
    acts = await getActions(page);
    ca = acts.filter((a) => a.name === 'setCells').pop();
    expect(ca.payload.cells[0]).toBe(4);

    await clearActions(page);
    await cellBtn.click();
    acts = await getActions(page);
    ca = acts.filter((a) => a.name === 'setCells').pop();
    expect(ca.payload.cells[0]).toBe(2);
  });
});

test.describe('multi-lane expand switching', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
  });

  test('expanding lane 2 collapses lane 0', async ({ page }) => {
    await expandStrip(page, 0);
    await expect(page.locator('.strip[data-lane="0"]')).toHaveClass(/expanded/);

    await expandStrip(page, 2);
    await expect(page.locator('.strip[data-lane="2"]')).toHaveClass(/expanded/);
    await expect(page.locator('.strip[data-lane="0"]')).not.toHaveClass(/expanded/);
  });

  test('only one strip expanded at a time across all lanes', async ({ page }) => {
    for (let i = 0; i < 5; i++) {
      await expandStrip(page, i);
      const expandedCount = await page.locator('.strip.expanded').count();
      expect(expandedCount).toBe(1);
      await expect(page.locator(`.strip[data-lane="${i}"]`)).toHaveClass(/expanded/);
    }
  });

  test('expand button toggles off when already expanded', async ({ page }) => {
    await expandStrip(page, 1);
    await expect(page.locator('.strip[data-lane="1"]')).toHaveClass(/expanded/);

    await page.click('.strip[data-lane="1"] .ex');
    await expect(page.locator('.strip[data-lane="1"]')).not.toHaveClass(/expanded/);
    expect(await page.locator('.strip.expanded').count()).toBe(0);
  });
});
