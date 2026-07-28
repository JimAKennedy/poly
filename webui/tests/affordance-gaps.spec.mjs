import { test, expect } from '@playwright/test';
import {
  pageUrl,
  setupWithActionLog,
  getActions,
  clearActions,
  getEdits,
  clearEdits,
  expandStrip,
} from './test-helpers.mjs';

// M053 S03c T04: parity specs for the three small affordance/gating gaps closed
// in this slice, proven against the real ui.js render + the mock host:
//   G07 — capture length is a 1-32 stepper driving setCaptureBars (any integer
//         reachable), replacing the old fixed {4,8,16,32} cycle.
//   G08 — double-clicking a per-step micro-timing bar resets it to 0 ms via the
//         existing setMicroTiming path (native one-gesture reset parity).
//   G15 — Phrase Gap/Offset sliders carry a disabled affordance and emit no edit
//         while phrase Length is Off; they re-enable once Length > 0.

test.describe('G07 — capture length 1-32 stepper', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
    await page.click('#mCloth');
    await expect(page.locator('#cloth')).toHaveClass(/on/);
    await expect(page.locator('#capBars')).toHaveText('8 bars');
  });

  test('clicking steps the window by +1 (not a {4,8,16,32} jump)', async ({ page }) => {
    const bars = page.locator('#capBars');
    await bars.click(); // 8 -> 9
    let acts = await getActions(page);
    expect(acts).toContainEqual(
      expect.objectContaining({ name: 'setCaptureBars', payload: { bars: 9 } })
    );
    await expect(bars).toHaveText('9 bars');

    await clearActions(page);
    await bars.click(); // 9 -> 10
    acts = await getActions(page);
    expect(acts).toContainEqual(
      expect.objectContaining({ name: 'setCaptureBars', payload: { bars: 10 } })
    );
    await expect(bars).toHaveText('10 bars');
    // Host truth agrees — an arbitrary integer length, impossible under the old cycle.
    expect(await page.evaluate(() => window.PolyMockHost._getCapture().bars)).toBe(10);
  });

  test('right-click / contextmenu steps the window by -1', async ({ page }) => {
    const bars = page.locator('#capBars');
    await bars.click({ button: 'right' }); // 8 -> 7
    const acts = await getActions(page);
    expect(acts).toContainEqual(
      expect.objectContaining({ name: 'setCaptureBars', payload: { bars: 7 } })
    );
    await expect(bars).toHaveText('7 bars');
    // 7 is a length the old fixed {4,8,16,32} picker could never reach.
    expect(await page.evaluate(() => window.PolyMockHost._getCapture().bars)).toBe(7);
  });

  test('the stepper wraps across the 1-32 domain in both directions', async ({ page }) => {
    const bars = page.locator('#capBars');
    // Wrap high: 32 -> 1.
    await page.evaluate(() => window.PolyMockHost.action('setCaptureBars', { bars: 32 }));
    await expect(bars).toHaveText('32 bars');
    await bars.click();
    await expect(bars).toHaveText('1 bars');
    expect(await page.evaluate(() => window.PolyMockHost._getCapture().bars)).toBe(1);
    // Wrap low: 1 -> 32.
    await bars.click({ button: 'right' });
    await expect(bars).toHaveText('32 bars');
    expect(await page.evaluate(() => window.PolyMockHost._getCapture().bars)).toBe(32);
  });

  test('the stepper is inert once capture latches (state >= 2)', async ({ page }) => {
    await page.evaluate(() => window.PolyMockHost._setCapture({ state: 2, bars: 12 }));
    await expect(page.locator('#capBars')).toHaveText('12 bars');
    await clearActions(page);
    await page.locator('#capBars').click({ force: true });
    // No action dispatched, window frozen.
    expect(await getActions(page)).toEqual([]);
    expect(await page.evaluate(() => window.PolyMockHost._getCapture().bars)).toBe(12);
  });
});

