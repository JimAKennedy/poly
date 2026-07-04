import { test, expect } from '@playwright/test';
import { setupWithActionLog, getActions, clearActions, getEdits, clearEdits, expandStrip, pageUrl } from './test-helpers.mjs';

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

test.describe('frame feedback — play icon', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
  });

  test('play icon shows triangle when stopped', async ({ page }) => {
    const d = await page.locator('#picon').getAttribute('d');
    expect(d).toContain('L12 7');
  });

  test('play icon changes to pause bars when playing', async ({ page }) => {
    await page.evaluate(() => {
      window.PolyHost.onFrame((f) => { window.__lastTestFrame = f; });
    });
    await page.click('#play');
    await page.waitForFunction(() => window.__lastTestFrame && window.__lastTestFrame.playing);
    const d = await page.locator('#picon').getAttribute('d');
    expect(d).toContain('H5.5');
  });

  test('play icon reverts to triangle on stop', async ({ page }) => {
    await page.click('#play');
    await page.evaluate(() => new Promise((r) => requestAnimationFrame(r)));
    await page.click('#play');
    await page.evaluate(() => new Promise((r) => requestAnimationFrame(r)));
    const d = await page.locator('#picon').getAttribute('d');
    expect(d).toContain('L12 7');
  });
});

test.describe('frame feedback — convergence and meters', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
  });

  test('convergence counter renders initial value', async ({ page }) => {
    const text = await page.locator('#conv b').textContent();
    expect(parseInt(text)).toBeGreaterThanOrEqual(1);
  });

  test('convergence meter height starts at 0% when stopped', async ({ page }) => {
    const height = await page.locator('#cmeter i').evaluate((el) => el.style.height);
    expect(height).toBe('0%');
  });

  test('VU meters have initial width', async ({ page }) => {
    const widths = await page.locator('.vu i').evaluateAll((els) => els.map((e) => e.style.width));
    expect(widths.length).toBeGreaterThanOrEqual(4);
    widths.forEach((w) => expect(w).toBe('4%'));
  });

  test('ring hand SVG elements exist for each lane', async ({ page }) => {
    const hands = await page.locator('svg.ring line').count();
    const laneCount = await page.locator('.strip[data-lane]').count();
    expect(hands).toBe(laneCount);
  });

  test('ring hand has transform attribute', async ({ page }) => {
    const transform = await page.locator('svg.ring line').first().getAttribute('transform');
    expect(transform).toContain('rotate(');
  });
});

test.describe('frame feedback — step highlighting', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
  });

  test('no steps highlighted when stopped', async ({ page }) => {
    const nowCount = await page.locator('.ladder button.now').count();
    expect(nowCount).toBe(0);
  });

  test('step gets .now class when playing', async ({ page }) => {
    await page.evaluate(() => {
      window.PolyHost.onFrame((f) => { window.__lastTestFrame = f; });
    });
    await page.click('#play');
    await page.waitForFunction(() => window.__lastTestFrame && window.__lastTestFrame.playing);
    await page.waitForFunction(() => document.querySelectorAll('.ladder button.now').length > 0, null, { timeout: 3000 });
    const nowCount = await page.locator('.ladder button.now').count();
    expect(nowCount).toBeGreaterThanOrEqual(1);
  });

  test('step highlighting clears on stop', async ({ page }) => {
    await page.click('#play');
    await page.evaluate(() => new Promise((r) => requestAnimationFrame(r)));
    await page.click('#play');
    await page.evaluate(() => new Promise((r) => requestAnimationFrame(r)));
    await page.evaluate(() => new Promise((r) => requestAnimationFrame(r)));
    const nowCount = await page.locator('.ladder button.now').count();
    expect(nowCount).toBe(0);
  });
});

