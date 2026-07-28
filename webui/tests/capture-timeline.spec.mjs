import { test, expect } from '@playwright/test';
import { fileURLToPath } from 'node:url';
import path from 'node:path';

// M051 S08 T04: Cloth as a bar-anchored capture timeline. These specs drive the
// mock host's capture-machine mirror (_setCapture / armCapture) and assert the
// Cloth render receipt (window.__polyClothState), the annotation/chrome toggle,
// and the capline narration across idle -> armed -> capturing -> complete.

const pageUrl =
  'file://' + path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..', 'index.html');

test.beforeEach(async ({ page }) => {
  await page.goto(pageUrl);
  await page.click('#mCloth');
  await expect(page.locator('#cloth')).toHaveClass(/on/);
});

// Force a capture-machine state on the mock and wait for the next Cloth frame
// to render it (the rAF pump carries capture fields to onFrame -> drawLoom).
async function setCapture(page, cap) {
  await page.evaluate((c) => window.PolyMockHost._setCapture(c), cap);
  await page.waitForFunction(
    (s) => window.__polyClothState && window.__polyClothState.capState === s,
    cap.state,
  );
  return page.evaluate(() => window.__polyClothState);
}

test('idle Cloth keeps the convergence weave and shows annotations', async ({ page }) => {
  const st = await setCapture(page, { state: 0, bars: 8 });
  expect(st.mode).toBe('convergence');
  // Annotations are the idle story — visible in idle.
  const annDisplay = await page.evaluate(() => getComputedStyle(document.getElementById('annA')).display);
  expect(annDisplay).not.toBe('none');
  await expect(page.locator('#cloth')).not.toHaveClass(/capturing/);
});

test('armed switches Cloth to a bar-anchored timeline and hides annotations', async ({ page }) => {
  const st = await setCapture(page, { state: 1, bars: 8 });
  expect(st.mode).toBe('timeline');
  expect(st.bars).toBe(8);
  await expect(page.locator('#cloth')).toHaveClass(/capArmed/);
  const annDisplay = await page.evaluate(() => getComputedStyle(document.getElementById('annA')).display);
  expect(annDisplay).toBe('none');
  await expect(page.locator('#cloth .capline')).toContainText('Armed');
});

test('capturing sweeps the playhead L->R proportional to progress', async ({ page }) => {
  const st = await setCapture(page, { state: 2, bars: 8, prog: 3 });
  expect(st.mode).toBe('timeline');
  // prog 3 of 8 bars -> playhead at 3/8.
  expect(st.playhead).toBeCloseTo(3 / 8, 3);
  await expect(page.locator('#cloth')).toHaveClass(/capturing/);
  await expect(page.locator('#cloth .capline')).toContainText('Capturing');
  await expect(page.locator('#cloth .capline')).toContainText('4/8');
});

test('complete pins the playhead at bar N and paints the gold selvage', async ({ page }) => {
  const st = await setCapture(page, { state: 3, bars: 8 });
  expect(st.playhead).toBe(1);
  await expect(page.locator('#cloth')).toHaveClass(/capComplete/);
  await expect(page.locator('#cloth .capline')).toContainText('Complete');
  // The gold selvage (#F0C24A ~ rgb(240,194,74)) is painted on the right edge.
  const gold = await page.evaluate(() => {
    const c = document.getElementById('loom');
    const g = c.getContext('2d');
    const px = g.getImageData(c.width - 2, Math.floor(c.height / 2), 1, 1).data;
    return { r: px[0], g: px[1], b: px[2] };
  });
  expect(gold.r).toBeGreaterThan(200);
  expect(gold.g).toBeGreaterThan(150);
  expect(gold.b).toBeLessThan(140);
});

test('bar-window length flows from capBars into the timeline', async ({ page }) => {
  const st = await setCapture(page, { state: 2, bars: 16, prog: 0 });
  expect(st.bars).toBe(16);
  await expect(page.locator('#cloth .capline')).toContainText('1/16');
});

