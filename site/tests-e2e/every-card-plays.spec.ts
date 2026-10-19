import { test, expect } from '@playwright/test';
import * as fs from 'node:fs';
import * as path from 'node:path';

// Every card plays (central-africa M006/S06, row CA50).
//
// The site's audio gate plays a sample of cards, so a preset added to a
// chapter was not tested in Try it until a reader found it broken. This spec
// reads every PolyPreviewCard from the built site, opens its Try it at a
// laptop viewport, plays it, and fails if no note starts, a lane card
// overflows, or the page or the editor logs an error. The list is derived
// from the pages, never written out, so a new card is covered as it lands.

test.use({ viewport: { width: 1440, height: 900 } });

const DIST = path.resolve(process.cwd(), 'dist');
// Cards on the built site on 2026-10-18: 38 in the chapters, 39 in all. A
// floor, so a broken extraction cannot pass by finding nothing.
const MIN_CARDS = 38;

interface Card {
  page: string;
  preset: string;
}

function findCards(dir: string): Card[] {
  const out: Card[] = [];
  const walk = (d: string) => {
    for (const entry of fs.readdirSync(d, { withFileTypes: true })) {
      const p = path.join(d, entry.name);
      if (entry.isDirectory()) walk(p);
      else if (entry.name === 'index.html') {
        const html = fs.readFileSync(p, 'utf8');
        const route = '/' + path.relative(DIST, path.dirname(p)).split(path.sep).join('/');
        const page = route === '/' ? '/' : `${route}/`;
        for (const m of html.matchAll(/class="poly-preview [^"]*" data-poly-preset="([^"]+)"/g)) {
          out.push({ page, preset: m[1].replace(/&amp;/g, '&').replace(/&#39;/g, "'") });
        }
      }
    }
  };
  if (fs.existsSync(dir)) walk(dir);
  return out;
}

const CARDS = findCards(DIST);

test('M006/S06 — the built site carries every preview card', () => {
  expect(fs.existsSync(DIST), `no built site at ${DIST}; build it before this spec`).toBe(true);
  expect(CARDS.length, 'fewer preview cards than the site carries; the extraction is broken').toBeGreaterThanOrEqual(
    MIN_CARDS,
  );
});

for (const card of CARDS) {
  test(`M006/S06 — ${card.preset} plays in Try it (${card.page})`, async ({ page }) => {
    const errors: string[] = [];
    page.on('pageerror', (e) => errors.push(`pageerror: ${e.message}`));
    page.on('console', (m) => {
      if (m.type() === 'error') errors.push(`console: ${m.text()}`);
    });

    await page.goto(card.page);
    const selector = `.poly-preview[data-poly-preset="${card.preset.replace(/"/g, '\\"')}"]`;
    await page.locator(`${selector} .poly-preview-btn`).first().click();
    const frame = page.frameLocator('iframe.poly-modal-frame');
    await expect(frame.locator('#play'), `${card.preset}: Try it did not open`).toBeVisible({ timeout: 25_000 });
    await frame.locator('#play').click();

    const started = await page
      .waitForFunction(
        () => {
          const f = document.querySelector('iframe.poly-modal-frame') as HTMLIFrameElement | null;
          const probe = (f?.contentWindow as any)?.__polyAudioProbe;
          return probe && probe.nodesStarted > 0 ? probe.nodesStarted : false;
        },
        null,
        { timeout: 5000, polling: 100 },
      )
      .then(() => true)
      .catch(() => false);
    expect(started, `${card.preset} on ${card.page}: no note started within 5 s of Play`).toBe(true);

    const overflow = await page.evaluate(() => {
      const f = document.querySelector('iframe.poly-modal-frame') as HTMLIFrameElement | null;
      const doc = f?.contentDocument;
      if (!doc) return ['no Try it iframe'];
      return Array.from(doc.querySelectorAll('.strip')).flatMap((strip, i) => {
        const el = strip as HTMLElement;
        const out: string[] = [];
        if (el.scrollHeight > el.clientHeight + 1) out.push(`strip ${i + 1} overflows its card`);
        const ring = strip.querySelector('svg.ring');
        const top = strip.querySelector('.ladder')?.lastElementChild;
        if (ring && top && top.getBoundingClientRect().top < ring.getBoundingClientRect().bottom - 0.5)
          out.push(`strip ${i + 1}: steps spill over the dial`);
        return out;
      });
    });
    expect(overflow, `${card.preset} on ${card.page}: a lane card overflows`).toEqual([]);

    // The editor's own console errors reach the page listener only through its
    // frame, so read them there too.
    for (const f of page.frames()) {
      if (f === page.mainFrame()) continue;
      const frameErrors = await f
        .evaluate(() => (window as unknown as { __polyAudioProbe?: { lastError?: string | null } }).__polyAudioProbe?.lastError ?? null)
        .catch(() => null);
      if (frameErrors) errors.push(`editor: ${frameErrors}`);
    }
    expect(errors, `${card.preset} on ${card.page}: errors while playing`).toEqual([]);
  });
}