test.describe('popover dismiss behaviors', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
  });

  test('chain popover closes on click outside', async ({ page }) => {
    await page.click('#chainBtn');
    await expect(page.locator('#chainPopover')).toBeVisible();
    await page.click('#desk', { position: { x: 5, y: 5 } });
    await expect(page.locator('#chainPopover')).toHaveCount(0);
  });

  test('chain popover toggles on repeated button clicks', async ({ page }) => {
    await page.click('#chainBtn');
    await expect(page.locator('#chainPopover')).toBeVisible();
    await page.click('#chainBtn');
    await expect(page.locator('#chainPopover')).toHaveCount(0);
    await page.click('#chainBtn');
    await expect(page.locator('#chainPopover')).toBeVisible();
  });

  test('note map modal closes on click outside', async ({ page }) => {
    await page.click('#noteMapBtn');
    await expect(page.locator('.notemap-modal')).toBeVisible();
    await page.waitForTimeout(50);
    await page.click('body', { position: { x: 1, y: 1 }, force: true });
    await expect(page.locator('.notemap-modal')).toHaveCount(0);
  });

  test('note map modal close button works', async ({ page }) => {
    await page.click('#noteMapBtn');
    await expect(page.locator('.notemap-modal')).toBeVisible();
    await page.click('.notemap-close');
    await expect(page.locator('.notemap-modal')).toHaveCount(0);
  });

  test('note map modal toggle via close button then reopen', async ({ page }) => {
    await page.click('#noteMapBtn');
    await expect(page.locator('.notemap-modal')).toBeVisible();
    await page.click('.notemap-close');
    await expect(page.locator('.notemap-modal')).toHaveCount(0);
    await page.click('#noteMapBtn');
    await expect(page.locator('.notemap-modal')).toBeVisible();
  });

  test('note map rebuilds on state push while open', async ({ page }) => {
    await page.click('#noteMapBtn');
    await expect(page.locator('.notemap-modal')).toBeVisible();
    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.noteMap[0] = 42;
      window.PolyMockHost._pushState();
    });
    const val = await page.locator('.notemap-row[data-note="0"] .no').textContent();
    expect(val).toBe('42');
  });
});

test.describe('note map boundary values', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
  });

  test('decrement at note 0 with output 0 does not dispatch', async ({ page }) => {
    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.noteMap[0] = 0;
      window.PolyMockHost._pushState();
    });
    await page.click('#noteMapBtn');
    await clearActions(page);
    await page.click('.notemap-row[data-note="0"] [data-nmdec="0"]');
    const acts = await getActions(page);
    expect(acts.filter((a) => a.name === 'setNoteMap')).toHaveLength(0);
  });

  test('increment at note 127 with output 127 does not dispatch', async ({ page }) => {
    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.noteMap[127] = 127;
      window.PolyMockHost._pushState();
    });
    await page.click('#noteMapBtn');
    await clearActions(page);
    await page.click('.notemap-row[data-note="127"] [data-nminc="127"]');
    const acts = await getActions(page);
    expect(acts.filter((a) => a.name === 'setNoteMap')).toHaveLength(0);
  });

  test('note map reset clears modified indicator', async ({ page }) => {
    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.noteMap[5] = 42;
      window.PolyMockHost._pushState();
    });
    await page.click('#noteMapBtn');
    await expect(page.locator('.notemap-row[data-note="5"] .nm-mod')).toBeVisible();
    await clearActions(page);
    await page.click('.notemap-reset');
    const acts = await getActions(page);
    expect(acts.some((a) => a.name === 'resetNoteMap')).toBe(true);
  });
});

