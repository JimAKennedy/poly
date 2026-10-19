// The editor fits a laptop (central-africa M006/S04, row CA47).
//
// Each step bar had min-height 6px and the ladder no overflow rule, so a
// 24-step lane needed about 213px and, given less, spilled upward over its
// title and dial. Every other WebUI spec runs at 1280×840 with at most 16
// steps, so nothing caught it.
import { test, expect } from '@playwright/test';

import { startServer, openWasmHost, applyPresetByName } from './wasm-playback-helpers.mjs';

let server;
let baseUrl;

test.beforeAll(async () => {
  ({ server, baseUrl } = await startServer());
});

test.afterAll(() => {
  if (server && !server.killed) server.kill('SIGTERM');
});

// For every strip: its ladder's top bar sits below its dial, and the ladder
// lies inside the strip.
async function laddersInsideStrips(page) {
  return page.evaluate(() =>
    Array.from(document.querySelectorAll('.strip')).flatMap((strip, i) => {
      const ring = strip.querySelector('svg.ring');
      const ladder = strip.querySelector('.ladder');
      if (!ring || !ladder || !ladder.lastElementChild) return [];
      const top = ladder.lastElementChild.getBoundingClientRect(); // column-reverse: last is highest
      const r = ring.getBoundingClientRect();
      const l = ladder.getBoundingClientRect();
      const s = strip.getBoundingClientRect();
      const out = [];
      if (top.top < r.bottom - 0.5) out.push(`strip ${i + 1}: top step bar at ${top.top} is above the dial's bottom ${r.bottom}`);
      if (l.top < s.top - 0.5 || l.bottom > s.bottom + 0.5) out.push(`strip ${i + 1}: ladder leaves the strip`);
      return out;
    }),
  );
}

test.describe('M006/S04 — the editor fits a laptop', () => {
  test.use({ viewport: { width: 1200, height: 520 } });

  test('a 24-step ladder stays inside its card at 1200×520', async ({ page }) => {
    await openWasmHost(page, baseUrl);
    await applyPresetByName(page, 'Aka: Ngbolu');
    await expect(page.locator('.strip')).toHaveCount(3);
    const steps = await page.evaluate(() => window.PolyHost.getState().lanes.map((l) => l.steps));
    expect(steps, 'Aka: Ngbolu should carry a 24-step lane').toContain(24);
    expect(await laddersInsideStrips(page)).toEqual([]);
  });
});
