import { chromium } from '@playwright/test';

const BASE = 'http://localhost:4321';
const PAGE_URL = BASE + '/poly/13-drum-and-bass/';

const browser = await chromium.launch({
  headless: true,
  args: ['--autoplay-policy=no-user-gesture-required'],
});
const context = await browser.newContext();
const page = await context.newPage();

const logs = [];
page.on('console', (msg) => logs.push({ type: msg.type(), text: msg.text() }));
page.on('pageerror', (err) => logs.push({ type: 'pageerror', text: err.message }));

await page.goto(PAGE_URL, { waitUntil: 'networkidle' });

const card = page.locator('.poly-preview[data-poly-preset="Jungle Break"]');
await card.locator('.poly-preview-btn').click();

const dialog = page.locator('dialog.poly-modal');
await dialog.waitFor({ state: 'attached' });

const frame = await dialog.locator('iframe').elementHandle().then((h) => h.contentFrame());
await frame.waitForLoadState('networkidle', { timeout: 15000 });

// Wait for host initialization.
await frame.waitForFunction(() => window.PolyHost && window.PolyHost.getState && window.PolyHost.getState().presets?.length > 0, null, { timeout: 10000 });

// Click Play button in the modal UI.
const playClicked = await frame.evaluate(() => {
  const btn = document.querySelector('#play');
  if (!btn) return { clicked: false, reason: 'no #play button' };
  btn.click();
  return { clicked: true };
});
console.log('PLAY_CLICKED', JSON.stringify(playClicked));

// Give scheduler + sample loading time.
await page.waitForTimeout(3000);

const audio = await frame.evaluate(() => {
  const p = window.__polyAudioProbe;
  return {
    nodesStarted: p?.nodesStarted ?? null,
    samplesLoaded: p?.samplesLoaded ?? null,
    fallbackActive: p?.fallbackActive ?? null,
    currentPreset: p?.currentPreset ?? null,
    tempo: window.PolyHost?.getState()?.tempo ?? null,
    lanes: (window.PolyHost?.getState()?.lanes ?? []).map((l) => l.name),
  };
});
console.log('AUDIO_STATE', JSON.stringify(audio, null, 2));

const warnings = logs.filter((l) => /wasm|sample|manifest|synth|fallback/i.test(l.text));
console.log('AUDIO_LOGS', JSON.stringify(warnings, null, 2));

await browser.close();
