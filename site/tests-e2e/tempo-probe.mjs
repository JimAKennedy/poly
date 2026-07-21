// Ad-hoc verification: Try It → modal receives tempo=<bpm> URL param and
// the modal's #tempoVal renders the sent BPM.
import { chromium } from '@playwright/test';

const BASE = 'http://localhost:4321';

const browser = await chromium.launch({
  headless: true,
  args: ['--autoplay-policy=no-user-gesture-required'],
});
const page = await (await browser.newContext()).newPage();

const logs = [];
page.on('console', (msg) => logs.push({ type: msg.type(), text: msg.text() }));
page.on('pageerror', (err) => logs.push({ type: 'pageerror', text: err.message }));

await page.goto(BASE + '/13-drum-and-bass/', { waitUntil: 'networkidle' });

const card = page.locator('.poly-preview[data-poly-preset="Jungle Break"]');
await card.locator('.poly-preview-btn').click();

const dialog = page.locator('dialog.poly-modal');
await dialog.waitFor({ state: 'attached', timeout: 5000 });

const iframe = dialog.locator('iframe');
const src = await iframe.getAttribute('src');
console.log('IFRAME_SRC', src);

const frame = await iframe.elementHandle().then((h) => h.contentFrame());
await frame.waitForLoadState('networkidle', { timeout: 15000 });
// Poll for the ui.js script + PolyHost initialization to settle.
await page.waitForTimeout(2500);

const readout = await frame.evaluate(() => {
  const tempoEl = document.querySelector('#tempoVal');
  const presetEl = document.querySelector('#presetName');
  const state = window.PolyHost ? window.PolyHost.getState() : null;
  return {
    tempoDisplayed: tempoEl ? tempoEl.textContent : null,
    presetDisplayed: presetEl ? presetEl.textContent : null,
    stateTempo: state ? state.tempo : null,
    statePreset: state ? state.preset : null,
    hasPolyHost: !!window.PolyHost,
  };
});
console.log('READOUT', JSON.stringify(readout, null, 2));

const errs = logs.filter((l) => l.type === 'pageerror' || l.type === 'error');
console.log('ERRORS', JSON.stringify(errs, null, 2));

await browser.close();
