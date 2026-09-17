import { test, expect, chromium } from '@playwright/test';
import type { Browser, Page } from '@playwright/test';

import {
  applyMutation,
  comparePresets,
  loadPresetExpectations,
  type ObservedPreset,
} from './lib/preset-contract';

// M004 S02 (DAW02) — select every factory preset inside Cubase.
//
// 31 of the 45 presets have never been opened in a DAW. kWebPresetLaneNames was
// once initialised sparsely — 14 rows against a kFactoryPresetCount-sized
// extent — and a null entry there crashed Cubase on preset change. The null
// guard at the applyPreset call site is the only thing between that table and
// the same crash, and nothing exercises it index by index.
//
// The sweep therefore asserts two different things. That the host survives each
// selection is the crash check, and the index being selected is the only useful
// thing to know when it does not. That each preset LOADS CORRECTLY is the
// second: a preset that loads wrongly would otherwise pass a survival-only
// sweep, which is the difference between "did not crash" and "works".
//
// Runner-gated and Windows-only, like toggle-step.spec.ts: WebView2 honors the
// CDP remote-debugging port and WKWebView does not. On a dev machine this file
// only needs to typecheck — the attach needs Cubase launched with -EnableCdp.

const CDP_ENDPOINT =
  process.env.POLY_CDP_ENDPOINT ||
  `http://127.0.0.1:${process.env.POLY_CDP_PORT || '9222'}`;
const ATTACH_TIMEOUT_MS = Number(process.env.POLY_ATTACH_TIMEOUT_MS || 60_000);
// Each selection re-renders every lane strip. Generous, because a slow render
// misread as a wrong lane count would be a false failure on a run that costs a
// Cubase launch.
const SETTLE_MS = Number(process.env.POLY_PRESET_SETTLE_MS || 250);

async function connectWithRetry(): Promise<Browser> {
  const deadline = Date.now() + ATTACH_TIMEOUT_MS;
  let lastError: unknown;
  while (Date.now() < deadline) {
    try {
      return await chromium.connectOverCDP(CDP_ENDPOINT);
    } catch (err) {
      lastError = err;
      await new Promise((r) => setTimeout(r, 1000));
    }
  }
  throw new Error(
    `Could not reach CDP at ${CDP_ENDPOINT} within ${ATTACH_TIMEOUT_MS}ms. ` +
      `Likely cause: Cubase was not launched with -EnableCdp. Last error: ${String(lastError)}`,
  );
}

/** The Poly editor is the WebView2 target rendering `.strip` lane containers. */
async function findPolyEditor(browser: Browser): Promise<Page> {
  const deadline = Date.now() + ATTACH_TIMEOUT_MS;
  let seenTargets = 0;
  while (Date.now() < deadline) {
    for (const context of browser.contexts()) {
      for (const page of context.pages()) {
        seenTargets++;
        const isPoly = await page
          .locator('.strip')
          .first()
          .isVisible()
          .catch(() => false);
        if (isPoly) return page;
      }
    }
    await new Promise((r) => setTimeout(r, 500));
  }
  throw new Error(
    `No Poly editor page found among ${seenTargets} WebView2 target(s) within ` +
      `${ATTACH_TIMEOUT_MS}ms.`,
  );
}

/**
 * Read back what the editor is showing: one lane per `.strip`, each carrying a
 * `.stat` line that ends `N<note>` (webui/ui.js). Reading the shipping DOM is
 * deliberate — there is no debug state dump in the embedded host, and adding
 * one for a test would put test-only code in the product.
 */
async function readLanes(page: Page): Promise<{ laneCount: number; noteNumbers: number[] }> {
  const stats = await page.locator('.strip .stat').allTextContents();
  const noteNumbers = stats.map((text) => {
    const m = /N(\d+)/.exec(text);
    return m ? Number(m[1]) : NaN;
  });
  return { laneCount: stats.length, noteNumbers };
}

test.describe('L4-web: every factory preset loads inside Cubase', () => {
  test.describe.configure({ mode: 'serial' });
  // 45 selections, each with a settle and a DOM read.
  test.setTimeout(Number(process.env.POLY_PRESET_TIMEOUT_MS || 600_000));

  test('select all 45 presets and compare each against presets.json', async () => {
    const expected = applyMutation(loadPresetExpectations(), process.env.POLY_E2E_MUTATE);
    expect(expected.length).toBeGreaterThan(0);

    const browser = await connectWithRetry();
    try {
      const page = await findPolyEditor(browser);

      // Show every preset regardless of the category chip's current state: the
      // menu filters by category, and a filtered menu would silently skip
      // indices rather than fail.
      await page.locator('#presetTrigger').click();
      const allChip = page.locator('#presetMenu .preset-chip[data-category="All"]');
      if (await allChip.count()) await allChip.click();

      const observed: ObservedPreset[] = [];
      for (const want of expected) {
        const option = page.locator(`#presetMenu [role="option"][data-index="${want.index}"]`);
        await expect(
          option,
          `preset ${want.index} (${want.name}) has no option in the menu`,
        ).toHaveCount(1);

        try {
          await option.click();
          await page.waitForTimeout(SETTLE_MS);
          observed.push({ index: want.index, ...(await readLanes(page)) });
        } catch (err) {
          // The host dying mid-sweep is the crash this slice exists for, and
          // the index is the only useful thing to know about it.
          throw new Error(
            `the editor stopped responding while selecting preset ${want.index} ` +
              `(${want.name}) — this is the kWebPresetLaneNames failure mode. ` +
              `Underlying error: ${String(err)}`,
          );
        }

        // Reopen for the next selection: choosing an option closes the menu.
        await page.locator('#presetTrigger').click();
      }

      const problems = comparePresets(expected, observed);
      expect(
        problems,
        `${problems.length} preset(s) did not match presets.json:\n  ${problems.join('\n  ')}`,
      ).toEqual([]);
    } finally {
      await browser.close();
    }
  });
});
