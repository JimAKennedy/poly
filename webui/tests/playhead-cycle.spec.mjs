// The playhead and the sound agree (central-africa M006/S05, row CA49).
//
// The WASM host rounded a lane's step to whole eighths
// (Math.round(8 / subdivision)) and ignored Tempo Mult, so a 1/16 lane got 1
// rather than 0.5 and a 1/12 lane 1 rather than 2/3. The audio sits at the
// engine's own ppq and was right; the playhead's cycle was not. The engine's
// step is 8 / subdivision / tempoMultiplier eighths.
import { test, expect } from '@playwright/test';

import { startServer, openWasmHost } from './wasm-playback-helpers.mjs';

let server;
let baseUrl;

test.beforeAll(async () => {
  ({ server, baseUrl } = await startServer());
});

test.afterAll(() => {
  if (server && !server.killed) server.kill('SIGTERM');
});

test.describe('M006/S05 — the playhead and the sound agree', () => {
  test("every factory preset's lanes cycle as the engine's do", async ({ page }) => {
    await openWasmHost(page, baseUrl);
    const failures = await page.evaluate(() => {
      const M = window.__polyModule;
      const ctx = window.__polyEngineCtx;
      const { cyc8 } = window.PolyGrooveMath;
      const out = [];
      const presets = window.PolyHost.getState().presets;
      presets.forEach((p, idx) => {
        window.PolyHost.action('applyPreset', { index: idx });
        window.PolyHost.getState().lanes.forEach((lane, li) => {
          if (lane.cells) return;
          const sub = M._poly_lane_int(ctx, li, 7);
          const tm = M._poly_lane_float(ctx, li, 12) || 1;
          const engine = (lane.steps * 8) / sub / tm;
          if (Math.abs(cyc8(lane) - engine) > 1e-9) out.push(`${p.name} lane ${li + 1}: ${cyc8(lane)} vs ${engine}`);
        });
      });
      return { failures: out, presets: presets.length };
    });
    expect(failures.presets, 'expected all factory presets').toBe(56);
    expect(failures.failures, `${new Set(failures.failures.map((f) => f.split(' lane')[0])).size} presets disagree`).toEqual(
      [],
    );
  });
});

test.describe('M006/S05 — the playhead lands on the sounding step', () => {
  test('Zande: Kponingbo at four eighths shows lane 1 back at step 0', async ({ page }) => {
    await openWasmHost(page, baseUrl, '&preset=Zande%3A%20Kponingbo');
    await page.waitForFunction(() => window.PolyHost.getState().preset === 'Zande: Kponingbo');
    const step = await page.evaluate(
      () =>
        new Promise((resolve) => {
          window.__polyHoldClock8 = 4; // an 8-step 1/16 lane completes one cycle every 4 eighths
          window.PolyHost.onFrame((f) => resolve({ t8: f.t8, step: f.lanes[0].step }));
        }),
    );
    expect(step.t8, 'the clock was not held at four eighths, so this case proves nothing').toBe(4);
    expect(step.step, 'the playhead is half a cycle from the sound').toBe(0);
  });

  // A 1/12 step is 2/3 of an eighth, which floating point cannot hold exactly,
  // so a modulo test misses most of its steps; 1/16's 0.5 is exact.
  test('the preview finds a hit on every step of a 1/12 lane', async ({ page }) => {
    await openWasmHost(page, baseUrl);
    const steps = await page.evaluate(() => {
      const { laneHitAt, stepEighths } = window.PolyGrooveMath;
      const lane = { steps: 12, stepLen: stepEighths(12), pattern: new Array(12).fill(1) };
      const found = [];
      for (let i = 0; i < 12; i++) {
        const hit = laneHitAt(lane, i * lane.stepLen);
        if (hit) found.push(hit.step);
      }
      return found;
    });
    expect(steps, 'laneHitAt skipped steps of a sub-eighth lane').toEqual([...Array(12).keys()]);
  });

  test("a 1/16 lane's step glyph is not the eighth note", async ({ page }) => {
    await openWasmHost(page, baseUrl, '&preset=Zande%3A%20Kponingbo');
    await page.waitForFunction(() => window.PolyHost.getState().preset === 'Zande: Kponingbo');
    await page.locator('.strip .ex').first().click();
    const glyph = await page.locator('.stepper .v').first().textContent();
    expect(glyph, 'the Steps row shows a 1/16 lane as eighths').not.toContain('♪');
    expect(glyph).toContain('8 ×');
  });
});
