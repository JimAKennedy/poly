import { test, expect } from '@playwright/test';
import * as fs from 'node:fs';
import * as path from 'node:path';

// S18 T03 — Chapter-wide console.error + pageerror smoke gate.
//
// Walks every chapter route (every .mdx under src/content/docs/) headlessly,
// subscribes to pageerror and console.error BEFORE navigation, waits for the
// page to settle, scrolls to bottom to trigger any lazy-hydrated Astro islands
// (PolyScreenshot, EuclideanDiagram, PolyPreviewCard), waits for hydration,
// then asserts both event arrays are empty (after allowlist filtering).
//
// The equivalence / audio-gate / preset-consistency specs cover the audio
// path; they don't fail on rendering regressions that produce a broken visual
// island but no audible symptom. This gate closes that gap.
//
// The allowlist is deliberately empty — we grow it only when a benign warning
// proves stable across CI + local. Every allowlisted match is logged in the
// summary JSON so drift is visible.
//
// Summary persistence: per-slug JSON records are written to PER_DIR during
// each test (BEFORE the assertion) so failing tests still record what was
// observed. afterAll aggregates from disk — worker-restart-safe, mirrors the
// pattern in equivalence.spec.ts.

const DOCS_DIR = path.resolve(process.cwd(), 'src/content/docs');
const OUT_DIR = path.resolve(process.cwd(), 'test-results');
const PER_DIR = path.join(OUT_DIR, 'console-error-per-chapter');

interface ChapterRoute {
  slug: string;   // filename minus .mdx
  route: string;  // page path under baseURL — Astro uses /poly/<slug>/ (or /poly/ for index)
}

interface AllowlistedMatch {
  source: 'console' | 'pageerror';
  message: string;
  matchedRule: string;
}

interface ChapterRecord {
  slug: string;
  route: string;
  allowlisted: AllowlistedMatch[];
  errors: { source: 'console' | 'pageerror'; message: string }[];
}

// Exact-match allowlist. Add entries only when a benign warning proves
// stable — every entry is drift-tracked in the summary JSON.
const ALLOWLIST: readonly string[] = [];

function discoverChapters(): ChapterRoute[] {
  const files = fs
    .readdirSync(DOCS_DIR)
    .filter((f) => f.endsWith('.mdx'))
    .sort();
  return files.map((file) => {
    const slug = file.slice(0, -'.mdx'.length);
    const route = slug === 'index' ? '/poly/' : `/poly/${slug}/`;
    return { slug, route };
  });
}

const CHAPTERS: readonly ChapterRoute[] = discoverChapters();

// PER_DIR is NOT cleaned in beforeAll: Playwright restarts the worker on
// test failure, which would re-run beforeAll and wipe all records the prior
// worker wrote. Instead, every test overwrites its own per-slug file, so a
// full run's writes dominate. Stale records from a removed chapter are the
// only leak surface — rare enough to be acceptable and easy to spot in the
// summary (the drift-tracking is the point). Matches equivalence.spec.ts.
fs.mkdirSync(PER_DIR, { recursive: true });

test.afterAll(() => {
  fs.mkdirSync(OUT_DIR, { recursive: true });
  const chapters: ChapterRecord[] = [];
  if (fs.existsSync(PER_DIR)) {
    for (const file of fs.readdirSync(PER_DIR).sort()) {
      if (!file.endsWith('.json')) continue;
      chapters.push(JSON.parse(fs.readFileSync(path.join(PER_DIR, file), 'utf8')));
    }
  }
  const allowlistedMatches = chapters.flatMap((c) =>
    c.allowlisted.map((a) => ({ route: c.route, ...a })),
  );
  const errors = chapters.flatMap((c) =>
    c.errors.map((e) => ({ route: c.route, ...e })),
  );
  const summary = {
    gate: 'S18-console-error',
    chaptersWalked: chapters.map((c) => c.slug),
    allowlistedMatches,
    errors,
  };
  const outPath = path.join(OUT_DIR, 'console-error-summary.json');
  fs.writeFileSync(outPath, JSON.stringify(summary, null, 2));
  // eslint-disable-next-line no-console
  console.log(
    `[console-error] wrote ${outPath} — walked ${summary.chaptersWalked.length}, allowlisted ${summary.allowlistedMatches.length}, errors ${summary.errors.length}`,
  );
});

test.describe('S18 T03 — chapter console/pageerror smoke', () => {
  for (const chapter of CHAPTERS) {
    test(`[${chapter.slug}] emits no console.error or pageerror`, async ({ page }) => {
      const record: ChapterRecord = {
        slug: chapter.slug,
        route: chapter.route,
        allowlisted: [],
        errors: [],
      };

      const classify = (
        source: 'console' | 'pageerror',
        message: string,
      ): 'allowed' | 'error' => {
        for (const rule of ALLOWLIST) {
          if (message === rule) {
            record.allowlisted.push({ source, message, matchedRule: rule });
            return 'allowed';
          }
        }
        return 'error';
      };

      const consoleErrors: string[] = [];
      const pageErrors: string[] = [];

      page.on('pageerror', (err) => {
        const msg = err.stack ?? err.message ?? String(err);
        if (classify('pageerror', msg) === 'error') {
          pageErrors.push(msg);
          record.errors.push({ source: 'pageerror', message: msg });
        }
      });

      page.on('console', (msg) => {
        if (msg.type() !== 'error') return;
        const text = msg.text();
        if (classify('console', text) === 'error') {
          consoleErrors.push(text);
          record.errors.push({ source: 'console', message: text });
        }
      });

      const response = await page.goto(chapter.route, { waitUntil: 'networkidle' });
      expect(response, `${chapter.route}: page.goto returned no response`).not.toBeNull();
      expect(
        response!.ok(),
        `${chapter.route}: HTTP ${response!.status()} — route missing from deploy?`,
      ).toBe(true);

      // Trigger lazy-hydrated Astro islands (PolyScreenshot, EuclideanDiagram,
      // PolyPreviewCard) by scrolling to the bottom. Then scroll back and let
      // any late hydration settle before harvesting errors.
      await page.evaluate(() => window.scrollTo(0, document.body.scrollHeight));
      await page.waitForTimeout(500);
      await page.evaluate(() => window.scrollTo(0, 0));
      await page.waitForTimeout(200);

      // Persist the record BEFORE assertions so afterAll's aggregation
      // captures errors even on failing tests. If the assertion below throws,
      // the JSON is already on disk.
      fs.writeFileSync(
        path.join(PER_DIR, `${chapter.slug}.json`),
        JSON.stringify(record, null, 2),
      );

      expect(
        pageErrors,
        `chapter ${chapter.route} emitted ${pageErrors.length} pageerror(s): ${pageErrors.join(' | ')}`,
      ).toEqual([]);
      expect(
        consoleErrors,
        `chapter ${chapter.route} emitted ${consoleErrors.length} console.error(s): ${consoleErrors.join(' | ')}`,
      ).toEqual([]);
    });
  }
});
