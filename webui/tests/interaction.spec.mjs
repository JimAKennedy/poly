import { test, expect } from '@playwright/test';
import { setupWithActionLog, getActions, clearActions, getEdits, clearEdits, expandStrip } from './test-helpers.mjs';

test.beforeEach(async ({ page }) => {
  await setupWithActionLog(page);
});

test.describe('chrome controls', () => {
  test('scene A/B buttons dispatch selectScene', async ({ page }) => {
    await page.click('#scB');
    let acts = await getActions(page);
    expect(acts).toContainEqual(
      expect.objectContaining({ name: 'selectScene', payload: { scene: 'B' } })
    );
    await page.click('#scA');
    acts = await getActions(page);
    expect(acts).toContainEqual(
      expect.objectContaining({ name: 'selectScene', payload: { scene: 'A' } })
    );
  });

  test('learn button toggles body class', async ({ page }) => {
    await expect(page.locator('body')).not.toHaveClass(/learn/);
    await page.click('#learnBtn');
    await expect(page.locator('body')).toHaveClass(/learn/);
    await page.click('#learnBtn');
    await expect(page.locator('body')).not.toHaveClass(/learn/);
  });

  test('L key toggles learn mode', async ({ page }) => {
    await page.keyboard.press('l');
    await expect(page.locator('body')).toHaveClass(/learn/);
    await page.keyboard.press('l');
    await expect(page.locator('body')).not.toHaveClass(/learn/);
  });

  test('1/2 keys switch cloth/desk modes', async ({ page }) => {
    await page.keyboard.press('1');
    await expect(page.locator('#cloth')).toHaveClass(/on/);
    await expect(page.locator('#desk')).not.toHaveClass(/on/);
    await page.keyboard.press('2');
    await expect(page.locator('#desk')).toHaveClass(/on/);
    await expect(page.locator('#cloth')).not.toHaveClass(/on/);
  });

  test('Escape closes expanded strip', async ({ page }) => {
    await expandStrip(page, 0);
    await expect(page.locator('.strip[data-lane="0"]')).toHaveClass(/expanded/);
    await page.keyboard.press('Escape');
    await expect(page.locator('.strip[data-lane="0"]')).not.toHaveClass(/expanded/);
  });
});

test.describe('collapsed strip', () => {
  test('timeline lane ladder buttons dispatch toggleStep', async ({ page }) => {
    const btn = page.locator('.strip[data-lane="0"] .ladder button').nth(2);
    await btn.click();
    const acts = await getActions(page);
    expect(acts).toContainEqual(
      expect.objectContaining({ name: 'toggleStep', payload: { lane: 0, step: 2 } })
    );
  });

  test('euclidean lane ladder buttons are display-only', async ({ page }) => {
    const btn = page.locator('.strip[data-lane="1"] .ladder button').first();
    const cursor = await btn.evaluate((el) => getComputedStyle(el).cursor);
    expect(cursor).toBe('default');
  });
});

