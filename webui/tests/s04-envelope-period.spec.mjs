import { test, expect } from '@playwright/test';
import { setupWithActionLog, getActions, clearActions, expandStrip } from './test-helpers.mjs';

// S04 — Editable envelope period with horizontal curve scaling and a discoverable
// depth slider, all routed through the EXISTING setEnvelope bridge (MEM040 — no
// new bridge action, no C++/serialization change). The default preset lane 0
// ("Velocity") ships one envelope at period 4, depth 0.35, on:true
// (mock-host.js:216), so it is the edit target. These specs assert the emitted
// setEnvelope payload (mock host action log), the horizontal curve rescale (path
// d-attribute), the explicit accessible depth slider, the non-positive period
// clamp, and that no bridge action other than setEnvelope is emitted.
async function openEnvPane(page, lane = 0) {
  await expandStrip(page, lane);
  await page.click(`.strip[data-lane="${lane}"] [data-tab="env"]`);
  await page.waitForSelector(`.strip[data-lane="${lane}"] [data-pane="env"].on [data-envperiod="0"]`);
}

// Horizontal drag of a .slider-track from its current position to a normalized
// x within the track (0 = far left, 1 = far right). Uses the real pointer path
// the UI listens to (pointerdown → move → up).
async function dragTrackTo(page, locator, norm) {
  const box = await locator.boundingBox();
  const targetX = box.x + Math.max(0, Math.min(1, norm)) * box.width;
  const y = box.y + box.height / 2;
  await page.mouse.move(box.x + box.width / 2, y);
  await page.mouse.down();
  await page.mouse.move(targetX, y, { steps: 6 });
  await page.mouse.up();
}

