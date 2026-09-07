import { test, expect } from '@playwright/test';
import { pageUrl, editLaneParam, expandStrip } from './test-helpers.mjs';

// ---------------------------------------------------------------------------
// MIDI-channel Auto sentinel symmetric encoding (M073 S03 T02).
//
// The WebUI shares the single -1↔Auto mapping the C++ native/state paths use:
// the channel control writes `Math.round(norm * 16) - 1` (range -1..15, where
// norm 0 → -1 Auto) and reads it back raw. The stored value is the MIDI *wire*
// channel, 0-15. Both display surfaces label ch<0 as 'Auto' else 'CH '+(ch+1),
// because MIDI channels are numbered 1-16 everywhere a human reads them — a
// lane storing 0 is the channel Cubase calls 1. This spec proves that mapping is
// symmetric through the mock host's real begin/perform/end edit gesture:
//   - editing the channel to norm 0 lands lane.ch === -1 (Auto), not 0/explicit;
//   - editing to (N+1)/16 lands lane.ch === N exactly, with no off-by-one;
//   - both the always-visible stat chip and the channel slider value label
//     render 'Auto' / 'CH N' in agreement with the stored value.
//
// The edit runs through window.PolyMockHost.edit (editLaneParam), which mutates
// state and re-emits it; ui.js's onState → refreshAll → buildDesk rebuilds the
// strip head (the always-visible stat chip) and, because the strip stays
// expanded, re-runs buildPanes to rebuild the expr-pane channel slider — so both
// rendered labels reflect the committed value. getState().lanes[0].ch is the
// stored data model — the round-trip contract T03 mirrors on the C++
// save/restore path. The channel slider only exists in the DOM while the strip
// is expanded, so the slider-label assertions expand lane 0 first.
// ---------------------------------------------------------------------------

// The norm value the channel slider produces for an explicit MIDI channel N,
// inverting the write formula ch = round(norm * 16) - 1  →  norm = (N + 1) / 16.
const chNorm = (n) => (n + 1) / 16;

// Stored channel after committing a channel edit at norm `v` to lane 0.
async function editChannelReadCh(page, v) {
  await editLaneParam(page, 0, 'channel', v);
  return page.evaluate(() => window.PolyMockHost.getState().lanes[0].ch);
}

// The lane-0 stat-chip label text — the always-visible 'Auto'/'CH N · Nnn' chip.
async function statLabel(page) {
  return page.evaluate(() =>
    document.querySelector('.strip[data-lane="0"] .stat').textContent.trim());
}

// The lane-0 channel slider's value label (the '.v' span beside the track).
async function sliderLabel(page) {
  return page.evaluate(() => {
    const track = document.querySelector(
      '.strip[data-lane="0"] .slider-track[data-field="channel"]');
    return track ? track.nextElementSibling.textContent.trim() : null;
  });
}

test.describe('MIDI channel Auto↔explicit symmetric round-trip (mock host)', () => {
  test('norm 0 round-trips to the -1 Auto sentinel and both labels read Auto', async ({ page }) => {
    await page.goto(pageUrl);
    await expandStrip(page, 0); // channel slider only exists while expanded

    const ch = await editChannelReadCh(page, 0);
    expect(ch).toBe(-1); // Auto sentinel, not 0 or an explicit channel

    expect(await statLabel(page)).toMatch(/^Auto\b/);
    expect(await sliderLabel(page)).toBe('Auto');
  });

  test('norm (9+1)/16 round-trips to explicit channel 9 with no off-by-one', async ({ page }) => {
    await page.goto(pageUrl);
    await expandStrip(page, 0);

    const ch = await editChannelReadCh(page, chNorm(9));
    expect(ch).toBe(9);

    expect(await statLabel(page)).toContain('CH 10'); // wire 9 is DAW channel 10
    expect(await sliderLabel(page)).toBe('CH 10');
  });

  test('every explicit channel 0..15 round-trips exactly (symmetric, no drift)', async ({ page }) => {
    await page.goto(pageUrl);
    await expandStrip(page, 0);

    for (let n = 0; n <= 15; n++) {
      const ch = await editChannelReadCh(page, chNorm(n));
      expect(ch, `channel ${n} should store exactly ${n}`).toBe(n);
      expect(await sliderLabel(page)).toBe('CH ' + (n + 1));
    }
  });

  // The defect this pins: both labels printed the stored wire value, so a lane
  // reading 'CH 1' emitted wire channel 1, which every DAW shows as channel 2.
  // The engine value, the state format and the VST3 event are all correct at
  // 0-15; only the label was wrong, so the fix is +1 at display time and the
  // stored values asserted below must stay untouched.
  test('the label reads one above the stored wire channel, matching DAW numbering', async ({ page }) => {
    await page.goto(pageUrl);
    await expandStrip(page, 0);

    // Lowest explicit channel: stored 0, shown as the DAW's channel 1.
    expect(await editChannelReadCh(page, chNorm(0))).toBe(0);
    expect(await sliderLabel(page)).toBe('CH 1');

    // Highest: stored 15, shown as 16 — never 17, so the label cannot run off
    // the end of the 16-channel range.
    expect(await editChannelReadCh(page, chNorm(15))).toBe(15);
    expect(await sliderLabel(page)).toBe('CH 16');
  });

  test('toggling Auto → explicit → Auto tracks symmetrically (no stuck value)', async ({ page }) => {
    await page.goto(pageUrl);

    expect(await editChannelReadCh(page, 0)).toBe(-1); // Auto
    expect(await statLabel(page)).toMatch(/^Auto\b/);

    expect(await editChannelReadCh(page, chNorm(5))).toBe(5); // explicit 5
    expect(await statLabel(page)).toContain('CH 6'); // wire 5 is DAW channel 6

    expect(await editChannelReadCh(page, 0)).toBe(-1); // back to Auto
    expect(await statLabel(page)).toMatch(/^Auto\b/);
  });
});