test.describe('expanded strip — pattern tab', () => {
  test('steps stepper dispatches setEuclid', async ({ page }) => {
    await expandStrip(page, 1); // Kick E(4,4)
    const strip = page.locator('.strip[data-lane="1"]');

    await strip.locator('[data-st="1"]').click();
    let acts = await getActions(page);
    expect(acts).toContainEqual(
      expect.objectContaining({ name: 'setEuclid', payload: { lane: 1, steps: 5 } })
    );

    await clearActions(page);
    await strip.locator('[data-st="-1"]').click();
    acts = await getActions(page);
    expect(acts).toContainEqual(
      expect.objectContaining({ name: 'setEuclid', payload: { lane: 1, steps: 4 } })
    );
  });

  test('hits stepper updates pattern display', async ({ page }) => {
    await expandStrip(page, 2); // Snare E(2,3)
    const strip = page.locator('.strip[data-lane="2"]');

    await strip.locator('[data-ht="1"]').click();
    await expect(strip.locator('.stepper .v').nth(1)).toContainText('E(3,3)');

    await strip.locator('[data-ht="-1"]').click();
    await expect(strip.locator('.stepper .v').nth(1)).toContainText('E(2,3)');
  });

  test('rotation stepper dispatches setEuclid', async ({ page }) => {
    await expandStrip(page, 4); // Conga E(3,5) rot=0
    const strip = page.locator('.strip[data-lane="4"]');

    await strip.locator('[data-rt="1"]').click();
    const acts = await getActions(page);
    expect(acts).toContainEqual(
      expect.objectContaining({ name: 'setEuclid', payload: { lane: 4, rotation: 1 } })
    );
    await expect(strip.locator('.stepper .v').nth(2)).toContainText('1');
  });

  test('cells toggle enables additive mode', async ({ page }) => {
    await expandStrip(page, 4); // Conga
    const strip = page.locator('.strip[data-lane="4"]');

    await strip.locator('[data-cl]').click();
    const acts = await getActions(page);
    expect(acts).toContainEqual(
      expect.objectContaining({ name: 'setCells', payload: { lane: 4, cells: [2, 2, 3] } })
    );
    await expect(strip.locator('[data-pane="pattern"]')).toContainText('cycle = 7');
  });

  test('cell value click cycles size', async ({ page }) => {
    await expandStrip(page, 4);
    const strip = page.locator('.strip[data-lane="4"]');
    await strip.locator('[data-cl]').click(); // enable cells [2,2,3]
    await clearActions(page);

    await strip.locator('[data-cells] button[data-i="0"]').click();
    const acts = await getActions(page);
    const cellAction = acts.find((a) => a.name === 'setCells');
    expect(cellAction).toBeTruthy();
    expect(cellAction.payload.cells[0]).toBe(3); // 2→3
  });

  test('add cell button appends to array', async ({ page }) => {
    await expandStrip(page, 4);
    const strip = page.locator('.strip[data-lane="4"]');
    await strip.locator('[data-cl]').click(); // [2,2,3]
    await clearActions(page);

    await strip.locator('[data-addcell]').click();
    const acts = await getActions(page);
    const cellAction = acts.filter((a) => a.name === 'setCells').pop();
    expect(cellAction.payload.cells).toHaveLength(4);
  });

  test('timeline fixed step toggle dispatches setFixedStep', async ({ page }) => {
    await expandStrip(page, 0); // Bell: timeline mode
    const strip = page.locator('.strip[data-lane="0"]');

    // fixed[1] = 0, so toggling gives on: true
    await strip.locator('[data-fixed] button[data-i="1"]').click();
    const acts = await getActions(page);
    expect(acts).toContainEqual(
      expect.objectContaining({
        name: 'setFixedStep',
        payload: expect.objectContaining({ lane: 0, step: 1, on: true }),
      })
    );
  });
});

test.describe('expanded strip — timing tab', () => {
  test('micro-timing bar click dispatches setMicroTiming', async ({ page }) => {
    await expandStrip(page, 1); // Kick (4 steps)
    const strip = page.locator('.strip[data-lane="1"]');
    await strip.locator('[data-tab="timing"]').click();
    await expect(strip.locator('[data-pane="timing"]')).toHaveClass(/on/);

    const bar = strip.locator('.mb').first();
    const box = await bar.boundingBox();
    // Click in upper quarter: late = positive ms
    await bar.click({ position: { x: box.width / 2, y: box.height * 0.25 } });

    const acts = await getActions(page);
    const mtAction = acts.find((a) => a.name === 'setMicroTiming');
    expect(mtAction).toBeTruthy();
    expect(mtAction.payload.lane).toBe(1);
    expect(mtAction.payload.step).toBe(0);
    expect(mtAction.payload.ms).toBeGreaterThan(0);
  });
});

test.describe('expanded strip — envelope tab', () => {
  test('envelope on/off toggle dispatches setEnvelope', async ({ page }) => {
    await expandStrip(page, 0); // Bell: has Velocity envelope
    const strip = page.locator('.strip[data-lane="0"]');
    await strip.locator('[data-tab="env"]').click();

    await strip.locator('[data-envon="0"]').click();
    const acts = await getActions(page);
    const envAction = acts.find((a) => a.name === 'setEnvelope');
    expect(envAction).toBeTruthy();
    expect(envAction.payload.envelope.on).toBe(false);
  });

  test('add envelope dispatches setEnvelope with new entry', async ({ page }) => {
    await expandStrip(page, 1); // Kick: no envelopes
    const strip = page.locator('.strip[data-lane="1"]');
    await strip.locator('[data-tab="env"]').click();

    await strip.locator('.addenv').click();
    const acts = await getActions(page);
    const envAction = acts.find((a) => a.name === 'setEnvelope');
    expect(envAction).toBeTruthy();
    expect(envAction.payload.index).toBe(0);
    expect(envAction.payload.envelope.target).toBe('Velocity');
    expect(envAction.payload.envelope.on).toBe(true);
  });
});

