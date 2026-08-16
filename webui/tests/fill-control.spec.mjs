import { test, expect } from '@playwright/test';
import {
  setupWithActionLog, getActions, clearActions, getEdits, clearEdits, expandStrip,
} from './test-helpers.mjs';

// M034 S01 T04 — WebUI fill controls. The engine (T01), state serialization
// (T02) and plugin param plumbing (T03) already carry a per-lane fillEveryNBars
// core param and a global momentary manual-fill trigger. This slice exposes both
// in the Advanced pane:
//   - a "Fill / Every" slider that emits host.edit('lane.N.fillEveryN', norm) and
//     round-trips through getState (mock host maps norm -> round(norm*64) bars,
//     mirroring the engine's Ranged0_64 0..64 registry scaling), and
//   - a "Fill Now" button that emits host.action('manualFill') — the native
//     bridge pulses kFillManualTrigger for exactly one fill render pass.
// bridge_params.h resolves "lane.N.fillEveryN" -> laneCoreParam(N, kCoreFillEveryN)
// and web_ui_view.cpp handleAction("manualFill") drives the momentary edge, so
// these mock-host assertions exercise the same contract the plugin host honours.

async function openAdvPane(page, lane = 0) {
  await expandStrip(page, lane);
  await page.click(`.strip[data-lane="${lane}"] [data-tab="adv"]`);
  await page.waitForSelector(`.strip[data-lane="${lane}"] [data-pane="adv"].on`);
}

function fillTrack(page, lane = 0) {
  return page.locator(`.strip[data-lane="${lane}"] [data-pane="adv"] .slider-track[data-field="fillEveryN"]`);
}

// Horizontal drag of a .slider-track to a normalized x within the track
// (0 = far left, 1 = far right), using the real pointer path the UI listens to.
async function dragTrackTo(page, locator, norm) {
  const box = await locator.boundingBox();
  const targetX = box.x + Math.max(0, Math.min(1, norm)) * box.width;
  const y = box.y + box.height / 2;
  await page.mouse.move(box.x + box.width / 2, y);
  await page.mouse.down();
  await page.mouse.move(targetX, y, { steps: 6 });
  await page.mouse.up();
}

test.describe('M034 S01 T04 — WebUI fill controls', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
  });

  test('Advanced pane exposes a Fill/Every slider and a Fill Now button', async ({ page }) => {
    await openAdvPane(page, 0);
    const adv = page.locator('.strip[data-lane="0"] [data-pane="adv"]');
    await expect(adv).toContainText('Fill');
    await expect(fillTrack(page, 0)).toHaveCount(1);
    await expect(adv.locator('[data-manualfill]')).toHaveCount(1);
  });

  test('a lane with fillEveryN=0 shows "Off"', async ({ page }) => {
    await openAdvPane(page, 0);
    const vSpan = fillTrack(page, 0).locator('xpath=following-sibling::span[1]');
    await expect(vSpan).toHaveText('Off');
  });

  test('dragging the fill slider emits lane.0.fillEveryN and round-trips through getState', async ({ page }) => {
    await openAdvPane(page, 0);
    await clearEdits(page);
    await dragTrackTo(page, fillTrack(page, 0), 0.25); // ~16 bars

    const edits = await getEdits(page);
    const fillEdits = edits.filter((e) => e.paramId === 'lane.0.fillEveryN');
    expect(fillEdits.length).toBeGreaterThan(0);
    // The UI drives a begin/perform.../end gesture like every other slider.
    expect(fillEdits[0].gesture).toBe('begin');
    expect(fillEdits[fillEdits.length - 1].gesture).toBe('end');

    // Round-trip: the mock host maps the last committed norm to round(norm*64)
    // bars, exactly as the engine's Ranged0_64 registry entry does.
    const lastNorm = fillEdits[fillEdits.length - 1].value;
    const expectedBars = Math.round(lastNorm * 64);
    const bars = await page.evaluate(() => window.PolyMockHost.getState().lanes[0].fillEveryN);
    expect(bars).toBe(expectedBars);
    expect(bars).toBeGreaterThan(0);

    // The visible readout reflects the round-tripped value in bars.
    const vSpan = fillTrack(page, 0).locator('xpath=following-sibling::span[1]');
    await expect(vSpan).toHaveText(`${expectedBars} bars`);
  });

  test('the fill slider emits only edits, never the manualFill action', async ({ page }) => {
    await openAdvPane(page, 0);
    await clearActions(page);
    await dragTrackTo(page, fillTrack(page, 0), 0.5);
    const actions = await getActions(page);
    expect(actions.find((a) => a.name === 'manualFill')).toBeUndefined();
  });

  test('Fill Now emits the manualFill action and no lane edit', async ({ page }) => {
    await openAdvPane(page, 0);
    await clearActions(page);
    await clearEdits(page);
    await page.click('.strip[data-lane="0"] [data-pane="adv"] [data-manualfill]');

    const actions = await getActions(page);
    const manual = actions.filter((a) => a.name === 'manualFill');
    expect(manual.length).toBe(1);

    // Manual fill is a momentary trigger, not a lane parameter — it must not emit
    // a fillEveryN edit (that would sticky-arm the periodic fill).
    const edits = await getEdits(page);
    expect(edits.find((e) => e.paramId && e.paramId.endsWith('.fillEveryN'))).toBeUndefined();
  });
});
