// Regenerates the guide's plugin-interface screenshot from the mock-host web UI.
//
// first-release M004/S01, FR21. The picture the guide carried showed a
// CLOTH/DESK chip pair and a LEARN button — controls M001 removed — and none
// of the capture controls or Export the prose describes. Its BPM (126.0) and
// seed (88) are this mock host's defaults for Afrobeat 12/8, so the same source
// regenerates it, and this script refuses to write a picture of a UI that does
// not ship: it asserts the single-view state before it captures, on the same
// ids site/tests/webui-single-view.test.mjs forbids in the markup.
//
// A script rather than a spec on purpose: `playwright test` would run a spec
// on every CI run and rewrite a tracked binary each time. Run when the UI
// changes:
//
//   node webui/tests/capture-guide-screenshots.mjs
//
// Output: site/public/screenshots/ui-overview.png, 1280 px wide (the webui
// Playwright config's own viewport; at 1100 px a two-line role label pushes a
// lane name out of its head).
//
// The mock host hides Export unless opened with `?export=1` (it has no SMF
// path of its own); the plugin always shows it, so the capture opens with the
// seam on and requires the chip visible, as the guide's walkthrough names it.

import { chromium } from '@playwright/test';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { pageUrl } from './test-helpers.mjs';

const HERE = path.dirname(fileURLToPath(import.meta.url));
const OUT = path.resolve(HERE, '..', '..', 'site', 'public', 'screenshots', 'ui-overview.png');
const VIEWPORT = { width: 1280, height: 870 };

// Controls that left with the Cloth view. Any one present means the page is
// not the UI that ships, and the capture must not proceed.
const FORBIDDEN = ['modes', 'mCloth', 'mDesk', 'cloth', 'loom', 'learnBtn'];
// Controls the guide's header walkthrough names. Any one missing means the
// picture would not match the prose.
const REQUIRED = ['presetName', 'scA', 'scB', 'scM', 'chainBtn', 'noteMapBtn', 'capCtl', 'exportBtn', 'desk'];

const browser = await chromium.launch();
try {
  const page = await browser.newPage({ viewport: VIEWPORT });
  await page.goto(`${pageUrl}?export=1`);

  await page.evaluate(() => {
    const s = window.PolyMockHost.getState();
    s.timeSigNumerator = 12;
    s.timeSigDenominator = 8;
    window.PolyMockHost._pushState();
  });
  await page.locator('#timeSigVal').filter({ hasText: '12/8' }).waitFor();
  await page.locator('#desk.on').waitFor();
  await page.locator('.strip[data-lane]').nth(4).waitFor();

  const present = await page.evaluate(
    (ids) => ids.filter((id) => document.getElementById(id) !== null),
    FORBIDDEN,
  );
  if (present.length > 0) {
    throw new Error(
      `refusing to capture: the page carries ${present.map((id) => `#${id}`).join(', ')}, ` +
        'which the shipped UI does not have',
    );
  }
  const missing = await page.evaluate(
    (ids) => ids.filter((id) => document.getElementById(id) === null),
    REQUIRED,
  );
  if (missing.length > 0) {
    throw new Error(
      `refusing to capture: the page lacks ${missing.map((id) => `#${id}`).join(', ')}, ` +
        'which the guide names',
    );
  }

  if (!(await page.locator('#exportBtn').isVisible())) {
    throw new Error('refusing to capture: #exportBtn is hidden, so the header would not match the walkthrough');
  }

  await page.screenshot({ path: OUT, fullPage: true });
  const size = await page.evaluate(() => ({
    w: document.documentElement.scrollWidth,
    h: document.documentElement.scrollHeight,
  }));
  console.log(`wrote ${path.relative(process.cwd(), OUT)} (${size.w}×${size.h})`);
} finally {
  await browser.close();
}