test.describe('tab switching', () => {
  test('clicking tab shows corresponding pane', async ({ page }) => {
    await expandStrip(page, 0);
    const strip = page.locator('.strip[data-lane="0"]');

    await strip.locator('[data-tab="timing"]').click();
    await expect(strip.locator('[data-pane="timing"]')).toHaveClass(/on/);
    await expect(strip.locator('[data-pane="pattern"]')).not.toHaveClass(/on/);

    await strip.locator('[data-tab="env"]').click();
    await expect(strip.locator('[data-pane="env"]')).toHaveClass(/on/);
    await expect(strip.locator('[data-pane="timing"]')).not.toHaveClass(/on/);

    await strip.locator('[data-tab="pattern"]').click();
    await expect(strip.locator('[data-pane="pattern"]')).toHaveClass(/on/);
  });

  test('expr tab shows expression pane', async ({ page }) => {
    await expandStrip(page, 0);
    const strip = page.locator('.strip[data-lane="0"]');
    await strip.locator('[data-tab="expr"]').click();
    await expect(strip.locator('[data-pane="expr"]')).toHaveClass(/on/);
    await expect(strip.locator('[data-pane="pattern"]')).not.toHaveClass(/on/);
  });
});

test.describe('expanded strip — expression tab', () => {
  test('expression tab shows all parameter sliders', async ({ page }) => {
    await expandStrip(page, 1); // Kick
    const strip = page.locator('.strip[data-lane="1"]');
    await strip.locator('[data-tab="expr"]').click();

    const sliders = strip.locator('[data-pane="expr"] .param-slider');
    await expect(sliders).toHaveCount(9);

    const labels = await sliders.locator('label').allTextContents();
    expect(labels).toEqual([
      'Velocity', 'Probability', 'Ghost', 'Spread',
      'Swing', 'Humanize', 'Duration', 'Note', 'Channel',
    ]);
  });

  test('velocity slider shows correct initial value', async ({ page }) => {
    await expandStrip(page, 1); // Kick: vel=112
    const strip = page.locator('.strip[data-lane="1"]');
    await strip.locator('[data-tab="expr"]').click();

    const velValue = strip.locator('[data-pane="expr"] .param-slider').first().locator('.v');
    await expect(velValue).toHaveText('112');
  });

  test('dragging velocity slider fires edit with begin/perform/end', async ({ page }) => {
    await expandStrip(page, 1); // Kick
    const strip = page.locator('.strip[data-lane="1"]');
    await strip.locator('[data-tab="expr"]').click();

    const track = strip.locator('.slider-track[data-field="velocity"]');
    const box = await track.boundingBox();

    // Click at 60% position — fires begin + perform + end
    await track.click({ position: { x: box.width * 0.6, y: box.height / 2 } });

    const edits = await getEdits(page);
    const gestures = edits.map((e) => e.gesture);
    expect(edits[0].paramId).toBe('lane.1.velocity');
    expect(gestures).toContain('begin');
    expect(gestures).toContain('perform');
    expect(gestures).toContain('end');
    const performs = edits.filter((e) => e.gesture === 'perform');
    expect(performs[0].value).toBeGreaterThan(0.4);
    expect(performs[0].value).toBeLessThan(0.8);
  });

  test('probability slider round-trips through mock-host', async ({ page }) => {
    await expandStrip(page, 2); // Snare: prob=0.9
    const strip = page.locator('.strip[data-lane="2"]');
    await strip.locator('[data-tab="expr"]').click();

    const track = strip.locator('.slider-track[data-field="probability"]');
    const box = await track.boundingBox();

    // Click at 50% position
    await track.click({ position: { x: box.width * 0.5, y: box.height / 2 } });

    const state = await page.evaluate(() => window.PolyMockHost.getState());
    expect(state.lanes[2].prob).toBeCloseTo(0.5, 1);
  });

  test('ghost floor slider updates model', async ({ page }) => {
    await expandStrip(page, 4); // Conga: ghost=25
    const strip = page.locator('.strip[data-lane="4"]');
    await strip.locator('[data-tab="expr"]').click();

    const track = strip.locator('.slider-track[data-field="ghostFloor"]');
    const box = await track.boundingBox();
    await track.click({ position: { x: box.width * 0.5, y: box.height / 2 } });

    const state = await page.evaluate(() => window.PolyMockHost.getState());
    expect(state.lanes[4].ghost).toBeGreaterThan(50);
    expect(state.lanes[4].ghost).toBeLessThan(80);
  });

  test('swing slider updates model', async ({ page }) => {
    await expandStrip(page, 1); // Kick: swing=0
    const strip = page.locator('.strip[data-lane="1"]');
    await strip.locator('[data-tab="expr"]').click();

    const track = strip.locator('.slider-track[data-field="swing"]');
    const box = await track.boundingBox();
    await track.click({ position: { x: box.width * 0.3, y: box.height / 2 } });

    const state = await page.evaluate(() => window.PolyMockHost.getState());
    expect(state.lanes[1].swing).toBeCloseTo(0.3, 1);
  });

  test('note slider updates model', async ({ page }) => {
    await expandStrip(page, 1); // Kick: note=36
    const strip = page.locator('.strip[data-lane="1"]');
    await strip.locator('[data-tab="expr"]').click();

    const track = strip.locator('.slider-track[data-field="note"]');
    const box = await track.boundingBox();
    await track.click({ position: { x: box.width * 0.5, y: box.height / 2 } });

    const state = await page.evaluate(() => window.PolyMockHost.getState());
    expect(state.lanes[1].note).toBeGreaterThan(50);
    expect(state.lanes[1].note).toBeLessThan(80);
  });

  test('channel slider updates model', async ({ page }) => {
    await expandStrip(page, 1); // Kick: ch=2
    const strip = page.locator('.strip[data-lane="1"]');
    await strip.locator('[data-tab="expr"]').click();

    const track = strip.locator('.slider-track[data-field="channel"]');
    const box = await track.boundingBox();
    await track.click({ position: { x: box.width * 0.5, y: box.height / 2 } });

    const state = await page.evaluate(() => window.PolyMockHost.getState());
    expect(state.lanes[1].ch).toBe(8);
  });

  test('preset load resets expression tab values', async ({ page }) => {
    await expandStrip(page, 1); // Kick
    const strip = page.locator('.strip[data-lane="1"]');
    await strip.locator('[data-tab="expr"]').click();

    // Change velocity to ~50%
    const track = strip.locator('.slider-track[data-field="velocity"]');
    const box = await track.boundingBox();
    await track.click({ position: { x: box.width * 0.5, y: box.height / 2 } });

    // Apply Sparse Pulse preset (index 2)
    await page.evaluate(() => window.PolyMockHost.action('applyPreset', { index: 2 }));
    await page.waitForTimeout(50);

    // Re-expand (preset rebuild resets strips)
    await expandStrip(page, 0);
    await page.locator('.strip[data-lane="0"]').locator('[data-tab="expr"]').click();

    const velValue = page.locator('.strip[data-lane="0"] [data-pane="expr"] .param-slider').first().locator('.v');
    await expect(velValue).toHaveText('100'); // Sparse Pulse Kick vel=100
  });
});

