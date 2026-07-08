// M043 S11 T04/T05/T06 verification probe. Walks the chapter pages whose cards
// changed across the alias pruning + manifest widening. After T06 every
// chapter preset resolves natively — the "aliased" path no longer exists.
// This probe still asserts native resolution for the presets that were once
// aliased so a future regression (someone re-adds an alias, or a preset gets
// renamed) fails loudly.
import { chromium } from '@playwright/test';

const BASE = 'http://localhost:4321/poly';

const CASES = [
  // T04 pruned aliases — card plays its native chapter engine preset.
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
  // T05 pruned aliases — manifest widened to cover 44/47/48/62/72 via
  // close-pitch same-role reuse.
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
  // T06 pruned aliases — cymbal (51 ride, 55 china-as-splash-fallback), low
  // conga (64), and long guiro (74) now shipped in the manifest.
  {
    page: '/10-brazilian/',
    cardPreset: 'Bossa Nova Trio',
    expectedResolved: 'Bossa Nova Trio',
  },
  {
    page: '/12-jazz/',
    cardPreset: 'Jazz Bop Ride',
    expectedResolved: 'Jazz Bop Ride',
  },
  {
    page: '/12-jazz/',
    cardPreset: 'Elvin Jones Cascade',
    expectedResolved: 'Elvin Jones Cascade',
  },
  {
    page: '/13-drum-and-bass/',
    cardPreset: 'Liquid Drum and Bass',
    expectedResolved: 'Liquid Drum and Bass',
  },
  {
    page: '/05-gamelan/',
    cardPreset: 'Balinese Kotekan',
    expectedResolved: 'Balinese Kotekan',
  },
  {
    page: '/05-gamelan/',
    cardPreset: 'Javanese Colotomic',
    expectedResolved: 'Javanese Colotomic',
  },
  {
    page: '/14-synthesis/',
    cardPreset: 'Afro-Electronic Fusion',
    expectedResolved: 'Afro-Electronic Fusion',
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
