// M043 S11 T04 verification probe. Walks the 3 chapter pages whose cards
// changed after the alias pruning + one that still routes through an alias +
// component-demo (was Category-C blindspot). For each: locates the card,
// resolves its preset via __polyPatterns.resolvePresetName in the page context,
// and asserts the resolved display name matches expectation.
import { chromium } from '@playwright/test';

const BASE = 'http://localhost:4321/poly';

const CASES = [
  // T04 pruned aliases — card should now play the native chapter engine preset.
  {
    page: '/01-foundations/',
    cardPreset: 'Polymetric Foundation',
    expectedResolved: 'Polymetric Foundation',
  },
  {
    page: '/02-sub-saharan-africa/',
    cardPreset: 'Manding Djembe',
    expectedResolved: 'Manding Djembe',
  },
  {
    page: '/03-afro-cuban/',
    cardPreset: 'Cuban Son Montuno',
    expectedResolved: 'Cuban Son Montuno',
  },
  {
    page: '/09-electronic/',
    cardPreset: 'Minimal Techno',
    expectedResolved: 'Minimal Techno',
  },
  {
    page: '/09-electronic/',
    cardPreset: 'Deep House',
    expectedResolved: 'Deep House',
  },
  // T05 pruned aliases — manifest widened to cover their notes (47, 44/48/62/72
  // via close-pitch same-role reuse), so these now play their native preset.
  {
    page: '/02-sub-saharan-africa/',
    cardPreset: 'Ewe Polymetric Ensemble',
    expectedResolved: 'Ewe Polymetric Ensemble',
  },
  {
    page: '/08-minimalism/',
    cardPreset: 'Riley Layered Entry',
    expectedResolved: 'Riley Layered Entry',
  },
  {
    page: '/10-brazilian/',
    cardPreset: 'Samba Batucada',
    expectedResolved: 'Samba Batucada',
  },
  {
    page: '/15-compositional-grammar/',
    cardPreset: 'Compositional Arc',
    expectedResolved: 'Compositional Arc',
  },
  // Kept aliases — still awaiting cymbal / low conga / long guiro samples.
  {
    page: '/10-brazilian/',
    cardPreset: 'Bossa Nova Trio',
    expectedResolved: 'Factory: Bossa Nova',
  },
  {
    page: '/12-jazz/',
    cardPreset: 'Jazz Bop Ride',
    expectedResolved: 'Factory: Pocket Groove',
  },
  // Category-C blindspot fix — bare engine name.
  {
    page: '/component-demo/',
    cardPreset: 'Afrobeat 12/8',
    expectedResolved: 'Afrobeat 12/8',
  },
];

const browser = await chromium.launch({ headless: true });
const context = await browser.newContext();
const page = await context.newPage();

const failures = [];
const passes = [];

for (const c of CASES) {
  const url = BASE + c.page;
  await page.goto(url, { waitUntil: 'networkidle' });
  const card = page.locator(`.poly-preview[data-poly-preset="${c.cardPreset}"]`);
  const visible = await card.first().isVisible().catch(() => false);
  if (!visible) {
    failures.push({ ...c, reason: `card not visible on ${url}` });
    continue;
  }
  const resolved = await page.evaluate((name) => {
    return window.__polyPatterns?.resolvePresetName(name) ?? null;
  }, c.cardPreset);
  if (resolved !== c.expectedResolved) {
    failures.push({ ...c, reason: `resolved="${resolved}" expected="${c.expectedResolved}"` });
  } else {
    passes.push({ ...c, resolved });
  }
}

console.log('PASSES', JSON.stringify(passes, null, 2));
console.log('FAILURES', JSON.stringify(failures, null, 2));

await browser.close();
process.exit(failures.length > 0 ? 1 : 0);