test.describe('scene morph', () => {
  test('morph button dispatches selectScene Morph', async ({ page }) => {
    await page.click('#scM');
    const acts = await getActions(page);
    expect(acts).toContainEqual(
      expect.objectContaining({ name: 'selectScene', payload: { scene: 'Morph' } })
    );
  });

  test('morph button highlights and shows slider', async ({ page }) => {
    await page.click('#scM');
    await expect(page.locator('#scM')).toHaveClass(/\bon\b/);
    await expect(page.locator('#scA')).not.toHaveClass(/\bon\b/);
    await expect(page.locator('#morphSlider')).toBeVisible();
  });

  test('morph slider dispatches scene.morph edit', async ({ page }) => {
    await page.click('#scM');
    const track = page.locator('.morph-track');
    const box = await track.boundingBox();
    await track.click({ position: { x: box.width * 0.75, y: box.height / 2 } });

    const edits = await getEdits(page);
    const morphEdits = edits.filter(e => e.paramId === 'scene.morph');
    expect(morphEdits.length).toBeGreaterThanOrEqual(2);
    expect(morphEdits.some(e => e.gesture === 'begin')).toBe(true);
    expect(morphEdits.some(e => e.gesture === 'end')).toBe(true);
  });

  test('morph slider hidden for scene A', async ({ page }) => {
    await page.click('#scM');
    await expect(page.locator('#morphSlider')).toBeVisible();
    await page.click('#scA');
    await expect(page.locator('#morphSlider')).not.toBeVisible();
  });

  test('scene A/B/Morph highlighting is exclusive', async ({ page }) => {
    await page.click('#scB');
    await expect(page.locator('#scB')).toHaveClass(/\bon\b/);
    await expect(page.locator('#scA')).not.toHaveClass(/\bon\b/);
    await expect(page.locator('#scM')).not.toHaveClass(/\bon\b/);

    await page.click('#scM');
    await expect(page.locator('#scM')).toHaveClass(/\bon\b/);
    await expect(page.locator('#scA')).not.toHaveClass(/\bon\b/);
    await expect(page.locator('#scB')).not.toHaveClass(/\bon\b/);
  });
});

