import { test, expect } from '@playwright/test';
import { setupWithActionLog, getEdits, clearEdits } from './test-helpers.mjs';

// M053 S07 (F1): the WebUI desk shipped with no way to add or remove lanes — the
// user was stuck with whatever a preset brought, even though the activeLaneCount
// param (1..8) is wired end-to-end through the bridge. These specs prove the new
// Desk-footer control edits activeLaneCount and the desk grows/shrinks to match.

test.describe('F1 — lane add/remove', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
    // Default preset (Afrobeat 12/8) ships 5 lanes; Desk is the default mode.
    await expect(page.locator('#desk .strip')).toHaveCount(5);
    await expect(page.locator('.lane-mgr .lane-count')).toHaveText('5 / 8 lanes');
  });

  test('Add Lane grows the desk and edits activeLaneCount', async ({ page }) => {
    await clearEdits(page);
    await page.click('.lane-mgr .lane-add');
    const edits = await getEdits(page);
    expect(edits.some((e) => e.paramId === 'activeLaneCount' && e.gesture === 'end')).toBe(true);
    await expect(page.locator('#desk .strip')).toHaveCount(6);
    await expect(page.locator('.lane-mgr .lane-count')).toHaveText('6 / 8 lanes');
    // Host truth agrees — the shipping bridge serializes exactly this many lanes.
    expect(await page.evaluate(() => window.PolyMockHost.getState().lanes.length)).toBe(6);
  });

  test('Remove Lane shrinks the desk', async ({ page }) => {
    await page.click('.lane-mgr .lane-del');
    await expect(page.locator('#desk .strip')).toHaveCount(4);
    await expect(page.locator('.lane-mgr .lane-count')).toHaveText('4 / 8 lanes');
  });

  test('bounds: 8 lanes disables Add, 1 lane disables Remove', async ({ page }) => {
    for (let i = 0; i < 3; i++) await page.click('.lane-mgr .lane-add');
    await expect(page.locator('#desk .strip')).toHaveCount(8);
    await expect(page.locator('.lane-mgr .lane-add')).toBeDisabled();

    for (let i = 0; i < 7; i++) await page.click('.lane-mgr .lane-del');
    await expect(page.locator('#desk .strip')).toHaveCount(1);
    await expect(page.locator('.lane-mgr .lane-del')).toBeDisabled();
  });
});
