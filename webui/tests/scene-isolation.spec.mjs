// M043 S14 T01 verification: A/B scene isolation on preset load.
//
// Before the fix, `applyPreset` unconditionally called
// `_poly_action_apply_preset` which overwrote the engine's single
// GrooveState regardless of scene selection — so loading a preset into
// scene B clobbered scene A. This spec drives the wasm-host through the
// scene-swap sequence from the S14 T01 plan and asserts each scene keeps
// its own preset. It exercises the real WASM engine (?host=wasm), not the
// mock. Broader control-audit assertions live in T05's control-audit.spec.
//
// A short-lived static server is spun up per-suite because WebAssembly
// streaming instantiation refuses file:// URLs in Chromium and every
// other WASM-driven check in this repo does the same.
import { test, expect } from '@playwright/test';
import { spawn } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import path from 'node:path';

const WEBUI_DIR = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
let server;
let baseUrl;

test.describe('M043 S14 T01 — A/B scene isolation', () => {
  test.beforeAll(async () => {
    // `-u` forces unbuffered output; without it the "Serving HTTP on ..."
    // banner (which we scrape for the auto-assigned port) doesn't reach the
    // pipe in time.
    server = spawn('python3', ['-u', '-m', 'http.server', '0', '--directory', WEBUI_DIR], {
      stdio: ['ignore', 'pipe', 'pipe'],
    });
    baseUrl = await new Promise((resolve, reject) => {
      const timer = setTimeout(() => reject(new Error('http.server did not report a port')), 5000);
      const onChunk = (chunk) => {
        const match = chunk.toString().match(/port (\d+)/);
        if (match) {
          clearTimeout(timer);
          resolve(`http://127.0.0.1:${match[1]}`);
        }
      };
      server.stderr.on('data', onChunk);
      server.stdout.on('data', onChunk);
      server.on('error', (err) => {
        clearTimeout(timer);
        reject(err);
      });
    });
  });

  test.afterAll(() => {
    if (server && !server.killed) server.kill('SIGTERM');
  });

  test('scene B preset load leaves scene A intact and vice versa', async ({ page }) => {
    await page.goto(`${baseUrl}/index.html?host=wasm`);
    await page.waitForFunction(() => window.__polyAudioProbe && window.PolyHost, null, {
      timeout: 15000,
    });

    // Confirm we actually got the WASM host, not the mock fallback. Without
    // this the rest of the test could pass against the mock and give a
    // false-positive on the isolation fix.
    const hostInfo = await page.evaluate(() => ({
      isMock: window.PolyHost === window.PolyMockHost,
      scene: window.PolyHost.getState().scene,
    }));
    expect(hostInfo.isMock, 'expected WASM host, got mock — WASM load failed').toBe(false);
    expect(hostInfo.scene).toBe('A');

    // Apply preset index 0 into scene A.
    const sceneAName = await page.evaluate(() => {
      window.PolyHost.action('applyPreset', { index: 0 });
      return window.__polyAudioProbe.sceneAPreset;
    });
    expect(sceneAName).not.toBe('');
    expect(sceneAName).not.toBe('Init');

    // Switch to scene B and apply a different preset. The regression this
    // guards against: this apply would previously overwrite scene A too.
    const sceneBName = await page.evaluate(() => {
      window.PolyHost.action('selectScene', { scene: 'B' });
      window.PolyHost.action('applyPreset', { index: 1 });
      return window.__polyAudioProbe.sceneBPreset;
    });
    expect(sceneBName).not.toBe('');
    expect(sceneBName).not.toBe(sceneAName);

    // Switching back to A must reveal A's original preset untouched.
    const observedAAfterSwap = await page.evaluate(() => {
      window.PolyHost.action('selectScene', { scene: 'A' });
      return window.__polyAudioProbe.currentPreset;
    });
    expect(observedAAfterSwap).toBe(sceneAName);

    // And switching back to B must still show B's preset.
    const observedBAfterSwap = await page.evaluate(() => {
      window.PolyHost.action('selectScene', { scene: 'B' });
      return window.__polyAudioProbe.currentPreset;
    });
    expect(observedBAfterSwap).toBe(sceneBName);
  });
});