test.describe('chain popover', () => {
  test('chain button opens popover', async ({ page }) => {
    await page.click('#chainBtn');
    await expect(page.locator('#chainPopover')).toBeVisible();
    await expect(page.locator('#chainPopover h4')).toHaveText('Scene Chain');
  });

  test('enable toggle dispatches chain.enabled edit', async ({ page }) => {
    await page.click('#chainBtn');
    await page.click('[data-chain-enable]');
    const edits = await getEdits(page);
    const enableEdits = edits.filter(e => e.paramId === 'chain.enabled');
    expect(enableEdits.some(e => e.value === 1)).toBe(true);
  });

  test('mode buttons dispatch chain.mode edit', async ({ page }) => {
    await page.click('#chainBtn');
    await page.click('[data-chain-mode="1"]');
    const edits = await getEdits(page);
    const modeEdits = edits.filter(e => e.paramId === 'chain.mode');
    expect(modeEdits.some(e => e.value === 0.5)).toBe(true);
  });

  test('add entry creates chain entry row', async ({ page }) => {
    await page.click('#chainBtn');
    await page.click('[data-chain-add]');
    const state = await page.evaluate(() => window.PolyMockHost.getState());
    expect(state.chain.entryCount).toBe(1);
    expect(state.chain.entries[0]).toEqual({ scene: 0, bars: 4 });
    await expect(page.locator('.chain-entry')).toHaveCount(1);
  });

  test('entry scene button dispatches chain.entry.N.scene edit', async ({ page }) => {
    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.chain.entries.push({ scene: 0, bars: 4 });
      s.chain.entryCount = 1;
      window.PolyMockHost._pushState();
    });
    await page.click('#chainBtn');
    await page.click('[data-entry-scene="0"][data-sv="1"]');
    const edits = await getEdits(page);
    expect(edits.some(e => e.paramId === 'chain.entry.0.scene' && e.value === 0.5)).toBe(true);
  });

  test('entry bars increment dispatches chain.entry.N.bars edit', async ({ page }) => {
    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.chain.entries.push({ scene: 0, bars: 4 });
      s.chain.entryCount = 1;
      window.PolyMockHost._pushState();
    });
    await page.click('#chainBtn');
    await page.click('[data-bars-inc="0"]');
    const state = await page.evaluate(() => window.PolyMockHost.getState());
    expect(state.chain.entries[0].bars).toBe(5);
  });

  test('remove entry removes chain entry', async ({ page }) => {
    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.chain.entries.push({ scene: 0, bars: 4 }, { scene: 1, bars: 2 });
      s.chain.entryCount = 2;
      window.PolyMockHost._pushState();
    });
    await page.click('#chainBtn');
    await expect(page.locator('.chain-entry')).toHaveCount(2);
    await page.click('[data-rm="0"]');
    const state = await page.evaluate(() => window.PolyMockHost.getState());
    expect(state.chain.entryCount).toBe(1);
    expect(state.chain.entries[0].scene).toBe(1);
  });

  test('preset load resets chain state', async ({ page }) => {
    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.chain.enabled = true;
      s.chain.entries.push({ scene: 0, bars: 4 });
      s.chain.entryCount = 1;
      s.scene = 'Morph';
      window.PolyMockHost._pushState();
    });
    await page.evaluate(() => window.PolyMockHost.action('applyPreset', { index: -1 }));
    await page.waitForTimeout(50);
    const state = await page.evaluate(() => window.PolyMockHost.getState());
    expect(state.chain.enabled).toBe(false);
    expect(state.chain.entryCount).toBe(0);
    expect(state.scene).toBe('A');
  });
});

