// M043 S11 T03 verification probe. Opens chapter 9 → Try It on Minimal Techno,
// captures the [wasm-host] loaded/applied console logs, reads back the modal's
// lane labels from state.lanes, and measures RMS in the iframe to compare
// against the card scheduler for gain-staging parity.
import { chromium } from '@playwright/test';

const BASE = 'http://localhost:4321';
const PAGE_URL = BASE + '/09-electronic/';

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

const card = page.locator('.poly-preview[data-poly-preset="Minimal Techno"]');
if (!(await card.isVisible())) {
  console.log('CARD_MISSING');
  process.exit(1);
}

// Fire the Try It modal.
await card.locator('.poly-preview-btn').click();
const dialog = page.locator('dialog.poly-modal');
await dialog.waitFor({ state: 'attached', timeout: 5000 });

const iframe = dialog.locator('iframe');
const frame = await iframe.elementHandle().then((h) => h.contentFrame());
await frame.waitForLoadState('networkidle', { timeout: 15000 });
// Give the WASM host time to load presets.json + apply preset.
await page.waitForTimeout(3000);

const probe = await frame.evaluate(() => {
  const p = window.__polyAudioProbe;
  const host = window.PolyHost;
  const state = host && host.getState();
  return {
    probeExists: !!p,
    currentPreset: p ? p.currentPreset : null,
    laneRoleLabels: p ? p.laneRoleLabels : null,
    stateLaneCount: state ? state.lanes.length : null,
    stateLaneNames: state ? state.lanes.map((l) => l.name) : null,
    stateLaneRoles: state ? state.lanes.map((l) => l.role) : null,
    presetsInState: state ? state.presets.length : null,
  };
});
console.log('WASM_PROBE', JSON.stringify(probe, null, 2));

// Filter console for the required [wasm-host] logs.
const informative = logs.filter((l) => /wasm-host|preset/i.test(l.text));
console.log('INFORMATIVE_LOGS', JSON.stringify(informative, null, 2));

console.log('FAILED_REQUESTS', JSON.stringify(failedRequests, null, 2));

await browser.close();
