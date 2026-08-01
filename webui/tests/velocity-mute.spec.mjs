import { test, expect } from '@playwright/test';
import { pageUrl, editLaneParam } from './test-helpers.mjs';

// Count pixels on the loom canvas that are NOT one of the two solid band
// background fills (drawConvergence paints '#222E52' / '#26335A' behind every
// lane). Hit bars are drawn in the lane's bright hue over that background, so a
// muted lane's removed hit bars show up as a drop in this non-background count.
// (loomFingerprint.painted counts all opaque pixels — useless here because the
// backgrounds cover the whole canvas regardless of hits.)
async function loomHitPixels(page) {
  return page.evaluate(() => {
    const c = document.getElementById('loom');
    const g = c.getContext('2d');
    const d = g.getImageData(0, 0, c.width, c.height).data;
    // rgb of the two band backgrounds.
    const bg = [[34, 46, 82], [38, 51, 90]];
    let n = 0;
    for (let i = 0; i < d.length; i += 4) {
      const r = d[i], gg = d[i + 1], b = d[i + 2];
      const isBg = bg.some(([br, bgc, bb]) => r === br && gg === bgc && b === bb);
      if (!isBg && d[i + 3] !== 0) n++;
    }
    return n;
  });
}

// ---------------------------------------------------------------------------
// Velocity-zero mutes the lane in the WebUI decorative preview (M073 S01 T02).
//
// The site preview audio path (_poly_render) inherits the engine's classifyStep
// Silent short-circuit from T01 automatically. The WebUI weave viz is a separate
// JS mirror driven by PolyGrooveMath.hitVelocity — so it must be muted here too.
//
// Contract (slice Proof Level):
//   - hitVelocity(vel:0)  === 0   (mute overrides ghost/spread/env shaping)
//   - hitVelocity(vel:100) > 0    (nonzero output unchanged)
// and both weave draw loops (drawConvergence, drawCaptureTimeline) skip drawing
// the hit bar when hitVelocity returns <= 0, so a zero-velocity lane paints no
// hit while nonzero lanes are untouched.
// ---------------------------------------------------------------------------

// Minimal pattern-lane fixture for hitVelocity: envs must be an array
// (envVelFactor iterates it). hit is a non-downbeat step so ghost/timeline
// shading would normally scale a nonzero velocity — proving the mute overrides.
const laneWith = (vel) => ({ vel, spread: 0, ghost: true, timeline: false, envs: [] });
const HIT = { step: 3, acc: false };

test.describe('PolyGrooveMath.hitVelocity velocity-zero mute', () => {
  test('vel 0 returns exactly 0 and vel 100 returns > 0', async ({ page }) => {
    await page.goto(pageUrl);
    const [muted, audible] = await page.evaluate(
      ([l0, l100, hit]) => [
        window.PolyGrooveMath.hitVelocity(l0, 0, 3, hit),
        window.PolyGrooveMath.hitVelocity(l100, 0, 3, hit),
      ],
      [laneWith(0), laneWith(100), HIT],
    );
    expect(muted).toBe(0);
    expect(audible).toBeGreaterThan(0);
  });

  test('mute overrides ghost/spread/env shaping (still 0 with shaping on)', async ({ page }) => {
    await page.goto(pageUrl);
    const shapedMute = await page.evaluate(
      ([l, hit]) => window.PolyGrooveMath.hitVelocity(l, 2, 7, hit),
      [{ vel: 0, spread: 0.8, ghost: true, timeline: true, cells: null, envs: [
        { on: true, target: 'Velocity', depth: 0.9, period: 4 },
      ] }, HIT],
    );
    expect(shapedMute).toBe(0);
  });
});

test.describe('Weave draw loops skip a zero-velocity lane', () => {
  test('muting one lane to velocity 0 reduces painted hit pixels on the loom', async ({ page }) => {
    await page.goto(pageUrl);
    await page.waitForFunction(() => !!document.getElementById('loom'));

    // Baseline: every lane at its default (nonzero) velocity paints hit bars.
    const before = await loomHitPixels(page);
    expect(before).toBeGreaterThan(0); // guard: lanes actually draw hits

    // Mute lane 0 (velocity -> 0). The convergence draw loop must now skip
    // every hit bar for that lane, so fewer non-background pixels are painted.
    await editLaneParam(page, 0, 'velocity', 0);
    const after = await loomHitPixels(page);

    expect(after).toBeLessThan(before);
  });
});