/* ================= S05: Phrase, Mutation & Advanced ================= */

test.describe('lane mute toggle', () => {
  test('mute button dispatches lane.N.active edit', async ({ page }) => {
    await clearEdits(page);
    await page.click('.strip[data-lane="0"] [data-mute]');
    const edits = await getEdits(page);
    expect(edits.some(e => e.paramId === 'lane.0.active' && e.value === 0)).toBe(true);
  });

  test('mute button toggles off class and strip muted', async ({ page }) => {
    await page.click('.strip[data-lane="0"] [data-mute]');
    await page.waitForTimeout(50);
    await expect(page.locator('.strip[data-lane="0"]')).toHaveClass(/muted/);
    await expect(page.locator('.strip[data-lane="0"] [data-mute]')).toHaveClass(/off/);
  });
});

test.describe('advanced tab — phrase', () => {
  test('phrase length slider updates model', async ({ page }) => {
    await expandStrip(page, 0);
    await page.click('.strip[data-lane="0"] [data-tab="adv"]');
    await clearEdits(page);
    const track = page.locator('.strip[data-lane="0"] [data-pane="adv"] .slider-track[data-field="phraseLength"]');
    const box = await track.boundingBox();
    await page.mouse.click(box.x + box.width * 0.5, box.y + box.height / 2);
    const edits = await getEdits(page);
    expect(edits.some(e => e.paramId === 'lane.0.phraseLength')).toBe(true);
    const state = await page.evaluate(() => window.PolyMockHost.getState());
    expect(state.lanes[0].phraseLength).toBeGreaterThan(0);
  });

  test('phrase gap slider updates model', async ({ page }) => {
    await expandStrip(page, 0);
    await page.click('.strip[data-lane="0"] [data-tab="adv"]');
    await clearEdits(page);
    const track = page.locator('.strip[data-lane="0"] [data-pane="adv"] .slider-track[data-field="phraseGap"]');
    const box = await track.boundingBox();
    await page.mouse.click(box.x + box.width * 0.5, box.y + box.height / 2);
    const edits = await getEdits(page);
    expect(edits.some(e => e.paramId === 'lane.0.phraseGap')).toBe(true);
    const state = await page.evaluate(() => window.PolyMockHost.getState());
    expect(state.lanes[0].phraseGap).toBeGreaterThan(0);
  });

  test('phrase offset slider updates model', async ({ page }) => {
    await expandStrip(page, 0);
    await page.click('.strip[data-lane="0"] [data-tab="adv"]');
    await clearEdits(page);
    const track = page.locator('.strip[data-lane="0"] [data-pane="adv"] .slider-track[data-field="phraseOffset"]');
    const box = await track.boundingBox();
    await page.mouse.click(box.x + box.width * 0.75, box.y + box.height / 2);
    const edits = await getEdits(page);
    expect(edits.some(e => e.paramId === 'lane.0.phraseOffset')).toBe(true);
    const state = await page.evaluate(() => window.PolyMockHost.getState());
    expect(state.lanes[0].phraseOffset).toBeGreaterThan(0);
  });
});

test.describe('advanced tab — mutation', () => {
  test('mutation rate slider updates model', async ({ page }) => {
    await expandStrip(page, 0);
    await page.click('.strip[data-lane="0"] [data-tab="adv"]');
    await clearEdits(page);
    const track = page.locator('.strip[data-lane="0"] [data-pane="adv"] .slider-track[data-field="mutationRate"]');
    const box = await track.boundingBox();
    await page.mouse.click(box.x + box.width * 0.6, box.y + box.height / 2);
    const edits = await getEdits(page);
    expect(edits.some(e => e.paramId === 'lane.0.mutationRate')).toBe(true);
    const state = await page.evaluate(() => window.PolyMockHost.getState());
    expect(state.lanes[0].mutationRate).toBeGreaterThan(0);
  });

  test('drift rate slider updates model', async ({ page }) => {
    await expandStrip(page, 0);
    await page.click('.strip[data-lane="0"] [data-tab="adv"]');
    await clearEdits(page);
    const track = page.locator('.strip[data-lane="0"] [data-pane="adv"] .slider-track[data-field="driftRate"]');
    const box = await track.boundingBox();
    await page.mouse.click(box.x + box.width * 0.75, box.y + box.height / 2);
    const edits = await getEdits(page);
    expect(edits.some(e => e.paramId === 'lane.0.driftRate')).toBe(true);
    const state = await page.evaluate(() => window.PolyMockHost.getState());
    expect(state.lanes[0].driftRate).not.toBe(0);
  });
});

