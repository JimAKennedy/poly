import { test, expect } from '@playwright/test';
import {
  pageUrl,
  bootEmbedded,
  pushEmbeddedState,
  pushEmbeddedFrame,
  ringRotation,
  ringDots,
  editLaneParam,
  loomFingerprint,
} from './test-helpers.mjs';

// ---------------------------------------------------------------------------
// Drift / subdivision visualization sync (M053 S12 T04) — the slice demo proof.
//
// This is the browser-level e2e that makes the slice's two geometry claims
// regression-catchable in CI:
//   (a) Applying drift moves the circular/ring display: the ring <g> rotates per
//       frame by driftOffset/steps*360deg (engine.cpp computeDriftedCycleStep,
//       mirrored by PolyGrooveMath.driftOffset and carried on frame.lanes[].
//       driftOffset by both host pumps), and the drifted step lands the ladder
//       .now playhead on the audible step.
//   (b) Changing subdivision updates the convergence-timeline geometry (spacing/
//       width via stepLen) while — per D008 — the ring is geometrically
//       INVARIANT: its dots are cycle fractions (i/steps), which a stepLen change
//       does not move. That divergence is intentional, not a missing cue.
//
// The ring-drift assertions drive a deterministic per-frame driftOffset through
// the embedded (C++->JS) bridge, because the mock rAF pump reports t8=0 headless
// (no audio clock) and so would always compute driftOffset 0.
// ---------------------------------------------------------------------------

// Lane 1 of the default Afrobeat 12/8 preset is "Kick": a plain pattern lane
// (steps=4, stepLen=2, active) — no timeline/cells special-casing, so it is the
// clean surface for both the ring-rotation and subdivision-invariance checks.
const LANE = 1;

test.describe('drift moves the ring (M053 S12 T04)', () => {
  test('ring <g> rotates by driftOffset/steps*360 and sits at 0 under zero drift', async ({ page }) => {
    await bootEmbedded(page);
    await pushEmbeddedState(page);

    const steps = await page.evaluate(
      (li) => { const l = window.PolyMockHost.getState().lanes[li]; return l.cells ? l.cells.length : l.steps; },
      LANE,
    );
    expect(steps).toBeGreaterThan(0);

    // Zero drift -> ring parked at 0deg.
    await pushEmbeddedFrame(page, { playing: true });
    expect(await ringRotation(page, LANE)).toBe(0);

    // A one-step drift rotates the ring by exactly one cycle slice.
    await pushEmbeddedFrame(page, { playing: true, driftOffsets: { [LANE]: 1 } });
    const oneStep = await ringRotation(page, LANE);
    expect(oneStep).toBeCloseTo((1 / steps) * 360, 5);
    expect(oneStep).not.toBe(0);

    // A larger drift rotates further — the ring visibly MOVES across frames as
    // the engine's drift offset advances (the demo: drift is visible on the ring).
    await pushEmbeddedFrame(page, { playing: true, driftOffsets: { [LANE]: 2 } });
    const twoStep = await ringRotation(page, LANE);
    expect(twoStep).toBeCloseTo((2 / steps) * 360, 5);
    expect(twoStep).not.toBeCloseTo(oneStep, 5);
  });

  test('a muted lane freezes the ring at 0 even under nonzero drift', async ({ page }) => {
    // Negative case: muted reads as no motion, so drift must not rotate the ring.
    await bootEmbedded(page);
    await pushEmbeddedState(page, { muteAllLanes: true });
    await pushEmbeddedFrame(page, { playing: true, driftOffsets: { [LANE]: 2 } });
    expect(await ringRotation(page, LANE)).toBe(0);
  });

  test('the drifted playhead step lands on the ladder .now button', async ({ page }) => {
    await bootEmbedded(page);
    await pushEmbeddedState(page);
    // Frame carries the already-drifted step (step 2) for the lane; the ladder
    // .now highlight must land there — the playhead tracks the audible step.
    await pushEmbeddedFrame(page, { playing: true, steps: { [LANE]: 2 }, driftOffsets: { [LANE]: 2 } });
    const nowIndex = await page.evaluate((li) => {
      const btns = [...document.querySelectorAll(`.strip[data-lane="${li}"] .ladder button`)];
      return btns.findIndex((b) => b.classList.contains('now'));
    }, LANE);
    expect(nowIndex).toBe(2);
  });
});

test.describe('subdivision updates the timeline but leaves the ring invariant (D008)', () => {
  test('a subdivision edit changes the convergence-timeline canvas geometry', async ({ page }) => {
    await page.goto(pageUrl);
    await page.click('#mCloth');
    await expect(page.locator('#cloth')).toHaveClass(/on/);
    // Let the rAF pump paint at least one convergence frame into the canvas.
    await page.waitForFunction(() => {
      const c = document.getElementById('loom');
      return c && c.width > 0 && c.getContext('2d').getImageData(0, 0, c.width, c.height).data.some((v) => v !== 0);
    });

    const before = await loomFingerprint(page);
    // Guard against a zero/blank canvas silently passing the diff (Q5 failure path).
    expect(before.w).toBeGreaterThan(0);
    expect(before.painted).toBeGreaterThan(0);

    // Subdivision 8 -> stepLen 1 (was subdivision 4 / stepLen 2): the weave's
    // per-hit width (colW*stepLen) and cycle spacing (cyc8) both change.
    await editLaneParam(page, LANE, 'subdivision', 0.75);
    await page.evaluate(() => new Promise((r) => requestAnimationFrame(() => requestAnimationFrame(r))));

    const after = await loomFingerprint(page);
    expect(after.w).toBe(before.w);
    expect(after.hash).not.toBe(before.hash);
  });

  test('the ring dot geometry is invariant under a subdivision edit', async ({ page }) => {
    await page.goto(pageUrl); // desk mode by default
    const before = await ringDots(page, LANE);
    expect(before && before.length).toBeGreaterThan(0);

    await editLaneParam(page, LANE, 'subdivision', 0.75);
    // Desk re-rendered on the state change; capture the fresh ring.
    const after = await ringDots(page, LANE);

    // Dots are cycle fractions (i/steps); a stepLen change moves none of them.
    expect(after).toEqual(before);
  });
});
