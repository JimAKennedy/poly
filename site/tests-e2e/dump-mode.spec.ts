import { test, expect } from '@playwright/test';

// S12 T02 live-browser verification. Confirms the pieces that unit tests
// cannot: the real URL query is read at scheduler start(), the DOM download
// shim (URL.createObjectURL + <a>.click) actually fires, and the reload-
// without-dump path stays silent.
//
// Deep House is chosen for its 4-beat loopBeats → 8 loops = ~16s at 120 BPM,
// keeping the wall-clock wait comfortably under the 60s test timeout.

const DUMP_URL = '/09-electronic/?dump=1';
const CLEAN_URL = '/09-electronic/';
const CARD_SELECTOR = '.poly-preview[data-poly-preset="Deep House"]';

const TRYIT_DUMP_URL = '/13-drum-and-bass/?dump=1';
const TRYIT_CARD_SELECTOR = '.poly-preview[data-poly-preset="Jungle Break"]';
const TRYIT_IFRAME_URL_PATTERN = /\/webui\/index\.html\?/;

// Extra headroom over 8 loops * 4 beats * (60/120) = 16s wall time; the
// scheduler lookahead nudges the fire time ~100ms earlier, but we want a
// generous ceiling so a slow decode step doesn't flake the assertion.
const DUMP_WAIT_TIMEOUT_MS = 30_000;
const SILENT_WAIT_MS = 18_000;

interface CapturedDownload {
  filename: string;
  base64: string;
}

// Registered as an init script so it runs before ANY page script — including
// the Astro-hydrated card handler that would otherwise trigger the real
// download before the shim is in place.
const DOWNLOAD_SHIM = `
  (() => {
    const captures = [];
    (window).__dumpCaptures = captures;
    const origCreate = URL.createObjectURL.bind(URL);
    const blobsByUrl = new Map();
    URL.createObjectURL = (blob) => {
      const url = origCreate(blob);
      blobsByUrl.set(url, blob);
      return url;
    };
    const origClick = HTMLAnchorElement.prototype.click;
    HTMLAnchorElement.prototype.click = function () {
      const blob = blobsByUrl.get(this.href);
      if (blob && this.download) {
        blob.arrayBuffer().then((buf) => {
          const bytes = new Uint8Array(buf);
          let bin = '';
          for (let i = 0; i < bytes.length; i++) bin += String.fromCharCode(bytes[i]);
          captures.push({ filename: this.download, base64: btoa(bin) });
        });
        // Skip the real navigation attempt — we already have the bytes.
        return;
      }
      return origClick.apply(this, arguments);
    };
  })();
`;

async function waitForCapture(page: import('@playwright/test').Page): Promise<CapturedDownload> {
  const raw = await page.waitForFunction(
    () => {
      const c = (window as unknown as { __dumpCaptures?: CapturedDownload[] }).__dumpCaptures;
      return c && c.length > 0 ? c[0] : null;
    },
    null,
    { timeout: DUMP_WAIT_TIMEOUT_MS, polling: 250 },
  );
  return (await raw.jsonValue()) as CapturedDownload;
}

function decodeBase64(b64: string): Uint8Array {
  const bin = Buffer.from(b64, 'base64');
  return new Uint8Array(bin);
}

