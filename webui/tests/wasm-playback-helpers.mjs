// Shared helpers for specs that drive the real WASM host and inspect the
// context that is actually playing (central-africa M006).
//
// The host keeps two engine contexts: __polyEngineCtx, which the UI reads and
// edits, and __polyPlaybackCtx, created on Play and the only one schedule()
// renders. These helpers read either one, and render a scratch copy of a
// context so a spec can compare what two states would sound like without
// advancing the playing context itself.
import { spawn } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
import process from 'node:process';

export const WEBUI_DIR = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const PYTHON = process.platform === 'win32' ? 'python' : 'python3';

// A static server per suite: WebAssembly streaming instantiation refuses
// file:// URLs in Chromium.
export async function startServer() {
  const server = spawn(PYTHON, ['-u', '-m', 'http.server', '0', '--directory', WEBUI_DIR], {
    stdio: ['ignore', 'pipe', 'pipe'],
  });
  const baseUrl = await new Promise((resolve, reject) => {
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
  return { server, baseUrl };
}

// Load the WASM host, refuse the mock fallback, and optionally a preset.
export async function openWasmHost(page, baseUrl, query = '') {
  await page.goto(`${baseUrl}/index.html?host=wasm${query}`);
  await page.waitForFunction(() => window.__polyModule && window.__polyEngineCtx && window.PolyHost, null, {
    timeout: 15000,
  });
  const isMock = await page.evaluate(() => window.PolyHost === window.PolyMockHost);
  if (isMock) throw new Error('expected the WASM host, got the mock: the WASM failed to load');
}

export async function applyPresetByName(page, name) {
  await page.evaluate((n) => {
    const idx = window.PolyHost.getState().presets.findIndex((p) => p.name === n);
    if (idx < 0) throw new Error(`no preset named ${n}`);
    window.PolyHost.action('applyPreset', { index: idx });
  }, name);
}

// Start playback and wait until the playing context exists.
export async function play(page) {
  await page.evaluate(() => window.PolyHost.action('togglePlay', {}));
  await page.waitForFunction(() => window.__polyPlaybackCtx, null, { timeout: 5000 });
}

// Installed in the page once; every helper below calls into it.
export async function installProbes(page) {
  await page.evaluate(() => {
    const M = window.__polyModule;
    window.__m006 = {
      laneConfig(ctx, lane) {
        const ints = [];
        for (let f = 0; f < 15; f++) ints.push(M._poly_lane_int(ctx, lane, f));
        const floats = [];
        for (let f = 0; f < 14; f++) floats.push(Math.round(M._poly_lane_float(ctx, lane, f) * 1e6) / 1e6);
        const buf = M._malloc(64 * 4);
        const len = M._poly_lane_pattern(ctx, lane, buf, 64);
        const pattern = Array.from(M.HEAP32.subarray(buf >> 2, (buf >> 2) + Math.max(0, len)));
        const flen = M._poly_lane_fixed_pattern(ctx, lane, buf, 64);
        const fixed = Array.from(M.HEAP32.subarray(buf >> 2, (buf >> 2) + Math.max(0, flen)));
        M._free(buf);
        const f32 = (ptr, n) => (ptr ? Array.from(M.HEAPF32.subarray(ptr >> 2, (ptr >> 2) + n)).map((v) => v.toFixed(4)) : []);
        const cellsPtr = M._poly_lane_cells_ptr(ctx, lane);
        const cells = cellsPtr ? Array.from(M.HEAP32.subarray(cellsPtr >> 2, (cellsPtr >> 2) + ints[12])) : [];
        const envelopes = [];
        for (let i = 0; i < M._poly_lane_envelope_count(ctx, lane); i++) {
          envelopes.push([
            M._poly_lane_envelope_target(ctx, lane, i),
            M._poly_lane_envelope_period(ctx, lane, i).toFixed(4),
            M._poly_lane_envelope_depth(ctx, lane, i).toFixed(4),
            M._poly_lane_envelope_active(ctx, lane, i),
          ]);
        }
        return JSON.stringify({
          ints,
          floats,
          pattern,
          fixed,
          cells,
          microTiming: f32(M._poly_lane_micro_timing_ptr(ctx, lane), 64),
          accents: f32(M._poly_lane_accents_ptr(ctx, lane), 64),
          envelopes,
          seed: String(M._poly_seed(ctx)),
          activeLanes: M._poly_active_lane_count(ctx),
        });
      },
      renderSignature(src, quarters = 16) {
        const scratch = M._poly_create();
        M._poly_copy_scenes(scratch, src);
        for (let i = 0; i < 6; i++) M._poly_set_macro(scratch, i, M._poly_macro_value(src, i));
        const out = [];
        for (let q = 0; q < quarters; q++) {
          const n = M._poly_render(scratch, q, q + 1, 120, 44100, 512, 1, 0, 0, 0, q === 0 ? 1 : 0);
          const base = M._poly_event_buffer(scratch) >> 3;
          for (let i = 0; i < n; i++) {
            const o = base + i * 6;
            out.push(
              [M.HEAPF64[o].toFixed(4), M.HEAPF64[o + 1] | 0, M.HEAPF64[o + 2].toFixed(3), M.HEAPF64[o + 5] | 0].join(':'),
            );
          }
        }
        M._poly_destroy(scratch);
        return out.join(' ');
      },
    };
  });
}

export const laneConfig = (page, which, lane) =>
  page.evaluate(
    ([w, l]) => window.__m006.laneConfig(w === 'playback' ? window.__polyPlaybackCtx : window.__polyEngineCtx, l),
    [which, lane],
  );

export const renderSignature = (page, which) =>
  page.evaluate(
    (w) => window.__m006.renderSignature(w === 'playback' ? window.__polyPlaybackCtx : window.__polyEngineCtx),
    which,
  );
