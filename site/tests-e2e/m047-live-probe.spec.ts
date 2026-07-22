import { test, expect } from '@playwright/test';
import { writeFileSync, mkdirSync, existsSync, readFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const OUT_PATH = resolve(HERE, '..', '..', '.gsd', 'artifacts', 'M047-live-probe.json');

// M047 milestone-validation live probe. Runs against the deployed
// poly.jk.digital host directly (not baseURL) so it produces the
// browser-observable evidence the milestone-validation gate needs
// before gsd_complete_milestone will accept a pass verdict.

const HOST = 'https://poly.jk.digital';
const OLD_HOST = 'jimakennedy.github.io';

interface CheckRec {
  url: string;
  checks: Record<string, { value?: unknown; pass: boolean; note?: string }>;
  pass: boolean;
}
const evidence: { host: string; timestamp: string; pages: CheckRec[]; allPass: boolean } = {
  host: HOST,
  timestamp: '2026-07-22',
  pages: [],
  allPass: true,
};

const PAGES = [
  '/',
  '/03-afro-cuban/',
  '/08-minimalism/',
  '/07-balkan/',
  '/appendix-euclidean-reference/',
];

for (const path of PAGES) {
  test(`M047 live probe: ${path}`, async ({ page }) => {
    const url = HOST + path;
    const rec: CheckRec = { url, checks: {}, pass: false };

    const resp = await page.goto(url, { waitUntil: 'domcontentloaded', timeout: 30_000 });
    rec.checks.httpStatus = { value: resp?.status(), pass: resp?.status() === 200 };

    const canonical = await page.locator('link[rel="canonical"]').getAttribute('href');
    rec.checks.canonicalHost = { value: canonical, pass: !!canonical && canonical.startsWith(HOST) };
    rec.checks.canonicalNoOldHost = {
      value: canonical,
      pass: !!canonical && !canonical.includes(OLD_HOST),
    };

    const ogImage = await page.locator('meta[property="og:image"]').getAttribute('content');
    rec.checks.ogImageHost = { value: ogImage, pass: !!ogImage && ogImage.startsWith(HOST) };

    const twImage = await page.locator('meta[name="twitter:image"]').getAttribute('content');
    rec.checks.twitterImageHost = { value: twImage, pass: !!twImage && twImage.startsWith(HOST) };

    const headHtml = await page.locator('head').innerHTML();
    rec.checks.noOldHostInHead = {
      value: headHtml.includes(OLD_HOST) ? 'PRESENT' : 'absent',
      pass: !headHtml.includes(OLD_HOST),
    };

    if (path === '/03-afro-cuban/') {
      const body = await page.locator('body').innerText();
      rec.checks.sonClaveNotE516 = {
        pass: !/son clave.*is exactly.*E\(5,16\)/i.test(body),
        note: 'D1 — must not claim son clave = E(5,16)',
      };
    }
    if (path === '/08-minimalism/') {
      const body = await page.locator('body').innerText();
      // Source uses the space-separated 12-onset visualization; corrected pattern.
      rec.checks.clappingPatternCorrect = {
        pass: body.includes('x x x . x x . x . x x .'),
        note: 'D3 — Reich Clapping Music pattern rendered (12-pulse form)',
      };
      // The page *does* mention E(8,12) in order to *reject* the claim — check
      // that any mention is negated ("not E(8,12)"), which is the corrected
      // framing S02 landed. The pre-S02 text asserted the equality.
      rec.checks.e812IsNegated = {
        pass: /not\s+E\(8,\s*12\)/i.test(body),
        note: 'D3 — E(8,12) mention must be explicitly rejected, not asserted',
      };
    }
    if (path === '/') {
      const body = await page.locator('body').innerText();
      rec.checks.no26Patches = {
        pass: !/26 patches/i.test(body),
        note: 'D6 — stale "26 patches" text removed',
      };
    }

    rec.pass = Object.values(rec.checks).every((c) => c.pass);

    // Write incrementally so partial results survive if a later test crashes.
    let doc = evidence;
    if (existsSync(OUT_PATH)) {
      try { doc = JSON.parse(readFileSync(OUT_PATH, 'utf8')); } catch { doc = evidence; }
    }
    doc.pages = (doc.pages || []).filter((p: CheckRec) => p.url !== url);
    doc.pages.push(rec);
    doc.allPass = doc.pages.every((p: CheckRec) => p.pass);
    mkdirSync(dirname(OUT_PATH), { recursive: true });
    writeFileSync(OUT_PATH, JSON.stringify(doc, null, 2));

    // Assert per page so the test fails if any check fails.
    for (const [name, check] of Object.entries(rec.checks)) {
      expect(check.pass, `${path} :: ${name} :: ${JSON.stringify(check.value)}`).toBe(true);
    }
  });
}