test.describe('S12 T02 dump mode — live browser', () => {
  test('?dump=1 downloads a valid SMF blob after 8 loops', async ({ page }) => {
    await page.addInitScript(DOWNLOAD_SHIM);
    await page.goto(DUMP_URL);

    const card = page.locator(CARD_SELECTOR);
    await expect(card).toBeVisible();
    await expect(card).toHaveAttribute('data-state', 'stopped');
    const playBtn = card.locator('.poly-preview-play');
    await expect(playBtn).toBeEnabled();
    await playBtn.click();
    await expect(card).toHaveAttribute('data-state', 'playing');

    const capture = await waitForCapture(page);

    // Filename shape: dump-play-<slug>-<timestamp>.mid. Deep House slugifies to
    // deep-house; timestamp is a Date.now() millisecond value.
    expect(capture.filename).toMatch(/^dump-play-deep-house-\d{10,}\.mid$/);

    // SMF header: 'MThd' then 6-byte length, format, tracks, ppq. Body must
    // include 'MTrk' for the single track.
    const bytes = decodeBase64(capture.base64);
    expect(bytes.length).toBeGreaterThan(22);
    const header = String.fromCharCode(...bytes.slice(0, 4));
    expect(header).toBe('MThd');
    const asString = String.fromCharCode(...bytes.slice(0, Math.min(bytes.length, 64)));
    expect(asString).toContain('MTrk');

    // ppq lives at bytes 12..13 (big-endian). writeSMF defaults to 480.
    const ppq = (bytes[12] << 8) | bytes[13];
    expect(ppq).toBe(480);
  });

  test('Try It ?dump=1 downloads a scratch-context SMF via the WASM host', async ({ page }) => {
    // addInitScript is applied to every frame (parent + iframe) so the WASM
    // Try It host's downloadSMF gets shimmed the same way as the site card path.
    await page.addInitScript(DOWNLOAD_SHIM);
    await page.goto(TRYIT_DUMP_URL);

    const card = page.locator(TRYIT_CARD_SELECTOR);
    await expect(card).toBeVisible();
    const tryBtn = card.locator('.poly-preview-btn');
    await expect(tryBtn).toBeEnabled();
    await tryBtn.click();

    // The Try It modal loads /webui/index.html in an iframe. Wait for it
    // to appear, then wait for #play inside the frame (host boot is async).
    const frameLocator = page.frameLocator('iframe.poly-modal-frame');
    const playBtn = frameLocator.locator('#play');
    await expect(playBtn).toBeVisible({ timeout: 20_000 });
    // Give the WASM host + presets.json a beat to settle before clicking.
    // Boot completes when currentPresetIndex is set; verify via the ready hook.
    await page.waitForFunction(
      () => {
        const frames = Array.from(document.querySelectorAll('iframe.poly-modal-frame'));
        for (const f of frames) {
          const w = (f as HTMLIFrameElement).contentWindow as
            | (Window & { PolyWasmHost?: { getState?: () => { preset?: string } } })
            | null;
          if (w && w.PolyWasmHost && typeof w.PolyWasmHost.getState === 'function') {
            const s = w.PolyWasmHost.getState();
            if (s && typeof s.preset === 'string' && s.preset.length > 0) return true;
          }
        }
        return false;
      },
      null,
      { timeout: 20_000, polling: 250 },
    );
    await playBtn.click();

    // Poll the iframe's window for the dump capture. The shim is installed by
    // the same addInitScript so __dumpCaptures lives on the iframe's window.
    const capture = await page.waitForFunction(
      (pattern) => {
        const frames = Array.from(document.querySelectorAll('iframe.poly-modal-frame'));
        for (const f of frames) {
          const src = (f as HTMLIFrameElement).src;
          if (!new RegExp(pattern).test(src)) continue;
          const w = (f as HTMLIFrameElement).contentWindow as
            | (Window & { __dumpCaptures?: CapturedDownload[] })
            | null;
          const c = w && w.__dumpCaptures;
          if (c && c.length > 0) return c[0];
        }
        return null;
      },
      TRYIT_IFRAME_URL_PATTERN.source,
      { timeout: DUMP_WAIT_TIMEOUT_MS, polling: 250 },
    );
    const cap = (await capture.jsonValue()) as CapturedDownload;

    expect(cap.filename).toMatch(/^dump-tryit-[a-z0-9-]+-\d{10,}\.mid$/);

    const bytes = decodeBase64(cap.base64);
    expect(bytes.length).toBeGreaterThan(22);
    const header = String.fromCharCode(...bytes.slice(0, 4));
    expect(header).toBe('MThd');
    const asString = String.fromCharCode(...bytes.slice(0, Math.min(bytes.length, 64)));
    expect(asString).toContain('MTrk');
    const ppq = (bytes[12] << 8) | bytes[13];
    expect(ppq).toBe(480);
  });

  test('reload without ?dump=1 fires no download', async ({ page }) => {
    await page.addInitScript(DOWNLOAD_SHIM);
    await page.goto(CLEAN_URL);

    const card = page.locator(CARD_SELECTOR);
    await expect(card).toBeVisible();
    const playBtn = card.locator('.poly-preview-play');
    await playBtn.click();
    await expect(card).toHaveAttribute('data-state', 'playing');

    // Wait past the 8-loop boundary — the dump path must stay dormant.
    await page.waitForTimeout(SILENT_WAIT_MS);
    const captures = await page.evaluate(
      () => (window as unknown as { __dumpCaptures?: CapturedDownload[] }).__dumpCaptures ?? [],
    );
    expect(captures).toEqual([]);
  });
});