test.describe('S04 — editable envelope period (setEnvelope bridge)', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
  });

  test('period control exposes an accessible slider with the baseline period', async ({ page }) => {
    await openEnvPane(page, 0);
    const track = page.locator('.strip[data-lane="0"] [data-envperiod="0"]');
    await expect(track).toHaveAttribute('role', 'slider');
    await expect(track).toHaveAttribute('aria-valuemin', '1');
    await expect(track).toHaveAttribute('aria-valuemax', '16');
    // mock-host lane 0 envelope period is 4.
    await expect(track).toHaveAttribute('aria-valuenow', '4');
    await expect(page.locator('.strip[data-lane="0"] [data-envperiodval="0"]')).toHaveText('4 bars');
    await expect(page.locator('.strip[data-lane="0"] [data-envmeta="0"]')).toHaveText('4 bars · sine');
  });

  test('dragging the period control emits setEnvelope with the changed period', async ({ page }) => {
    await openEnvPane(page, 0);
    const track = page.locator('.strip[data-lane="0"] [data-envperiod="0"]');

    await clearActions(page);
    // Drag to the far right → maximum period (16 bars), well above the 4 baseline.
    await dragTrackTo(page, track, 1);

    const envActs = (await getActions(page)).filter((a) => a.name === 'setEnvelope');
    expect(envActs.length).toBeGreaterThan(0);
    const last = envActs[envActs.length - 1];
    expect(last.payload.lane).toBe(0);
    expect(last.payload.index).toBe(0);
    // Period is a positive integer bar count, strictly greater than the baseline.
    expect(last.payload.envelope.period).toBe(16);
    expect(Number.isInteger(last.payload.envelope.period)).toBe(true);
    // Depth was carried through unchanged (period edit preserves the rest of the envelope).
    expect(last.payload.envelope.depth).toBeCloseTo(0.35, 5);
    // Live readouts tracked the drag.
    await expect(track).toHaveAttribute('aria-valuenow', '16');
    await expect(page.locator('.strip[data-lane="0"] [data-envperiodval="0"]')).toHaveText('16 bars');
    await expect(page.locator('.strip[data-lane="0"] [data-envmeta="0"]')).toHaveText('16 bars · sine');
  });

  test('the curve wavelength scales with period — more bars draw a wider, gentler wave', async ({ page }) => {
    await openEnvPane(page, 0);
    const path = page.locator('.strip[data-lane="0"] [data-envdepth="0"] path');
    const track = page.locator('.strip[data-lane="0"] [data-envperiod="0"]');

    // Count sine humps (local direction reversals) in the path's y-samples: a
    // longer wavelength means fewer reversals across the fixed box width.
    const humpCount = async () => page.evaluate(() => {
      const p = document.querySelector('.strip[data-lane="0"] [data-envdepth="0"] path');
      const ys = p.getAttribute('d').split('L').slice(1).map((seg) => parseFloat(seg.trim().split(/\s+/)[1]));
      let reversals = 0;
      for (let i = 1; i < ys.length - 1; i++) {
        const a = ys[i] - ys[i - 1];
        const b = ys[i + 1] - ys[i];
        if (a !== 0 && b !== 0 && Math.sign(a) !== Math.sign(b)) reversals++;
      }
      return reversals;
    });

    // The box is a fixed PERIOD_MAX(=16)-bar window, so cycles = 16/period:
    // wavelength ∝ period. A LONGER period must draw FEWER humps (a wider,
    // slower-reading wave); a shorter period more humps. This is the intuitive
    // frequency reading — the inverse of the old "more bars = more humps" bug.
    const before = await path.getAttribute('d');
    expect(before).toBeTruthy();

    await dragTrackTo(page, track, 1); // grow to 16 bars → widest wave (1 cycle)
    const afterGrow = await path.getAttribute('d');
    expect(afterGrow).not.toBe(before);
    const humpsAt16 = await humpCount();

    await dragTrackTo(page, track, 0); // shrink to 1 bar → tightest wave (16 cycles)
    const afterShrink = await path.getAttribute('d');
    expect(afterShrink).not.toBe(afterGrow);
    const humpsAt1 = await humpCount();

    // Longer period → fewer humps (wider wavelength); shorter → more.
    expect(humpsAt16).toBeLessThan(humpsAt1);

    // At the maximum period the box holds exactly one full cycle: the curve
    // returns to the y=15 midline at its halfway point.
    await dragTrackTo(page, track, 1); // back to 16 bars
    const midY = await page.evaluate(() => {
      const p = document.querySelector('.strip[data-lane="0"] [data-envdepth="0"] path');
      const total = p.getTotalLength();
      return p.getPointAtLength(total / 2).y;
    });
    expect(midY).toBeCloseTo(15, 0);
  });

  test('non-positive period is clamped — setEnvelope never emits period <= 0', async ({ page }) => {
    await openEnvPane(page, 0);
    const track = page.locator('.strip[data-lane="0"] [data-envperiod="0"]');

    await clearActions(page);
    // Drag hard to the far left (norm 0, i.e. x at/left of the track origin).
    await dragTrackTo(page, track, 0);

    const envActs = (await getActions(page)).filter((a) => a.name === 'setEnvelope');
    expect(envActs.length).toBeGreaterThan(0);
    // Every emitted period stays a positive integer — the clamp guarantees >= 1.
    for (const a of envActs) {
      expect(a.payload.envelope.period).toBeGreaterThanOrEqual(1);
    }
    const last = envActs[envActs.length - 1];
    expect(last.payload.envelope.period).toBe(1);
    await expect(track).toHaveAttribute('aria-valuenow', '1');
    await expect(page.locator('.strip[data-lane="0"] [data-envperiodval="0"]')).toHaveText('1 bars');
  });

  test('the discoverable depth slider emits depth through the same setEnvelope action', async ({ page }) => {
    await openEnvPane(page, 0);
    const slider = page.locator('.strip[data-lane="0"] [data-envdepthslider="0"]');
    await expect(slider).toHaveAttribute('role', 'slider');
    await expect(slider).toHaveAttribute('aria-valuemin', '0');
    await expect(slider).toHaveAttribute('aria-valuemax', '100');
    await expect(slider).toHaveAttribute('aria-valuenow', '35');

    await clearActions(page);
    // Drag the depth slider toward the right → deeper than the 0.35 baseline.
    await dragTrackTo(page, slider, 0.9);

    const envActs = (await getActions(page)).filter((a) => a.name === 'setEnvelope');
    expect(envActs.length).toBeGreaterThan(0);
    const last = envActs[envActs.length - 1];
    expect(last.payload.lane).toBe(0);
    expect(last.payload.index).toBe(0);
    expect(last.payload.envelope.depth).toBeGreaterThan(0.35);
    expect(last.payload.envelope.depth).toBeLessThanOrEqual(1);
    // Period carried through unchanged by a depth edit.
    expect(last.payload.envelope.period).toBe(4);
    // The slider and the curve's own depth readout stay in lock-step.
    const shownPct = parseInt(await page.locator('.strip[data-lane="0"] [data-envdepthsliderval="0"]').textContent(), 10);
    expect(shownPct).toBeGreaterThan(35);
    await expect(page.locator('.strip[data-lane="0"] [data-envdepthval="0"]')).toHaveText(`${shownPct}%`);
    await expect(page.locator('.strip[data-lane="0"] [data-envdepth="0"]')).toHaveAttribute('aria-valuenow', String(shownPct));
  });

  test('MEM040 — period and depth edits emit only the setEnvelope bridge action', async ({ page }) => {
    await openEnvPane(page, 0);
    const periodTrack = page.locator('.strip[data-lane="0"] [data-envperiod="0"]');
    const depthSlider = page.locator('.strip[data-lane="0"] [data-envdepthslider="0"]');

    await clearActions(page);
    await dragTrackTo(page, periodTrack, 0.7);
    await dragTrackTo(page, depthSlider, 0.6);

    const acts = await getActions(page);
    expect(acts.length).toBeGreaterThan(0);
    // No new bridge action name may appear — every emitted action is setEnvelope.
    for (const a of acts) {
      expect(a.name).toBe('setEnvelope');
    }
  });
});