test('armCapture action drives the mock machine idle -> armed', async ({ page }) => {
  await page.evaluate(() => window.PolyMockHost.action('armCapture', {}));
  const cap = await page.evaluate(() => window.PolyMockHost._getCapture());
  expect(cap.state).toBe(1);
  // Reset returns to idle from any state.
  await page.evaluate(() => window.PolyMockHost.action('resetCapture', {}));
  const cap2 = await page.evaluate(() => window.PolyMockHost._getCapture());
  expect(cap2.state).toBe(0);
});

test('setCaptureBars only takes the supported {4,8,16,32} lengths before latch', async ({ page }) => {
  await page.evaluate(() => window.PolyMockHost.action('setCaptureBars', { bars: 16 }));
  expect(await page.evaluate(() => window.PolyMockHost._getCapture().bars)).toBe(16);
  // Unsupported value ignored.
  await page.evaluate(() => window.PolyMockHost.action('setCaptureBars', { bars: 7 }));
  expect(await page.evaluate(() => window.PolyMockHost._getCapture().bars)).toBe(16);
  // Not editable once capturing has latched.
  await page.evaluate(() => window.PolyMockHost._setCapture({ state: 2 }));
  await page.evaluate(() => window.PolyMockHost.action('setCaptureBars', { bars: 8 }));
  expect(await page.evaluate(() => window.PolyMockHost._getCapture().bars)).toBe(16);
});

// --- M051 S08 T05: header capture controls (bars picker, Arm/Reset, Export) ---

test('the capture control cluster is Cloth-only', async ({ page }) => {
  // beforeEach leaves us in Cloth mode.
  await expect(page.locator('#capCtl')).toHaveClass(/show/);
  await page.click('#mDesk');
  await expect(page.locator('#capCtl')).not.toHaveClass(/show/);
  await page.click('#mCloth');
  await expect(page.locator('#capCtl')).toHaveClass(/show/);
});

test('the bars picker cycles kCaptureLength through {4,8,16,32} and reflects host truth', async ({ page }) => {
  const bars = page.locator('#capBars');
  await expect(bars).toHaveText('8 bars');
  await bars.click(); // 8 -> 16
  await expect(bars).toHaveText('16 bars');
  expect(await page.evaluate(() => window.PolyMockHost._getCapture().bars)).toBe(16);
  await bars.click(); // 16 -> 32
  await expect(bars).toHaveText('32 bars');
  await bars.click(); // 32 -> 4 (wrap)
  await expect(bars).toHaveText('4 bars');
  expect(await page.evaluate(() => window.PolyMockHost._getCapture().bars)).toBe(4);
});

test('the Arm chip flips to Reset and drives the machine, Reset returns to idle', async ({ page }) => {
  const arm = page.locator('#armBtn');
  await expect(arm).toHaveText('Arm');
  await expect(arm).not.toHaveClass(/on/);
  await arm.click();
  expect(await page.evaluate(() => window.PolyMockHost._getCapture().state)).toBe(1);
  await expect(arm).toHaveText('Reset');
  await expect(arm).toHaveClass(/on/); // armed -> Reset affordance is active
  await arm.click();
  expect(await page.evaluate(() => window.PolyMockHost._getCapture().state)).toBe(0);
  await expect(arm).toHaveText('Arm');
});

test('the bars picker locks once capture latches (state >= 2)', async ({ page }) => {
  await setCapture(page, { state: 2, bars: 8, prog: 1 });
  await expect(page.locator('#capBars')).toHaveClass(/locked/);
  // Click is a no-op while the window is frozen.
  await page.locator('#capBars').click({ force: true });
  expect(await page.evaluate(() => window.PolyMockHost._getCapture().bars)).toBe(8);
});

test('the Export chip is state-driven: capReady only in complete', async ({ page }) => {
  const exp = page.locator('#exportBtn');
  await setCapture(page, { state: 0, bars: 8 });
  await expect(exp).not.toHaveClass(/capReady/);
  await setCapture(page, { state: 1, bars: 8 });
  await expect(exp).not.toHaveClass(/capReady/);
  await setCapture(page, { state: 2, bars: 8, prog: 4 });
  await expect(exp).not.toHaveClass(/capReady/);
  await setCapture(page, { state: 3, bars: 8 });
  await expect(exp).toHaveClass(/capReady/);
});
