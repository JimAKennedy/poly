import { test, expect } from '@playwright/test';

// M052 S02 T02 — WebUI timeline-mode toggle round-trip.
//
// Before this slice, `webui/ui.js` rendered the pattern pane for a lane in
// Euclidean mode with no way to enter timeline mode. All the machinery to
// EDIT steps once in timeline mode existed (data-fixed grid click handlers,
// setFixedStep action, C++ bridge inbound), but the toggle affordance was
// missing — and the orphan `data-tl` button rendered inside the timeline
// pane had no click listener.
//
// This spec proves both wires now exist by exercising the toggle both ways
// against the mock host, which is where the UI's local `S` state model
// lives. If the toggle click flips `lane.N.timeline` correctly via
// host.edit(), the mock-host echoes the update back and the next render
// re-materializes the pattern pane in the opposite mode.
//
// Regression signal: if `ui.js:buildPanes` ever stops binding data-tl in
// either branch, the pane won't flip and this spec fails.

test.describe('WebUI timeline-mode toggle', () => {
  test('toggle round-trip: Euclidean pane ↔ timeline pane, then step click flips a hit', async ({ page }) => {
    const consoleErrors: string[] = [];
    page.on('pageerror', (err) => consoleErrors.push(String(err)));
    page.on('console', (msg) => {
      if (msg.type() === 'error') consoleErrors.push(msg.text());
    });

    await page.goto('/poly/webui/index.html');

    // Wait for the UI to boot — .strip elements are the per-lane containers
    // built by buildDesk once the host publishes its first state frame.
    await page.waitForSelector('.strip', { timeout: 10_000 });

    // Find a lane that starts in Euclidean mode (mock host's default preset
    // seeds Bell lanes in timeline mode; other lanes are Euclidean). Query
    // via the mock host's exposed state.
    const euclidLaneIndex = await page.evaluate(() => {
      const host = (window as unknown as { PolyHost?: { getState(): unknown } }).PolyHost;
      if (!host || typeof host.getState !== 'function') return -1;
      const state = host.getState() as { lanes?: Array<{ timeline?: boolean }> };
      const lanes = state?.lanes || [];
      for (let i = 0; i < lanes.length; i++) {
        if (!lanes[i].timeline) return i;
      }
      return -1;
    });

    expect(euclidLaneIndex, 'mock host should expose at least one Euclidean lane').toBeGreaterThanOrEqual(0);

    // Expand the target lane so buildPanes renders its pattern pane. The
    // expand button is the .ex element inside each strip (ui.js:544).
    const lane = page.locator('.strip').nth(euclidLaneIndex);
    await lane.locator('.ex').click();
    await expect(lane).toHaveClass(/expanded/);

    const patternPane = lane.locator('[data-pane="pattern"]');

    // Confirm we're in the Euclidean branch: Steps stepper exists,
    // fixed-pattern grid does not.
    await expect(patternPane.locator('[data-st]').first()).toBeVisible();
    await expect(patternPane.locator('[data-fixed]')).toHaveCount(0);

    // The new timeline-mode toggle should be visible and NOT "on".
    const tlToggle = patternPane.locator('[data-tl]');
    await expect(tlToggle).toBeVisible();
    await expect(tlToggle).not.toHaveClass(/\bon\b/);

    // Click the toggle — this dispatches host.edit('lane.N.timeline', 1.0)
    // via begin/perform/end. The mock host applies it to its S.lanes[N]
    // and pushes a new state frame; buildPanes re-runs and swaps the
    // pattern pane into the timeline-mode layout.
    await tlToggle.click();

    // After the flip: the pattern pane should contain the fixed-pattern
    // grid (data-fixed) instead of the Steps stepper. The re-rendered
    // toggle button carries the "on" class (ui.js:683).
    const fixedGrid = patternPane.locator('[data-fixed]');
    await expect(fixedGrid).toBeVisible({ timeout: 5_000 });
    await expect(patternPane.locator('[data-tl]')).toHaveClass(/\bon\b/);
    await expect(patternPane.locator('[data-st]')).toHaveCount(0);

    // Clicking a step button in the fixed grid should toggle its hit class.
    const step0 = fixedGrid.locator('button').nth(0);
    const wasHitBefore = (await step0.getAttribute('class') || '').includes('hit');
    await step0.click();
    if (wasHitBefore) {
      await expect(step0).not.toHaveClass(/\bhit\b/, { timeout: 5_000 });
    } else {
      await expect(step0).toHaveClass(/\bhit\b/, { timeout: 5_000 });
    }

    // Clicking the timeline toggle again returns to Euclidean mode.
    await patternPane.locator('[data-tl]').click();
    await expect(patternPane.locator('[data-fixed]')).toHaveCount(0, { timeout: 5_000 });
    await expect(patternPane.locator('[data-st]').first()).toBeVisible();
    await expect(patternPane.locator('[data-tl]')).not.toHaveClass(/\bon\b/);

    // No console errors during the entire round-trip.
    expect(consoleErrors, `console errors: ${consoleErrors.join(' | ')}`).toEqual([]);
  });
});