test.describe('G08 — micro-timing double-click-to-zero reset', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
    await expandStrip(page, 1); // Kick — 4 steps, mt [3,0,2,0]
    const strip = page.locator('.strip[data-lane="1"]');
    await strip.locator('[data-tab="timing"]').click();
    await expect(strip.locator('[data-pane="timing"]')).toHaveClass(/on/);
  });

  test('double-clicking a bar dispatches setMicroTiming ms:0 and recentres it', async ({ page }) => {
    const strip = page.locator('.strip[data-lane="1"]');
    const bar = strip.locator('.mb').first();
    const box = await bar.boundingBox();

    // First push the step late (positive ms) with a normal drag-click.
    await bar.click({ position: { x: box.width / 2, y: box.height * 0.2 } });
    let acts = await getActions(page);
    let last = acts.filter((a) => a.name === 'setMicroTiming').pop();
    expect(last.payload).toMatchObject({ lane: 1, step: 0 });
    expect(last.payload.ms).toBeGreaterThan(0);

    await clearActions(page);
    // Double-click near the top edge: a bare pointerdown there would set a
    // positive ms, so ms:0 proves the reset gesture wins over the drag.
    await bar.dblclick({ position: { x: box.width / 2, y: box.height * 0.2 } });
    acts = await getActions(page);
    last = acts.filter((a) => a.name === 'setMicroTiming').pop();
    expect(last.payload).toEqual({ lane: 1, step: 0, ms: 0 });

    // The model zeroes step 0 (setMicroTiming re-renders the pane from host truth).
    const mt0 = await page.evaluate(() => window.PolyMockHost.getState().lanes[1].mt[0]);
    expect(mt0).toBe(0);
    // The rendered bar recentres: fill height 0, anchored at the midline.
    const fill = await strip.locator('.mb').first().locator('i')
      .evaluate((el) => ({ h: el.style.height, top: el.style.top }));
    expect(fill.h).toBe('0%');
    expect(fill.top).toBe('50%');
  });
});

test.describe('G15 — phrase Gap/Offset gating on Length Off', () => {
  // Default preset (Afrobeat 12/8) ships every lane with phraseLength 0 (Off),
  // so lane 0's Gap/Offset start disabled.
  async function openAdv(page, lane) {
    await expandStrip(page, lane);
    const strip = page.locator(`.strip[data-lane="${lane}"]`);
    await strip.locator('[data-tab="adv"]').click();
    await expect(strip.locator('[data-pane="adv"]')).toHaveClass(/on/);
    return strip;
  }

  test('Gap and Offset are disabled while Length is Off; Length stays active', async ({ page }) => {
    await page.goto(pageUrl);
    const strip = await openAdv(page, 0);
    const length = strip.locator('.param-slider:has([data-field="phraseLength"])');
    const gap = strip.locator('.param-slider:has([data-field="phraseGap"])');
    const offset = strip.locator('.param-slider:has([data-field="phraseOffset"])');

    await expect(length).not.toHaveClass(/phrase-disabled/);
    await expect(gap).toHaveClass(/phrase-disabled/);
    await expect(offset).toHaveClass(/phrase-disabled/);
    // Disabled affordance carries the not-allowed cursor.
    await expect(gap.locator('.slider-track')).toHaveCSS('cursor', 'not-allowed');
  });

  test('dragging a disabled Gap slider emits no host.edit', async ({ page }) => {
    await setupWithActionLog(page);
    const strip = await openAdv(page, 0);
    await clearEdits(page);
    await strip.locator('.param-slider:has([data-field="phraseGap"]) .slider-track')
      .click({ position: { x: 40, y: 6 }, force: true });
    const edits = await getEdits(page);
    expect(edits.filter((e) => e.paramId === 'lane.0.phraseGap')).toEqual([]);
  });

  test('setting Length > 0 re-enables Gap/Offset and their edits flow', async ({ page }) => {
    await setupWithActionLog(page);
    let strip = await openAdv(page, 0);
    // Drag Length to ~50% -> phraseLength ~32 bars (> 0), which re-renders the pane.
    await strip.locator('.param-slider:has([data-field="phraseLength"]) .slider-track').click();

    strip = page.locator('.strip[data-lane="0"]');
    const gap = strip.locator('.param-slider:has([data-field="phraseGap"])');
    const offset = strip.locator('.param-slider:has([data-field="phraseOffset"])');
    await expect(gap).not.toHaveClass(/phrase-disabled/);
    await expect(offset).not.toHaveClass(/phrase-disabled/);

    // Now the Gap slider is live: a drag emits lane.0.phraseGap edits.
    await clearEdits(page);
    await gap.locator('.slider-track').click({ position: { x: 40, y: 6 } });
    const edits = await getEdits(page);
    expect(edits.some((e) => e.paramId === 'lane.0.phraseGap')).toBe(true);
  });
});
