import { test, expect } from '@playwright/test';
import { setupWithActionLog, expandStrip, pageUrl } from './test-helpers.mjs';
import path from 'node:path';

const SHOTS_DIR = path.resolve('test-results', 'screenshots');

test.describe('screenshot captures', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
  });

  test('desk — all lanes collapsed', async ({ page }) => {
    await expect(page.locator('#desk')).toHaveClass(/on/);
    await expect(page.locator('.strip[data-lane]')).toHaveCount(5);
    await page.screenshot({ path: path.join(SHOTS_DIR, 'desk-collapsed.png'), fullPage: true });
  });

  test('desk — lane expanded (pattern tab)', async ({ page }) => {
    await expandStrip(page, 0);
    await expect(page.locator('.strip[data-lane="0"]')).toHaveClass(/expanded/);
    await page.screenshot({ path: path.join(SHOTS_DIR, 'desk-expanded-pattern.png'), fullPage: true });
  });

  test('desk — lane expanded (cells mode)', async ({ page }) => {
    await expandStrip(page, 4);
    const strip = page.locator('.strip[data-lane="4"]');
    const clBtn = strip.locator('[data-cl]');
    await clBtn.click();
    await expect(clBtn).toHaveClass(/on/);
    await page.screenshot({ path: path.join(SHOTS_DIR, 'desk-expanded-cells.png'), fullPage: true });
  });

  test('cloth view', async ({ page }) => {
    await page.click('#mCloth');
    await expect(page.locator('#cloth')).toHaveClass(/on/);
    await page.screenshot({ path: path.join(SHOTS_DIR, 'cloth-view.png'), fullPage: true });
  });

  test('desk — 3 lanes after state change', async ({ page }) => {
    await page.evaluate(() => {
      const s = window.PolyMockHost.getState();
      s.lanes = s.lanes.slice(0, 3);
      window.PolyMockHost._pushState();
    });
    await expect(page.locator('.strip[data-lane]')).toHaveCount(3);
    await page.screenshot({ path: path.join(SHOTS_DIR, 'desk-3-lanes.png'), fullPage: true });
  });

  test('embedded mode', async ({ page }) => {
    await page.addInitScript(() => { window.__POLY_EMBEDDED__ = true; });
    await page.goto(pageUrl);
    await page.evaluate(() => {
      window.polyHostPush(JSON.stringify({ type: 'state', state: window.PolyMockHost.getState() }));
    });
    await page.screenshot({ path: path.join(SHOTS_DIR, 'embedded-mode.png'), fullPage: true });
  });

  test('master macros section', async ({ page }) => {
    const master = page.locator('#master');
    await expect(master).toBeVisible();
    await master.screenshot({ path: path.join(SHOTS_DIR, 'master-macros.png') });
  });

  test('lane head layout consistency', async ({ page }) => {
    const strips = page.locator('.strip[data-lane]');
    const count = await strips.count();
    const offsets = [];
    for (let i = 0; i < count; i++) {
      const strip = strips.nth(i);
      const stripBox = await strip.boundingBox();
      const btnBox = await strip.locator('.ex').boundingBox();
      if (stripBox && btnBox) {
        offsets.push(stripBox.x + stripBox.width - (btnBox.x + btnBox.width));
      }
    }
    const maxDrift = Math.max(...offsets) - Math.min(...offsets);
    expect(maxDrift).toBeLessThan(2);
    await page.screenshot({ path: path.join(SHOTS_DIR, 'lane-heads.png'), fullPage: true });
  });
});
