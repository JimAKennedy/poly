import { test, expect, type Page } from '@playwright/test';

// The Try it frame gives the editor its height (central-africa M006/S04,
// row CA48). The frame was min(560px, 80vh) while the editor is laid out for
// 760px, so on a laptop its lanes were squashed. At 1440x900 the frame must
// be at least 760px tall and no lane card may overflow inside it.

test.use({ viewport: { width: 1440, height: 900 } });

const CARD = '.poly-preview[data-poly-preset="Aka: Ngbolu"]';

// Problems inside the Try it iframe: a strip whose content overflows it, or a
// ladder whose top step bar sits above the strip's dial.
async function frameLayoutProblems(page: Page): Promise<string[]> {
  return page.evaluate(() => {
    const frame = document.querySelector('iframe.poly-modal-frame') as HTMLIFrameElement | null;
    const doc = frame?.contentDocument;
    if (!doc) return ['no Try it iframe'];
    return Array.from(doc.querySelectorAll('.strip')).flatMap((strip, i) => {
      const out: string[] = [];
      const el = strip as HTMLElement;
      if (el.scrollHeight > el.clientHeight + 1) out.push(`strip ${i + 1} overflows its card`);
      const ring = strip.querySelector('svg.ring');
      const top = strip.querySelector('.ladder')?.lastElementChild;
      if (ring && top && top.getBoundingClientRect().top < ring.getBoundingClientRect().bottom - 0.5)
        out.push(`strip ${i + 1}: steps spill over the dial`);
      return out;
    });
  });
}

test('M006/S04 — the Try it frame gives the editor its height at 1440x900', async ({ page }) => {
  await page.goto('/02b-central-africa/');
  await page.locator(`${CARD} .poly-preview-btn`).click();
  const frame = page.frameLocator('iframe.poly-modal-frame');
  await expect(frame.locator('#play')).toBeVisible({ timeout: 25_000 });
  await expect(frame.locator('.strip')).toHaveCount(3, { timeout: 10_000 });
  const height = await page.locator('iframe.poly-modal-frame').evaluate((f) => f.getBoundingClientRect().height);
  expect(height, 'the Try it frame is shorter than the 760px the editor is laid out for').toBeGreaterThanOrEqual(760);
  expect(await frameLayoutProblems(page)).toEqual([]);
});
