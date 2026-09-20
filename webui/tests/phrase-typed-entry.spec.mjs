import { test, expect } from '@playwright/test';
import { setupWithActionLog, getEdits, clearEdits, expandStrip } from './test-helpers.mjs';

// Phrase Length/Gap/Offset span 0-968 beats on a squared curve
// (params_def.h Kind::SquaredFloat). The curve keeps the low end draggable, but
// an exact value -- a 7-beat tihai cell, a 96-beat rest -- is still easier to
// type than to hunt for. The value readout is therefore click-to-type.
//
// norm = sqrt(beats / 968), which is what the engine inverts, so these
// assertions exercise the same mapping the plugin host applies.

const MAX_BEATS = 968;
const normFor = (beats) => Math.sqrt(beats / MAX_BEATS);

async function openAdvPane(page, lane = 0) {
  await expandStrip(page, lane);
  await page.click(`.strip[data-lane="${lane}"] [data-tab="adv"]`);
  await page.waitForSelector(`.strip[data-lane="${lane}"] [data-pane="adv"].on`);
}

const valueSpan = (page, field, lane = 0) =>
  page.locator(`.strip[data-lane="${lane}"] [data-pane="adv"] .slider-track[data-field="${field}"] + .v`);

async function typeValue(page, field, text, lane = 0) {
  const span = valueSpan(page, field, lane);
  await span.click();
  const input = span.locator('input.v-edit');
  await expect(input).toBeVisible();
  await input.fill(text);
  await input.press('Enter');
  return span;
}

test.describe('phrase values are typeable', () => {
  test.beforeEach(async ({ page }) => {
    await setupWithActionLog(page);
    await openAdvPane(page);
    await clearEdits(page);
  });

  test('typing a beat count emits the matching normalized edit', async ({ page }) => {
    const span = await typeValue(page, 'phraseLength', '96');
    await expect(span).toHaveText('96 beats');

    const edits = await getEdits(page);
    const perform = edits.filter((e) => e.paramId === 'lane.0.phraseLength' && e.gesture === 'perform');
    expect(perform.length).toBeGreaterThan(0);
    expect(perform[perform.length - 1].value).toBeCloseTo(normFor(96), 5);

    // A typed value is one gesture, so it must bracket begin/end like a drag --
    // a host that only sees 'perform' never commits the automation gesture.
    const phases = edits.filter((e) => e.paramId === 'lane.0.phraseLength').map((e) => e.gesture);
    expect(phases).toContain('begin');
    expect(phases).toContain('end');
  });

  test('a small exact value the curve makes hard to drag is reachable by typing', async ({ page }) => {
    await typeValue(page, 'phraseLength', '16'); // Gap is inert while Length is Off
    await clearEdits(page);
    const span = await typeValue(page, 'phraseGap', '7');
    await expect(span).toHaveText('7 beats');
    const edits = await getEdits(page);
    const last = edits.filter((e) => e.paramId === 'lane.0.phraseGap' && e.gesture === 'perform').pop();
    expect(last.value).toBeCloseTo(normFor(7), 5);
  });

  test('out-of-range input clamps to the 0-968 beat range', async ({ page }) => {
    await typeValue(page, 'phraseLength', '16'); // Offset is inert while Length is Off
    await clearEdits(page);
    const span = await typeValue(page, 'phraseOffset', '99999');
    await expect(span).toHaveText(`${MAX_BEATS} beats`);
    const edits = await getEdits(page);
    const last = edits.filter((e) => e.paramId === 'lane.0.phraseOffset' && e.gesture === 'perform').pop();
    expect(last.value).toBeCloseTo(1, 5);
  });

  test('"off" reads back as zero, so the displayed word can be typed in', async ({ page }) => {
    const span = await typeValue(page, 'phraseLength', 'off');
    await expect(span).toHaveText('Off');
    const edits = await getEdits(page);
    const last = edits.filter((e) => e.paramId === 'lane.0.phraseLength' && e.gesture === 'perform').pop();
    expect(last.value).toBeCloseTo(0, 5);
  });

  test('Gap is not typeable while Length is Off', async ({ page }) => {
    // Gap and Offset are inert when the lane never rests, and the UI disables
    // them to say so. Click-to-type must honour that rather than offering an
    // edit box for a value that cannot take effect.
    await expect(valueSpan(page, 'phraseLength')).toHaveText('Off');
    const gap = valueSpan(page, 'phraseGap');
    await gap.click();
    await expect(gap.locator('input.v-edit')).toHaveCount(0);
    expect(await getEdits(page)).toHaveLength(0);
  });

  test('Escape cancels without emitting an edit', async ({ page }) => {
    const span = valueSpan(page, 'phraseLength');
    const before = await span.textContent();
    await span.click();
    const input = span.locator('input.v-edit');
    await input.fill('512');
    await input.press('Escape');
    await expect(span).toHaveText(before.trim());
    const edits = await getEdits(page);
    expect(edits.filter((e) => e.paramId === 'lane.0.phraseLength')).toHaveLength(0);
  });

  test('gibberish leaves the value untouched', async ({ page }) => {
    await typeValue(page, 'phraseLength', '16'); // Gap is inert while Length is Off
    await clearEdits(page);
    const span = valueSpan(page, 'phraseGap');
    const before = await span.textContent();
    await span.click();
    await span.locator('input.v-edit').fill('not a number');
    await span.locator('input.v-edit').press('Enter');
    await expect(span).toHaveText(before.trim());
    const edits = await getEdits(page);
    expect(edits.filter((e) => e.paramId === 'lane.0.phraseGap')).toHaveLength(0);
  });
});
