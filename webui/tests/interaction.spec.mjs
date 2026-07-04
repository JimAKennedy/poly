import { test, expect } from '@playwright/test';
import { setupWithActionLog, getActions, clearActions, expandStrip } from './test-helpers.mjs';

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
});