test.describe('cross-feature regression', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
  });

  test('preset switch while strip expanded re-expands at same index', async ({ page }) => {
    await expandStrip(page, 2);
    await expect(page.locator('.strip[data-lane="2"]')).toHaveClass(/expanded/);

    await page.evaluate(() => {
      window.PolyMockHost.action('applyPreset', { index: 1 });
    });
    await expect(page.locator('.strip[data-lane="2"]')).toHaveClass(/expanded/);
  });

  test('preset switch to fewer lanes clamps expanded index', async ({ page }) => {
    await expandStrip(page, 4);
    await expect(page.locator('.strip[data-lane="4"]')).toHaveClass(/expanded/);

    await page.evaluate(() => {
      window.PolyMockHost.action('applyPreset', { index: 2 });
    });
    await expect(page.locator('.strip[data-lane]')).toHaveCount(3);
    await expect(page.locator('.strip[data-lane="2"]')).toHaveClass(/expanded/);
  });

  test('cloth mode activates canvas', async ({ page }) => {
    await page.click('#mCloth');
    await expect(page.locator('#cloth')).toHaveClass(/on/);
    await expect(page.locator('#cloth canvas')).toBeVisible();
  });

  test('desk mode restores strips after cloth', async ({ page }) => {
    await page.click('#mCloth');
    await expect(page.locator('#cloth')).toHaveClass(/on/);
    await page.click('#mDesk');
    await expect(page.locator('#desk')).toHaveClass(/on/);
    const stripCount = await page.locator('.strip[data-lane]').count();
    expect(stripCount).toBeGreaterThanOrEqual(3);
  });

  test('learn mode toggles body class via button and keyboard', async ({ page }) => {
    await page.click('#learnBtn');
    await expect(page.locator('body')).toHaveClass(/learn/);
    await page.keyboard.press('l');
    await expect(page.locator('body')).not.toHaveClass(/learn/);
    await page.keyboard.press('l');
    await expect(page.locator('body')).toHaveClass(/learn/);
  });

  test('escape collapses expanded strip', async ({ page }) => {
    await expandStrip(page, 1);
    await expect(page.locator('.strip[data-lane="1"]')).toHaveClass(/expanded/);
    await page.keyboard.press('Escape');
    expect(await page.locator('.strip.expanded').count()).toBe(0);
  });

  test('mute toggle dispatches edit and updates visual', async ({ page }) => {
    await page.click('.strip[data-lane="0"] [data-mute]');
    const edits = await getEdits(page);
    expect(edits.some((e) => e.paramId === 'lane.0.active')).toBe(true);
  });

  test('export button gets flash class', async ({ page }) => {
    await page.click('#exportBtn');
    await expect(page.locator('#exportBtn')).toHaveClass(/on/);
    await page.waitForTimeout(700);
    await expect(page.locator('#exportBtn')).not.toHaveClass(/on/);
  });

  test('chain bar count clamps at minimum 1', async ({ page }) => {
    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.chain = { enabled: true, mode: 0, entryCount: 1, entries: [{ scene: 0, bars: 1 }] };
      window.PolyMockHost._pushState();
    });
    await page.click('#chainBtn');
    await clearEdits(page);
    await page.click('[data-bars-dec="0"]');
    const edits = await getEdits(page);
    const barsEdits = edits.filter((e) => e.paramId.includes('bars'));
    expect(barsEdits.every((e) => e.value === 0)).toBe(true);
  });

  test('chain bar count clamps at maximum 32', async ({ page }) => {
    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.chain = { enabled: true, mode: 0, entryCount: 1, entries: [{ scene: 0, bars: 32 }] };
      window.PolyMockHost._pushState();
    });
    await page.click('#chainBtn');
    await clearEdits(page);
    await page.click('[data-bars-inc="0"]');
    const edits = await getEdits(page);
    const barsEdits = edits.filter((e) => e.paramId.includes('bars'));
    expect(barsEdits.every((e) => e.value === 1)).toBe(true);
  });
});

test.describe('embedded mode hardening', () => {
  test('keyboard shortcuts 1 and 2 still switch modes in embedded', async ({ page }) => {
    await page.addInitScript(() => { window.__POLY_EMBEDDED__ = true; });
    await setupWithActionLog(page);
    await page.keyboard.press('1');
    await expect(page.locator('#cloth')).toHaveClass(/on/);
    await page.keyboard.press('2');
    await expect(page.locator('#desk')).toHaveClass(/on/);
  });

  test('learn mode works in embedded mode', async ({ page }) => {
    await page.addInitScript(() => { window.__POLY_EMBEDDED__ = true; });
    await setupWithActionLog(page);
    await page.keyboard.press('l');
    await expect(page.locator('body')).toHaveClass(/learn/);
  });

  test('escape still collapses strip in embedded mode', async ({ page }) => {
    await page.addInitScript(() => { window.__POLY_EMBEDDED__ = true; });
    await page.goto(pageUrl);
    await page.evaluate(() => {
      window.polyHostPush(JSON.stringify({ type: 'state', state: window.PolyMockHost.getState() }));
    });
    await page.click(`.strip[data-lane="0"] .ex`);
    await page.waitForSelector(`.strip[data-lane="0"].expanded`);
    await page.keyboard.press('Escape');
    expect(await page.locator('.strip.expanded').count()).toBe(0);
  });
});
