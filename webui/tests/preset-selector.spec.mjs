import { test, expect } from '@playwright/test';
import { setupWithActionLog, getActions, clearActions } from './test-helpers.mjs';

test.beforeEach(async ({ page }) => {
  await setupWithActionLog(page);
});

test.describe('preset dropdown', () => {
  test('default state boots with Afrobeat 12/8', async ({ page }) => {
    await expect(page.locator('#presetName')).toHaveText('Afrobeat 12/8');
    await expect(page.locator('#seedVal')).toHaveText('88');
    await expect(page.locator('.strip[data-lane]')).toHaveCount(5);
  });

  test('clicking preset button opens menu', async ({ page }) => {
    await expect(page.locator('#presetMenu')).not.toHaveClass(/open/);
    await page.click('#presetName');
    await expect(page.locator('#presetMenu')).toHaveClass(/open/);
    await expect(page.locator('#presetName')).toHaveAttribute('aria-expanded', 'true');
  });

  test('menu lists Init plus 43 factory presets', async ({ page }) => {
    await page.click('#presetName');
    const options = page.locator('#presetMenu [role="option"]');
    await expect(options).toHaveCount(44);
    await expect(options.first()).toHaveText('Init (All Lanes)');
    await expect(options.nth(1)).toContainText('Four on the Floor');
    await expect(options.last()).toContainText('Compositional Arc');
  });

  test('separator exists between Init and factory presets', async ({ page }) => {
    await page.click('#presetName');
    await expect(page.locator('#presetMenu .sep')).toHaveCount(1);
  });

  test('current preset is marked active', async ({ page }) => {
    await page.click('#presetName');
    const active = page.locator('#presetMenu [role="option"].active');
    await expect(active).toHaveCount(1);
    await expect(active).toContainText('Afrobeat 12/8');
  });

  test('clicking outside closes menu', async ({ page }) => {
    await page.click('#presetName');
    await expect(page.locator('#presetMenu')).toHaveClass(/open/);
    await page.click('#chrome');
    await expect(page.locator('#presetMenu')).not.toHaveClass(/open/);
    await expect(page.locator('#presetName')).toHaveAttribute('aria-expanded', 'false');
  });

  test('Escape closes menu', async ({ page }) => {
    await page.click('#presetName');
    await expect(page.locator('#presetMenu')).toHaveClass(/open/);
    await page.keyboard.press('Escape');
    await expect(page.locator('#presetMenu')).not.toHaveClass(/open/);
  });

  test('second click on button toggles menu closed', async ({ page }) => {
    await page.click('#presetName');
    await expect(page.locator('#presetMenu')).toHaveClass(/open/);
    await page.click('#presetName');
    await expect(page.locator('#presetMenu')).not.toHaveClass(/open/);
  });
});

test.describe('preset selection', () => {
  test('selecting Four on the Floor changes state', async ({ page }) => {
    await page.click('#presetName');
    await page.click('#presetMenu [role="option"][data-index="0"]');
    await expect(page.locator('#presetMenu')).not.toHaveClass(/open/);
    await expect(page.locator('#presetName')).toHaveText('Four on the Floor');
    await expect(page.locator('#seedVal')).toHaveText('1');
    await expect(page.locator('.strip[data-lane]')).toHaveCount(4);
  });

  test('selecting Four on the Floor dispatches applyPreset action', async ({ page }) => {
    await clearActions(page);
    await page.click('#presetName');
    await page.click('#presetMenu [role="option"][data-index="0"]');
    const acts = await getActions(page);
    expect(acts).toContainEqual(
      expect.objectContaining({ name: 'applyPreset', payload: { index: 0 } })
    );
  });

  test('selecting Sparse Pulse shows 3 lanes', async ({ page }) => {
    await page.click('#presetName');
    await page.click('#presetMenu [role="option"][data-index="2"]');
    await expect(page.locator('#presetName')).toHaveText('Sparse Pulse');
    await expect(page.locator('.strip[data-lane]')).toHaveCount(3);
  });

  test('selecting IDM Glitch shows 5 lanes with cells', async ({ page }) => {
    await page.click('#presetName');
    await page.click('#presetMenu [role="option"][data-index="13"]');
    await expect(page.locator('#presetName')).toHaveText('IDM Glitch');
    await expect(page.locator('.strip[data-lane]')).toHaveCount(5);
    await expect(page.locator('#seedVal')).toHaveText('99');
  });

  test('Init resets to default state', async ({ page }) => {
    await page.click('#presetName');
    await page.click('#presetMenu [role="option"][data-index="-1"]');
    await expect(page.locator('#presetName')).toHaveText('Init');
    await expect(page.locator('#seedVal')).toHaveText('0');
    await expect(page.locator('.strip[data-lane]')).toHaveCount(4);
  });

  test('switching presets updates macros', async ({ page }) => {
    await page.click('#presetName');
    await page.click('#presetMenu [role="option"][data-index="0"]');
    const complexity = await page.locator('.macro .t b').first().textContent();
    expect(complexity).toBe('50');
  });

  test('active marker follows selection', async ({ page }) => {
    await page.click('#presetName');
    await page.click('#presetMenu [role="option"][data-index="2"]');
    await page.click('#presetName');
    const active = page.locator('#presetMenu [role="option"].active');
    await expect(active).toHaveCount(1);
    await expect(active).toContainText('Sparse Pulse');
  });

  test('lane names update after preset change', async ({ page }) => {
    await page.click('#presetName');
    await page.click('#presetMenu [role="option"][data-index="0"]');
    await expect(page.locator('.strip[data-lane="0"] .nm b')).toHaveText('Kick');
    await expect(page.locator('.strip[data-lane="1"] .nm b')).toHaveText('Snare');
    await expect(page.locator('.strip[data-lane="2"] .nm b')).toHaveText('Hi-Hat');
    await expect(page.locator('.strip[data-lane="3"] .nm b')).toHaveText('Open Hat');
  });

  test('rapid preset switching settles correctly', async ({ page }) => {
    await page.click('#presetName');
    await page.click('#presetMenu [role="option"][data-index="0"]');
    await page.click('#presetName');
    await page.click('#presetMenu [role="option"][data-index="2"]');
    await page.click('#presetName');
    await page.click('#presetMenu [role="option"][data-index="13"]');
    await expect(page.locator('#presetName')).toHaveText('IDM Glitch');
    await expect(page.locator('.strip[data-lane]')).toHaveCount(5);
  });
});

test.describe('preset with expanded strip', () => {
  test('switching preset rebuilds strips while keeping expand index', async ({ page }) => {
    await page.click('.strip[data-lane="0"] .ex');
    await expect(page.locator('.strip[data-lane="0"]')).toHaveClass(/expanded/);
    await page.click('#presetName');
    await page.click('#presetMenu [role="option"][data-index="0"]');
    await expect(page.locator('.strip[data-lane="0"]')).toHaveClass(/expanded/);
    await expect(page.locator('.strip[data-lane="0"] .nm b')).toHaveText('Kick');
  });

  test('strip can be re-expanded after preset change', async ({ page }) => {
    await page.click('#presetName');
    await page.click('#presetMenu [role="option"][data-index="0"]');
    await page.click('.strip[data-lane="0"] .ex');
    await expect(page.locator('.strip[data-lane="0"]')).toHaveClass(/expanded/);
    await expect(page.locator('.strip[data-lane="0"] [data-pane="pattern"]')).toBeVisible();
  });
});
