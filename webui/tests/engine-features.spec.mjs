// The engine features the browser host dropped (central-africa M006/S02,
// rows CA41–CA43). Fill Every N, Fill Now, Seed Lock and − Lane / + Lane reach
// the plugin's engine; on the WASM host they were dropped or unknown. Each case
// plays Aka: Ngbolu, whose macros leave its Euclidean lanes as written and
// whose first lane, E(1,3), has off-pattern steps for a fill to add.
import { test, expect } from '@playwright/test';

import {
  startServer,
  openWasmHost,
  applyPresetByName,
  play,
  installProbes,
  renderSignature,
} from './wasm-playback-helpers.mjs';

let server;
let baseUrl;

test.beforeAll(async () => {
  ({ server, baseUrl } = await startServer());
});

test.afterAll(() => {
  if (server && !server.killed) server.kill('SIGTERM');
});

async function playing(page) {
  await openWasmHost(page, baseUrl);
  await applyPresetByName(page, 'Aka: Ngbolu');
  await installProbes(page);
  await play(page);
}

const edit = (page, paramId, value) =>
  page.evaluate(([p, v]) => window.PolyHost.edit(p, v, 'perform'), [paramId, value]);

// Events the playing context renders on one lane, from a scratch copy.
const laneEvents = async (page, lane) =>
  (await renderSignature(page, 'playback'))
    .split(' ')
    .filter((e) => e && e.endsWith(`:${lane}`))
    .join(' ');

test.describe('M006/S02 — engine features in the browser', () => {
  test('Fill Every N changes what is playing', async ({ page }) => {
    await playing(page);
    const before = await laneEvents(page, 0);
    await edit(page, 'lane.0.fillEveryN', 1 / 64);
    expect(await laneEvents(page, 0), 'Fill Every 1 did not change the playing lane').not.toBe(before);
  });

  test('Fill Now reaches the playing context', async ({ page }) => {
    await playing(page);
    const seen = await page.evaluate(() => {
      const M = window.__polyModule;
      const calls = [];
      const real = M._poly_trigger_manual_fill;
      M._poly_trigger_manual_fill = (ctx) => {
        calls.push(ctx);
        return real(ctx);
      };
      window.PolyHost.action('manualFill', {});
      M._poly_trigger_manual_fill = real;
      return { calls, playback: window.__polyPlaybackCtx };
    });
    expect(seen.calls, 'Fill Now did not reach the engine').toContain(seen.playback);
  });

  test('Seed Lock holds a lane through a reroll', async ({ page }) => {
    await playing(page);
    await edit(page, 'lane.0.probability', 0.6);
    await edit(page, 'lane.1.probability', 0.6);
    await edit(page, 'lane.0.seedLock', 1);
    const locked = await laneEvents(page, 0);
    const other = await laneEvents(page, 1);
    await edit(page, 'seed', 0.9);
    expect(await laneEvents(page, 0), 'the locked lane re-rolled with the seed').toBe(locked);
    expect(await laneEvents(page, 1), 'the unlocked lane did not re-roll, so this case proves nothing').not.toBe(other);
    const state = await page.evaluate(() => window.PolyHost.getState().lanes[0].seedLocked);
    expect(state, 'the editor does not show the lane as locked').toBe(true);
  });

  test('− Lane / + Lane change the number of lanes that play', async ({ page }) => {
    await playing(page);
    await edit(page, 'activeLaneCount', 1 / 7);
    const lanes = await page.evaluate(() => window.__polyModule._poly_active_lane_count(window.__polyPlaybackCtx));
    expect(lanes, 'the playing engine did not take the lane count').toBe(2);
    expect(await laneEvents(page, 2), 'lane 3 still sounds with two lanes active').toBe('');
    expect(await laneEvents(page, 0), 'lane 1 went silent').not.toBe('');
  });
});
