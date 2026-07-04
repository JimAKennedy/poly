import { test, expect } from '@playwright/test';
import {
  setupWithActionLog,
  getActions,
  clearActions,
  expandStrip,
  startContinuousPush,
  stopContinuousPush,
} from './test-helpers.mjs';

test.beforeEach(async ({ page }) => {
  await setupWithActionLog(page);
});

test.afterEach(async ({ page }) => {
  await stopContinuousPush(page);
});

test.describe('controls work during rapid state pushes', () => {
  test('timeline step toggle', async ({ page }) => {
    await startContinuousPush(page, 16); // ~60Hz

    const btn = page.locator('.strip[data-lane="0"] .ladder button').nth(2);
    await expect(btn).toHaveClass(/hit/);
    await btn.click();

    const acts = await getActions(page);
    expect(acts.some((a) => a.name === 'toggleStep' && a.payload.step === 2)).toBe(true);
    await expect(page.locator('.strip[data-lane="0"] .ladder button').nth(2)).not.toHaveClass(
      /hit/
    );
  });

  test('strip expand and collapse', async ({ page }) => {
    await startContinuousPush(page, 16);

    await expandStrip(page, 1);
    await expect(page.locator('.strip[data-lane="1"]')).toHaveClass(/expanded/);
    await expect(page.locator('.strip[data-lane="1"] [data-pane="pattern"]')).toBeVisible();

    await page.keyboard.press('Escape');
    await expect(page.locator('.strip[data-lane="1"]')).not.toHaveClass(/expanded/);
  });

  test('euclid stepper', async ({ page }) => {
    await expandStrip(page, 2); // Snare E(2,3)
    await startContinuousPush(page, 16);

    const strip = page.locator('.strip[data-lane="2"]');
    await strip.locator('[data-ht="1"]').click();

    const acts = await getActions(page);
    expect(acts.some((a) => a.name === 'setEuclid' && a.payload.hits === 3)).toBe(true);
    await expect(strip.locator('[data-pane="pattern"]')).toContainText('E(3,3)');
  });

  test('mode switch', async ({ page }) => {
    await startContinuousPush(page, 16);

    await page.click('#mCloth');
    await expect(page.locator('#cloth')).toHaveClass(/on/);

    await page.click('#mDesk');
    await expect(page.locator('#desk')).toHaveClass(/on/);
  });

  test('scene switch', async ({ page }) => {
    await startContinuousPush(page, 16);

    await page.click('#scB');
    const acts = await getActions(page);
    expect(acts.some((a) => a.name === 'selectScene' && a.payload.scene === 'B')).toBe(true);
    await expect(page.locator('#scB')).toHaveClass(/on/);
  });

  test('envelope toggle', async ({ page }) => {
    await expandStrip(page, 0); // Bell: has envelope
    const strip = page.locator('.strip[data-lane="0"]');
    await strip.locator('[data-tab="env"]').click();

    await startContinuousPush(page, 16);

    await strip.locator('[data-envon="0"]').click();
    const acts = await getActions(page);
    expect(acts.some((a) => a.name === 'setEnvelope')).toBe(true);
  });

  test('cells toggle', async ({ page }) => {
    await expandStrip(page, 4); // Conga
    await startContinuousPush(page, 16);

    const strip = page.locator('.strip[data-lane="4"]');
    await strip.locator('[data-cl]').click();

    const acts = await getActions(page);
    expect(acts.some((a) => a.name === 'setCells')).toBe(true);
    await expect(strip.locator('[data-pane="pattern"]')).toContainText('cycle = 7');
  });
});

test.describe('DOM stability', () => {
  test('duplicate state pushes do not recreate DOM elements', async ({ page }) => {
    // Mark a button so we can detect if it was recreated
    await page.evaluate(() => {
      document.querySelector('.strip[data-lane="0"] .ladder button').__testMark = true;
    });

    // Push identical state 10 times
    for (let i = 0; i < 10; i++) {
      await page.evaluate(() => window.PolyMockHost._pushState());
    }

    const survived = await page.evaluate(
      () => document.querySelector('.strip[data-lane="0"] .ladder button').__testMark === true
    );
    expect(survived).toBe(true);
  });

  test('expanded strip controls survive unchanged state push', async ({ page }) => {
    await expandStrip(page, 2); // Snare
    const strip = page.locator('.strip[data-lane="2"]');

    await page.evaluate(() => {
      document.querySelector('.strip[data-lane="2"] [data-ht="1"]').__testMark = true;
    });

    await page.evaluate(() => window.PolyMockHost._pushState());

    const survived = await page.evaluate(
      () => document.querySelector('.strip[data-lane="2"] [data-ht="1"]').__testMark === true
    );
    expect(survived).toBe(true);

    // Controls still respond after the push
    await strip.locator('[data-ht="1"]').click();
    const acts = await getActions(page);
    expect(acts.some((a) => a.name === 'setEuclid')).toBe(true);
  });

  test('rapid sequential clicks on same button all register', async ({ page }) => {
    const btn = page.locator('.strip[data-lane="0"] .ladder button').nth(5);
    await btn.click();
    await btn.click();
    await btn.click();

    const acts = await getActions(page);
    const toggleActs = acts.filter((a) => a.name === 'toggleStep' && a.payload.step === 5);
    expect(toggleActs.length).toBe(3);
  });
});
