import { test, expect } from '@playwright/test';
import { setupWithActionLog, expandStrip, pageUrl } from './test-helpers.mjs';

test.describe('async round-trip verification', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
    await page.evaluate(() => window.PolyMockHost.setAsyncMode(true));
  });

  test.afterEach(async ({ page }) => {
    await page.evaluate(() => window.PolyMockHost.setAsyncMode(false));
  });

  test('macro edit does not update UI until state push arrives', async ({ page }) => {
    const macro = page.locator('.macro', { has: page.locator('.t span', { hasText: 'Complexity' }) });
    const valueBefore = await macro.locator('.t b').textContent();

    await page.evaluate(() => {
      window.PolyMockHost.edit('macro.complexity', 0.99, 'perform');
    });
    const valueAfterEdit = await macro.locator('.t b').textContent();
    expect(valueAfterEdit).toBe(valueBefore);

    const pending = await page.evaluate(() => window.PolyMockHost.hasPendingPush());
    expect(pending).toBe(true);

    await page.evaluate(() => window.PolyMockHost.flushState());
    await expect(macro.locator('.t b')).toHaveText('99');
  });

  test('setEuclid action does not update pattern until state push', async ({ page }) => {
    // Use lane 1 (Kick, euclidean mode) — lane 0 is timeline mode in Afrobeat preset
    const strip1 = page.locator('.strip[data-lane="1"]');
    const hitsBefore = await strip1.locator('.ladder button.hit').count();
    expect(hitsBefore).toBe(4);

    await page.evaluate(() => {
      window.PolyMockHost.action('setEuclid', { lane: 1, hits: 1 });
    });
    const hitsAfterAction = await strip1.locator('.ladder button.hit').count();
    expect(hitsAfterAction).toBe(hitsBefore);

    await page.evaluate(() => window.PolyMockHost.flushState());
    await expect(strip1.locator('.ladder button.hit')).toHaveCount(1);
  });

  test('preset switch does not update UI until state push', async ({ page }) => {
    // Afrobeat 12/8 has 5 lanes; Sparse Pulse (index 2) has 3
    await expect(page.locator('.strip')).toHaveCount(5);

    await page.evaluate(() => {
      window.PolyMockHost.action('applyPreset', { index: 2 });
    });
    // UI should still show 5 lanes (state push deferred)
    const laneCountAfterAction = await page.locator('.strip').count();
    expect(laneCountAfterAction).toBe(5);

    await page.evaluate(() => window.PolyMockHost.flushState());
    await expect(page.locator('.strip')).toHaveCount(3);
  });

  test('lane expression edit round-trip via edit()', async ({ page }) => {
    await expandStrip(page, 0);

    await page.evaluate(() => {
      window.PolyMockHost.edit('lane.0.velocity', 0.25, 'perform');
    });

    const pending = await page.evaluate(() => window.PolyMockHost.hasPendingPush());
    expect(pending).toBe(true);

    const modelVel = await page.evaluate(() => window.PolyMockHost.getState().lanes[0].vel);
    expect(modelVel).toBe(Math.round(0.25 * 127));

    await page.evaluate(() => window.PolyMockHost.flushState());
    await page.waitForTimeout(50);

    const stateAfterFlush = await page.evaluate(() => window.PolyMockHost.getState().lanes[0].vel);
    expect(stateAfterFlush).toBe(Math.round(0.25 * 127));
  });

  test('multiple deferred edits coalesce into single state push', async ({ page }) => {
    let pushCount = 0;
    await page.evaluate(() => {
      window.__roundtripPushCount = 0;
      window.PolyMockHost.onState(() => { window.__roundtripPushCount++; });
    });

    await page.evaluate(() => {
      window.PolyMockHost.edit('macro.complexity', 0.1, 'perform');
      window.PolyMockHost.edit('macro.density', 0.2, 'perform');
      window.PolyMockHost.edit('macro.swing', 0.3, 'perform');
    });

    pushCount = await page.evaluate(() => window.__roundtripPushCount);
    expect(pushCount).toBe(0);

    await page.evaluate(() => window.PolyMockHost.flushState());
    pushCount = await page.evaluate(() => window.__roundtripPushCount);
    expect(pushCount).toBe(1);

    await expect(page.locator('.macro', { has: page.locator('.t span', { hasText: 'Complexity' }) }).locator('.t b')).toHaveText('10');
    await expect(page.locator('.macro', { has: page.locator('.t span', { hasText: 'Density' }) }).locator('.t b')).toHaveText('20');
    await expect(page.locator('.macro', { has: page.locator('.t span', { hasText: 'Swing' }) }).locator('.t b')).toHaveText('30');
  });
});
