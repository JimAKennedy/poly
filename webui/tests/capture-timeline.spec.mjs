import { test, expect } from '@playwright/test';
import { fileURLToPath } from 'node:url';
import path from 'node:path';

// M051 S08 T04, retargeted by M001/S02. These specs drive the mock host's
// capture-machine mirror (_setCapture / armCapture) and assert the header
// chips across idle -> armed -> capturing -> complete.
//
// They used to assert the Cloth render receipt (window.__polyClothState), the
// annotation toggle and the capline narration. Cloth is gone; capture is not.
// The behaviour that survives is the state machine and the toolbar that
// reflects it, so the assertions point there instead.

const pageUrl =
  'file://' + path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..', 'index.html');

test.beforeEach(async ({ page }) => {
  await page.goto(pageUrl);
});

// Force a capture-machine state on the mock and wait for the chips to catch up.
// The wait used to be on the Cloth render receipt; the bars chip is now the
// observable that proves a frame carrying the new state has been applied.
async function setCapture(page, cap) {
  await page.evaluate((c) => window.PolyMockHost._setCapture(c), cap);
  await page.waitForFunction(
    (s) => {
      const el = document.getElementById('armBtn');
      return el && (s === 0 ? el.textContent === 'Arm' : el.textContent === 'Reset');
    },
    cap.state,
  );
}






test('armCapture action drives the mock machine idle -> armed', async ({ page }) => {
  await page.evaluate(() => window.PolyMockHost.action('armCapture', {}));
  const cap = await page.evaluate(() => window.PolyMockHost._getCapture());
  expect(cap.state).toBe(1);
  // Reset returns to idle from any state.
  await page.evaluate(() => window.PolyMockHost.action('resetCapture', {}));
  const cap2 = await page.evaluate(() => window.PolyMockHost._getCapture());
  expect(cap2.state).toBe(0);
});

test('setCaptureBars accepts any integer 1-32 before latch and rejects out-of-range', async ({ page }) => {
  const set = (bars) => page.evaluate((b) => window.PolyMockHost.action('setCaptureBars', { bars: b }), bars);
  const bars = () => page.evaluate(() => window.PolyMockHost._getCapture().bars);
  // G07: kCaptureLength is a 1-32 param, so any integer in [1,32] latches —
  // including odd lengths the old fixed {4,8,16,32} picker could never reach.
  await set(16); expect(await bars()).toBe(16);
  await set(7); expect(await bars()).toBe(7);
  await set(1); expect(await bars()).toBe(1);
  await set(32); expect(await bars()).toBe(32);
  // Out-of-range values are rejected (window stays at 32).
  await set(0); expect(await bars()).toBe(32);
  await set(33); expect(await bars()).toBe(32);
  // Not editable once capturing has latched (state >= 2).
  await page.evaluate(() => window.PolyMockHost._setCapture({ state: 2 }));
  await set(8); expect(await bars()).toBe(32);
});

// --- M051 S08 T05: header capture controls (bars picker, Arm/Reset, Export) ---

// FR01. The cluster used to be Cloth-only, which is the defect: M001/S02
// removes Cloth, so a capture control reachable only from there would be a
// capture control reachable from nowhere. The capability was never Cloth-bound
// -- chapter 16 documents it as VST3 parameters 600 and 601 -- only the chips
// were. This case is the inverse of the one it replaces, and it is what fails
// if the gating ever returns.
test('capture controls are reachable without entering another view', async ({ page }) => {
  await page.goto(pageUrl); // fresh load: no mode chip clicked, default view
  await expect(page.locator('#capCtl')).toBeVisible();
  await expect(page.locator('#capBars')).toBeVisible();
  await expect(page.locator('#armBtn')).toBeVisible();
});

test('the bars picker is a 1-32 stepper and reflects host truth', async ({ page }) => {
  const bars = page.locator('#capBars');
  await expect(bars).toHaveText('8 bars');
  await bars.click(); // 8 -> 9 (step +1, not a {4,8,16,32} jump)
  await expect(bars).toHaveText('9 bars');
  expect(await page.evaluate(() => window.PolyMockHost._getCapture().bars)).toBe(9);
  await bars.click({ button: 'right' }); // 9 -> 8 (step -1 via contextmenu)
  await expect(bars).toHaveText('8 bars');
  expect(await page.evaluate(() => window.PolyMockHost._getCapture().bars)).toBe(8);
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

// FR01. The capline inside #cloth is the only per-bar progress readout today,
// and it leaves with Cloth in M001/S02. The bars chip already renders the count
// and locks while capturing, so it carries the progress too rather than the
// information being lost. Numerator matches what the capline used:
// Math.min(bars, floor(prog) + 1).
test('the bars chip reports progress while capturing', async ({ page }) => {
  const bars = page.locator('#capBars');
  await expect(bars).toHaveText('8 bars');
  await setCapture(page, { state: 2, bars: 8, prog: 2.4 });
  await expect(bars).toHaveText('3/8 bars');
  await setCapture(page, { state: 3, bars: 8, prog: 8 });
  await expect(bars).toHaveText('8 bars'); // complete: back to the plain count
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
