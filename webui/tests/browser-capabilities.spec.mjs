// Controls the browser cannot honour are disabled there, and say why
// (central-africa M006/S03, rows CA44–CA46).
//
// The WASM host plays its own samples, ignores MIDI routing and note length,
// and captures nothing, so Note Map, Capture bars, Arm, Channel and Duration
// did nothing in Try it. Each is now disabled in the WASM host with a reason,
// and stays enabled where the plugin's capabilities apply.
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

const HEADER_CONTROLS = ['#noteMapBtn', '#capBars', '#armBtn'];
const LANE_SLIDERS = ['channel', 'duration'];

// Open lane 1's editor on its expression pane, where Channel and Duration live.
async function openExpression(page) {
  await page.locator('.strip .ex').first().click();
  const tab = page.locator('[data-tab="expr"]').first();
  if (await tab.count()) await tab.click();
  await expect(page.locator('.slider-track[data-field="channel"]').first()).toBeAttached();
}

test.describe('M006/S03 — controls the browser cannot honour', () => {
  test('the WASM host disables them and says why', async ({ page }) => {
    await openWasmHost(page, baseUrl);
    await openExpression(page);
    for (const sel of HEADER_CONTROLS) {
      const el = page.locator(sel);
      await expect(el, `${sel} is not disabled in the browser`).toHaveAttribute('aria-disabled', 'true');
      await expect(el, `${sel} does not say why`).toHaveAttribute('title', /browser/i);
    }
    for (const field of LANE_SLIDERS) {
      const el = page.locator(`.slider-track[data-field="${field}"]`).first();
      await expect(el, `${field} is not disabled in the browser`).toHaveAttribute('aria-disabled', 'true');
      await expect(el, `${field} does not say why`).toHaveAttribute('title', /browser/i);
    }
  });

  test('a disabled control sends nothing', async ({ page }) => {
    await openWasmHost(page, baseUrl);
    await openExpression(page);
    await page.evaluate(() => {
      window.__sent = [];
      const h = window.PolyHost;
      const edit = h.edit.bind(h);
      const action = h.action.bind(h);
      h.edit = (id, v, g) => (window.__sent.push(id), edit(id, v, g));
      h.action = (n, p) => (window.__sent.push(n), action(n, p));
    });
    await page.locator('#armBtn').click({ force: true });
    await page.locator('.slider-track[data-field="channel"]').first().click({ force: true });
    expect(await page.evaluate(() => window.__sent)).toEqual([]);
  });

  test('the plugin keeps them enabled', async ({ page }) => {
    // The mock host stands in for the plugin in these specs.
    await page.goto(`${baseUrl}/index.html`);
    await page.waitForFunction(() => window.PolyHost && window.PolyHost.getState());
    await openExpression(page);
    for (const sel of HEADER_CONTROLS) {
      await expect(page.locator(sel), `${sel} is disabled with the plugin's capabilities`).not.toHaveAttribute(
        'aria-disabled',
        'true',
      );
    }
    for (const field of LANE_SLIDERS) {
      await expect(
        page.locator(`.slider-track[data-field="${field}"]`).first(),
        `${field} is disabled with the plugin's capabilities`,
      ).not.toHaveAttribute('aria-disabled', 'true');
    }
    // And the plugin host's own declaration, not only the mock's.
    const caps = await page.evaluate(() => window.PolyPluginHost.capabilities);
    for (const flag of ['canMapNotes', 'canCapture', 'canRouteChannels', 'canSetDuration']) {
      expect(caps[flag], `PolyPluginHost.capabilities.${flag}`).toBe(true);
    }
  });
});

test.describe('M006/S03 — lane rename in the browser', () => {
  const name = (page) => page.evaluate(() => window.PolyHost.getState().lanes[0].name);
  const rename = (page, n) => page.evaluate((x) => window.PolyHost.action('setLaneName', { lane: 0, name: x }), n);

  test('a lane can be renamed, and keeps its name across a scene switch', async ({ page }) => {
    await openWasmHost(page, baseUrl);
    await rename(page, 'Bell');
    expect(await name(page), 'setLaneName did not rename the lane').toBe('Bell');
    await page.evaluate(() => window.PolyHost.action('selectScene', { scene: 'B' }));
    await page.evaluate(() => window.PolyHost.action('selectScene', { scene: 'A' }));
    expect(await name(page), 'the name was lost across a scene switch').toBe('Bell');
  });

  test('an empty or over-long name is ignored', async ({ page }) => {
    await openWasmHost(page, baseUrl);
    await rename(page, 'Bell');
    await rename(page, '');
    expect(await name(page)).toBe('Bell');
    await rename(page, 'x'.repeat(16));
    expect(await name(page), 'a 16-character name was accepted; the limit is 15').toBe('Bell');
  });
});

test.describe('M006/S03 — a failed WASM load is visible', () => {
  test('blocking the engine shows a notice', async ({ page }) => {
    await page.route('**/poly_engine.wasm', (route) => route.abort());
    await page.goto(`${baseUrl}/index.html?host=wasm`);
    await page.waitForFunction(() => window.PolyHost && window.PolyHost.getState());
    const notice = page.locator('[data-wasm-fallback]');
    await expect(notice, 'the reader is not told the engine failed to load').toBeVisible();
    await expect(notice).toContainText(/did not load/i);
    await expect(notice).toContainText(/simplified preview/i);
  });

  test('no notice when the engine loads', async ({ page }) => {
    await openWasmHost(page, baseUrl);
    await expect(page.locator('[data-wasm-fallback]')).toHaveCount(0);
  });
});
