import { test, expect } from '@playwright/test';
import { pageUrl } from './test-helpers.mjs';

// first-release M004/S01, FR26. The lane head is a fixed-height box so every
// strip's ring starts at the same Y; a name or role that wraps clips rather
// than reflows. The plugin always shows the per-lane export handle (canExport
// is unconditional there; the mock surfaces it via ?export=1), and that third
// button narrowed the name column enough that a two-word role wrapped — and
// because the name was a shrinkable flex item, the name was what got clipped:
// BELL rendered as a strip of pixels while "Anchor pulse" stayed whole.
//
// Three properties, on every lane, with and without the handle: the name's
// box lies inside the head's box, the name element is as tall as its own text
// (a shrunken flex item hides its overflow inside itself, so the column can
// look tidy while the name is a sliver — the first draft of this guard checked
// only the column and passed on the broken CSS), and the name column does not
// overflow. None pins a pixel height, so a redesign that keeps names legible
// passes.

for (const [label, url] of [
  ['mock default', pageUrl],
  ['with the per-lane export handle', `${pageUrl}?export=1`],
]) {
  test(`every lane name fits inside its head — ${label}`, async ({ page }) => {
    await page.goto(url);
    await page.locator('.strip[data-lane]').nth(4).waitFor();
    const lanes = await page.evaluate(() =>
      [...document.querySelectorAll('.strip[data-lane]')].map((strip) => {
        const head = strip.querySelector('.head').getBoundingClientRect();
        const nm = strip.querySelector('.nm');
        const b = strip.querySelector('.nm b');
        const name = b.getBoundingClientRect();
        return {
          lane: strip.dataset.lane,
          text: b.textContent.trim(),
          nameInsideHead: name.top >= head.top - 0.5 && name.bottom <= head.bottom + 0.5,
          nameShrunk: Math.max(0, b.scrollHeight - b.clientHeight),
          overflow: Math.max(0, nm.scrollHeight - nm.clientHeight),
          buttons: strip.querySelectorAll('.head button').length,
        };
      }),
    );
    expect(lanes.length).toBeGreaterThan(0);
    if (label !== 'mock default') {
      // The seam must actually have mounted the handle, or the case is vacuous.
      expect(lanes.every((l) => l.buttons === 3)).toBe(true);
    }
    const clipped = lanes.filter((l) => !l.nameInsideHead).map((l) => `${l.text} (lane ${l.lane})`);
    expect(clipped, 'lane names clipped by their head').toEqual([]);
    const shrunk = lanes.filter((l) => l.nameShrunk > 0).map((l) => `${l.text}: ${l.nameShrunk}px of text hidden`);
    expect(shrunk, 'lane names shorter than their own text').toEqual([]);
    const overflowing = lanes.filter((l) => l.overflow > 0).map((l) => `${l.text}: ${l.overflow}px`);
    expect(overflowing, 'name columns taller than their head').toEqual([]);
  });
}
