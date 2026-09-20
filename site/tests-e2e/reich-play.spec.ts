import { test, expect } from '@playwright/test';

// The scheduler must advance its lookahead past this many seconds. The value is
// a liveness floor, not a timing measurement — see #89.
const kMinLastFireTime = 2.5;

test('Reich Phase Process play button drives the AudioContext and fires >= 6 sources', async ({
  page,
}) => {
  await page.goto('/08-minimalism/');

  // Locate the PolyPreviewCard for the Reich Phase Process preset.
  const card = page.locator('.poly-preview[data-poly-preset="Reich Phase Process"]');
  await expect(card).toBeVisible();
  await expect(card).toHaveAttribute('data-state', 'stopped');

  const playBtn = card.locator('.poly-preview-play');
  await expect(playBtn).toBeEnabled();
  await playBtn.click();

  // Card should flip to playing.
  await expect(card).toHaveAttribute('data-state', 'playing');

  // Let the pattern run. Poll for the scheduler reaching the threshold rather
  // than sleeping a fixed 3s and asserting afterwards: the old form left only
  // 0.5s of headroom above the 2.5s threshold, so ordinary startup jitter on a
  // loaded machine failed it with lastFireTime ~2.47 while the scheduler was
  // working perfectly (#89). Polling keeps the same claim — the scheduler
  // advances past kMinLastFireTime — but stops treating "slower than usual" as
  // "stalled". A genuinely stalled scheduler never reaches it and still fails,
  // on the timeout.
  await page
    .waitForFunction(
      (threshold) => {
        const probe = (window as any).__polyAudioProbe;
        const times: number[] = probe?.scheduledNoteTimes ?? [];
        return times.length > 0 && times[times.length - 1] > threshold;
      },
      kMinLastFireTime,
      { timeout: 15_000 },
    )
    .catch(() => {
      // Swallow the timeout so the assertions below report *what* the probe
      // held rather than an opaque Playwright timeout with no numbers in it.
    });

  const probe = await page.evaluate(() => {
    const p = (window as any).__polyAudioProbe;
    if (!p) return null;
    const times: number[] = p.scheduledNoteTimes ?? [];
    return {
      lastFireTime: times.length ? times[times.length - 1] : 0,
      scheduledNoteCount: times.length,
      fallbackActive: !!p.fallbackActive,
      lastError: p.lastError ?? null,
    };
  });

  expect(probe, 'window.__polyAudioProbe was never set').not.toBeNull();
  expect(
    probe!.fallbackActive,
    `Reich fell into fallback: ${probe!.lastError ?? '(no error message)'}`,
  ).toBe(false);
  expect(
    probe!.lastFireTime,
    `latest scheduled fireTime = ${probe!.lastFireTime}, expected > ${kMinLastFireTime} (scheduler stalled?)`,
  ).toBeGreaterThan(kMinLastFireTime);
  expect(
    probe!.scheduledNoteCount,
    `scheduledNoteCount = ${probe!.scheduledNoteCount}, expected >= 6`,
  ).toBeGreaterThanOrEqual(6);

  // Second click stops.
  await playBtn.click();
  await expect(card).toHaveAttribute('data-state', 'stopped');
});

test('chapter alias card ("Jungle Break") resolves to a factory pattern and enables Play', async ({
  page,
}) => {
  // Jungle Break is a chapter-only preset name — resolved via CHAPTER_ALIASES
  // to Factory: Breakbeat. If the alias map is missing an entry, the Play
  // button stays disabled with "Audio preview coming soon".
  await page.goto('/13-drum-and-bass/');
  const card = page.locator('.poly-preview[data-poly-preset="Jungle Break"]');
  await expect(card).toBeVisible();
  const playBtn = card.locator('.poly-preview-play');
  await expect(playBtn).toBeEnabled();
  await expect(playBtn).not.toHaveAttribute('title', 'Audio preview coming soon');
});
