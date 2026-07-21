// Ad-hoc verification for M043 S08 T02: Try It → modal iframe carries
// host=wasm and preset=<name>, WASM host loads the preset.
import { chromium } from '@playwright/test';

const BASE = 'http://localhost:4321';
const PAGE_URL = BASE + '/13-drum-and-bass/';

const browser = await chromium.launch({
  headless: true,
  args: ['--autoplay-policy=no-user-gesture-required'],
});
const context = await browser.newContext();
const page = await context.newPage();

const logs = [];
page.on('console', (msg) => logs.push({ type: msg.type(), text: msg.text() }));
page.on('pageerror', (err) => logs.push({ type: 'pageerror', text: err.message }));
const failedRequests = [];
page.on('requestfailed', (req) => failedRequests.push({ url: req.url(), failure: req.failure()?.errorText }));
page.on('response', (res) => { if (res.status() >= 400) failedRequests.push({ url: res.url(), status: res.status() }); });

await page.goto(PAGE_URL, { waitUntil: 'networkidle' });

const card = page.locator('.poly-preview[data-poly-preset="Jungle Break"]');
console.log('CARD_VISIBLE', await card.isVisible());

const tryBtn = card.locator('.poly-preview-btn');
console.log('TRY_BTN_TEXT', await tryBtn.textContent());

await tryBtn.click();

// Modal is a <dialog> injected on first click.
const dialog = page.locator('dialog.poly-modal');
await dialog.waitFor({ state: 'attached', timeout: 5000 });
console.log('DIALOG_OPEN_ATTR', await dialog.getAttribute('open'));

const iframe = dialog.locator('iframe');
const src = await iframe.getAttribute('src');
console.log('IFRAME_SRC', src);
console.log('IFRAME_HAS_HOST_WASM', src.includes('host=wasm'));
console.log('IFRAME_HAS_PRESET', src.includes('preset=Jungle+Break') || src.includes('preset=Jungle%20Break'));

// Give the WASM module a beat to load and apply the preset.
const frame = await iframe.elementHandle().then((h) => h.contentFrame());
await frame.waitForLoadState('networkidle', { timeout: 15000 });
await page.waitForTimeout(2000);

const probe = await frame.evaluate(() => {
  const p = window.__polyAudioProbe;
  const presetNameEl = document.querySelector('#presetName');
  const presetBtnText = presetNameEl ? presetNameEl.textContent : null;
  return {
    probeExists: !!p,
    probeCurrentPreset: p ? p.currentPreset : null,
    presetBtnText,
    hasWasmModule: typeof window.Module === 'object',
  };
});
console.log('IFRAME_PROBE', JSON.stringify(probe, null, 2));

// Screenshot the modal for the report.
await page.screenshot({ path: '/tmp/try-it-jungle-break.png', fullPage: false });
console.log('SCREENSHOT /tmp/try-it-jungle-break.png');

// --- Probe: close and try a factory preset (Latin) from a different chapter. ---
await page.keyboard.press('Escape');
await page.goto(BASE + '/06-latin/', { waitUntil: 'networkidle' });
const latinCards = await page.locator('.poly-preview').evaluateAll((els) =>
  els.map((el) => el.getAttribute('data-poly-preset')),
);
console.log('LATIN_CARDS', JSON.stringify(latinCards));

// --- Probe: verify a chapter-alias preset also flows through Try It. ---
// Reload the drum-and-bass page and try a second alias name if any exist.
await page.goto(PAGE_URL, { waitUntil: 'networkidle' });
const dnbCards = await page.locator('.poly-preview').evaluateAll((els) =>
  els.map((el) => el.getAttribute('data-poly-preset')),
);
console.log('DNB_CARDS', JSON.stringify(dnbCards));

console.log('CONSOLE_LOG_COUNT', logs.length);
const errs = logs.filter((l) => l.type === 'pageerror' || l.type === 'error' || l.type === 'warning');
console.log('ERRORS_AND_WARNINGS', JSON.stringify(errs, null, 2));
console.log('FAILED_REQUESTS', JSON.stringify(failedRequests, null, 2));
const informative = logs.filter((l) => /poly|wasm|host|preset/i.test(l.text));
console.log('INFORMATIVE_LOGS', JSON.stringify(informative, null, 2));

await browser.close();
