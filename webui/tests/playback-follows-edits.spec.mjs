// Edits reach the playing engine (central-africa M006/S01, rows CA38–CA40).
//
// The WASM host edits __polyEngineCtx and plays __polyPlaybackCtx, a context
// seeded from it on Play. An edit that writes only the first is visible in
// the editor and inaudible until Stop and Play — the reported VEL defect.
// Each case edits a control while playing and asserts the playing context
// now holds the same lane state as the edited one; the VEL and probability
// cases also assert that what the playing context renders changes.
import { test, expect } from '@playwright/test';

import {
  startServer,
  openWasmHost,
  applyPresetByName,
  play,
  installProbes,
  laneConfig,
  renderSignature,
} from './wasm-playback-helpers.mjs';

// Red until M006/S01 Task 2 mirrors edits into the playing context.
const RED_UNTIL_TASK_2 = true;

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
  await applyPresetByName(page, 'Deep House');
  await installProbes(page);
  await play(page);
}

const EDITS = [
  ['lane.0.velocity', 0.2],
  ['lane.0.probability', 0.3],
  ['lane.0.hits', 0.03],
  ['lane.0.steps', 0.2],
  ['lane.0.rotation', 0.3],
  ['lane.0.ghostFloor', 0.5],
  ['lane.0.spread', 0.6],
  ['lane.0.swing', 0.4],
  ['lane.0.humanize', 0.3],
  ['lane.0.note', 0.4],
  ['lane.0.timeline', 'toggle'],
  ['lane.0.mutationRate', 0.5],
  ['lane.0.driftRate', 0.7],
  ['lane.0.timingOffset', 0.8],
  ['lane.0.phraseLength', 0.5],
  ['lane.0.tempoMult', 0.5],
  ['lane.0.emphasisProb', 0.95],
  ['lane.0.subdivision', 0.75],
  ['lane.0.duration', 0.5],
];

const ACTIONS = [
  ['toggleStep', { lane: 0, step: 1 }],
  ['setEuclid', { lane: 0, steps: 12, hits: 5, rotation: 2 }],
  ['setMicroTiming', { lane: 0, step: 0, ms: 8 }],
  ['setAccent', { lane: 0, step: 0, value: 0.8 }],
  ['setEnvelope', { lane: 0, index: 0, envelope: { target: 'Velocity', period: 4, depth: 0.5, on: true } }],
  ['setCells', { lane: 0, cells: [2, 2, 3] }],
];

test.describe('M006/S01 — edits reach the playing engine', () => {
  for (const [paramId, value] of EDITS) {
    test(`edit ${paramId} reaches the playing context`, async ({ page }) => {
      test.fail(RED_UNTIL_TASK_2, 'edits do not reach the playing context yet');
      await playing(page);
      const before = await laneConfig(page, 'engine', 0);
      const v =
        value === 'toggle' ? (JSON.parse(before).ints[10] ? 0 : 1) : value;
      await page.evaluate(([p, x]) => window.PolyHost.edit(p, x, 'perform'), [paramId, v]);
      expect(await laneConfig(page, 'engine', 0), `${paramId} did not change the edited lane, so this case proves nothing`).not.toBe(before);
      expect(await laneConfig(page, 'playback', 0), `${paramId} did not reach the playing engine`).toBe(
        await laneConfig(page, 'engine', 0),
      );
    });
  }

  for (const [name, payload] of ACTIONS) {
    test(`action ${name} reaches the playing context`, async ({ page }) => {
      test.fail(RED_UNTIL_TASK_2, 'edits do not reach the playing context yet');
      await playing(page);
      const before = await laneConfig(page, 'engine', 0);
      await page.evaluate(([n, p]) => window.PolyHost.action(n, p), [name, payload]);
      expect(await laneConfig(page, 'engine', 0), `${name} did not change the edited lane, so this case proves nothing`).not.toBe(before);
      expect(await laneConfig(page, 'playback', 0), `${name} did not reach the playing engine`).toBe(
        await laneConfig(page, 'engine', 0),
      );
    });
  }

  test('a seed change reaches the playing context', async ({ page }) => {
    test.fail(RED_UNTIL_TASK_2, 'edits do not reach the playing context yet');
    await playing(page);
    await page.evaluate(() => window.PolyHost.edit('seed', 0.42, 'perform'));
    const seeds = await page.evaluate(() => [
      String(window.__polyModule._poly_seed(window.__polyPlaybackCtx)),
      String(window.__polyModule._poly_seed(window.__polyEngineCtx)),
    ]);
    expect(seeds[0], 'the seed did not reach the playing engine').toBe(seeds[1]);
  });

  for (const [paramId, value] of [
    ['lane.0.velocity', 0.2],
    ['lane.0.probability', 0.3],
  ]) {
    test(`${paramId} changes what is playing`, async ({ page }) => {
      test.fail(RED_UNTIL_TASK_2, 'edits do not reach the playing context yet');
      await playing(page);
      const before = await renderSignature(page, 'playback');
      await page.evaluate(([p, v]) => window.PolyHost.edit(p, v, 'perform'), [paramId, value]);
      expect(await renderSignature(page, 'playback'), `${paramId} did not change the sound`).not.toBe(before);
    });
  }
});
