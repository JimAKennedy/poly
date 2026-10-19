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
