import { test, expect } from '@playwright/test';
import { fileURLToPath } from 'node:url';
import path from 'node:path';

const pageUrl =
  'file://' + path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..', 'index.html');

test.beforeEach(async ({ page }) => {
  await page.goto(pageUrl);
});

test('boots from mock host state', async ({ page }) => {
  await expect(page.locator('#presetName')).toHaveText('Afrobeat 12/8');
  await expect(page.locator('.strip[data-lane]')).toHaveCount(5);
  await expect(page.locator('#master')).toBeVisible();
});

test('cloth/desk mode switch', async ({ page }) => {
  await expect(page.locator('#desk')).toHaveClass(/on/);
  await page.click('#mCloth');
  await expect(page.locator('#cloth')).toHaveClass(/on/);
  await expect(page.locator('#desk')).not.toHaveClass(/on/);
  await page.click('#mDesk');
  await expect(page.locator('#desk')).toHaveClass(/on/);
});

test('cloth band click focuses lane on desk', async ({ page }) => {
  await page.click('#mCloth');
  const loom = page.locator('#loom');
  const box = await loom.boundingBox();
  // click in the second band (Kick)
  await loom.click({ position: { x: box.width / 2, y: box.height * 0.3 } });
  await expect(page.locator('#desk')).toHaveClass(/on/);
  await expect(page.locator('.strip[data-lane="1"]')).toHaveClass(/expanded/);
});

test('step toggle round-trips through host state', async ({ page }) => {
  // Bell lane (timeline mode), step 2 (index 1): initially off (fixed[1]=0)
  const btn = page.locator('.strip[data-lane="0"] .ladder button').nth(1);
  await expect(btn).not.toHaveClass(/hit/);
  await btn.click();
  // UI re-renders from host state — the rebuilt button must be on
  await expect(page.locator('.strip[data-lane="0"] .ladder button').nth(1)).toHaveClass(/hit/);
});

test('strip expansion exposes deep editors', async ({ page }) => {
  await page.click('.strip[data-lane="0"] .ex');
  const strip = page.locator('.strip[data-lane="0"]');
  await expect(strip).toHaveClass(/expanded/);
  await expect(strip.locator('[data-pane="pattern"]')).toBeVisible();
  await expect(strip.locator('[data-pane="pattern"]')).toContainText('Timeline mode');
  await strip.locator('[data-tab="env"]').click();
  await expect(strip.locator('[data-pane="env"]')).toContainText('Velocity');
  await page.keyboard.press('Escape');
  await expect(strip).not.toHaveClass(/expanded/);
});

test('additive cells editor changes cycle', async ({ page }) => {
  await page.click('.strip[data-lane="4"] .ex');
  const strip = page.locator('.strip[data-lane="4"]');
  await strip.locator('[data-cl]').click(); // enable cells [2,2,3]
  await expect(strip.locator('[data-pane="pattern"]')).toContainText('cycle = 7♪ (2+2+3)');
  await expect(strip.locator('.ladder button')).toHaveCount(3);
  // click first cell: 2 -> 3, cycle becomes 8
  await strip.locator('[data-cells] button[data-i="0"]').click();
  await expect(strip.locator('[data-pane="pattern"]')).toContainText('(3+2+3)');
});

test('euclid steppers regenerate pattern', async ({ page }) => {
  await page.click('.strip[data-lane="2"] .ex'); // Snare E(2,3)
  const strip = page.locator('.strip[data-lane="2"]');
  await strip.locator('[data-ht="1"]').click(); // hits 2 -> 3
  await expect(strip.locator('[data-pane="pattern"]')).toContainText('E(3,3)');
  await expect(strip.locator('.ladder button.hit')).toHaveCount(3);
});

test('transport toggles from keyboard', async ({ page }) => {
  const picon = page.locator('#picon');
  const before = await picon.getAttribute('d');
  await page.keyboard.press('Space');
  await expect
    .poll(async () => picon.getAttribute('d'))
    .not.toBe(before); // play icon became stop bars (frame-driven)
});
