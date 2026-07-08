// Ad-hoc diagnostic: click Play, then toggle each lane chip and confirm the
// scheduler's mutedLanes set + nodesStarted respond as expected.
import { chromium } from '@playwright/test';

const URL = 'http://localhost:4321/poly/08-minimalism/';

const browser = await chromium.launch({ headless: true });
const context = await browser.newContext();
const page = await context.newPage();

const logs = [];
page.on('console', (msg) => logs.push({ type: msg.type(), text: msg.text() }));
page.on('pageerror', (err) => logs.push({ type: 'pageerror', text: err.message }));

await page.goto(URL, { waitUntil: 'networkidle' });

const card = page.locator('[data-poly-preset="Reich Phase Process"]');
const chips = card.locator('.poly-lane-chip');
const chipCount = await chips.count();
console.log('CHIP_COUNT', chipCount);
console.log('CHIP_LABELS', JSON.stringify(
  await chips.evaluateAll((els) =>
    els.map((el) => ({
      label: el.querySelector('.poly-lane-chip-label')?.textContent,
      role: el.querySelector('.poly-lane-chip-role')?.textContent,
      pressed: el.getAttribute('aria-pressed'),
    })),
  ),
  null,
  2,
));

await card.locator('.poly-preview-play').click();
await page.waitForTimeout(1500);

const before = await page.evaluate(() => {
  const p = window.__polyAudioProbe;
  return { nodes: p.scheduler.nodesStarted, muted: [...p.scheduler.mutedLanes] };
});
console.log('BEFORE_MUTE', JSON.stringify(before));

// Mute lane 1 (Drifting pulse), let it run, then read again.
await chips.nth(1).click();
await page.waitForTimeout(1500);
const afterMute = await page.evaluate(() => {
  const p = window.__polyAudioProbe;
  return { nodes: p.scheduler.nodesStarted, muted: [...p.scheduler.mutedLanes] };
});
console.log('AFTER_MUTE_LANE1', JSON.stringify(afterMute));

// Unmute lane 1, run again.
await chips.nth(1).click();
await page.waitForTimeout(1500);
const afterUnmute = await page.evaluate(() => {
  const p = window.__polyAudioProbe;
  return { nodes: p.scheduler.nodesStarted, muted: [...p.scheduler.mutedLanes] };
});
console.log('AFTER_UNMUTE_LANE1', JSON.stringify(afterUnmute));

console.log('CONSOLE', JSON.stringify(logs, null, 2));
await browser.close();