test.describe('advanced tab — more controls', () => {
  test('subdivision chip dispatches edit', async ({ page }) => {
    await expandStrip(page, 0);
    await page.click('.strip[data-lane="0"] [data-tab="adv"]');
    await clearEdits(page);
    await page.click('.strip[data-lane="0"] [data-pane="adv"] [data-sub="8"]');
    const edits = await getEdits(page);
    expect(edits.some(e => e.paramId === 'lane.0.subdivision' && e.value === 0.75)).toBe(true);
    const state = await page.evaluate(() => window.PolyMockHost.getState());
    expect(state.lanes[0].subdivision).toBe(8);
  });

  test('emphasis slider updates model', async ({ page }) => {
    await expandStrip(page, 0);
    await page.click('.strip[data-lane="0"] [data-tab="adv"]');
    await clearEdits(page);
    const track = page.locator('.strip[data-lane="0"] [data-pane="adv"] .slider-track[data-field="emphasisProb"]');
    const box = await track.boundingBox();
    await page.mouse.click(box.x + box.width * 0.5, box.y + box.height / 2);
    const edits = await getEdits(page);
    expect(edits.some(e => e.paramId === 'lane.0.emphasisProb')).toBe(true);
  });

  test('timing offset slider updates model', async ({ page }) => {
    await expandStrip(page, 0);
    await page.click('.strip[data-lane="0"] [data-tab="adv"]');
    await clearEdits(page);
    const track = page.locator('.strip[data-lane="0"] [data-pane="adv"] .slider-track[data-field="timingOffset"]');
    const box = await track.boundingBox();
    await page.mouse.click(box.x + box.width * 0.8, box.y + box.height / 2);
    const edits = await getEdits(page);
    expect(edits.some(e => e.paramId === 'lane.0.timingOffset')).toBe(true);
  });

  test('kotekan chip dispatches edit', async ({ page }) => {
    await expandStrip(page, 0);
    await page.click('.strip[data-lane="0"] [data-tab="adv"]');
    await clearEdits(page);
    await page.click('.strip[data-lane="0"] [data-pane="adv"] [data-kot="1"]');
    const edits = await getEdits(page);
    expect(edits.some(e => e.paramId === 'lane.0.kotekanSource' && e.value === 0.25)).toBe(true);
    const state = await page.evaluate(() => window.PolyMockHost.getState());
    expect(state.lanes[0].kotekanSource).toBe(1);
  });

  test('kotekan None chip resets source', async ({ page }) => {
    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.lanes[0].kotekanSource = 2;
      window.PolyMockHost._pushState();
    });
    await expandStrip(page, 0);
    await page.click('.strip[data-lane="0"] [data-tab="adv"]');
    await clearEdits(page);
    await page.click('.strip[data-lane="0"] [data-pane="adv"] [data-kot="-1"]');
    const state = await page.evaluate(() => window.PolyMockHost.getState());
    expect(state.lanes[0].kotekanSource).toBe(-1);
  });

  test('preset load resets advanced params', async ({ page }) => {
    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.lanes[0].phraseLength = 16;
      s.lanes[0].mutationRate = 0.5;
      s.lanes[0].kotekanSource = 2;
      window.PolyMockHost._pushState();
    });
    await page.evaluate(() => window.PolyMockHost.action('applyPreset', { index: -1 }));
    await page.waitForTimeout(50);
    const state = await page.evaluate(() => window.PolyMockHost.getState());
    expect(state.lanes[0].phraseLength).toBe(0);
    expect(state.lanes[0].mutationRate).toBe(0);
    expect(state.lanes[0].kotekanSource).toBe(-1);
  });
});
