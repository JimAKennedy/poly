import { test, expect } from '@playwright/test';
import { pageUrl } from './test-helpers.mjs';


// ---------------------------------------------------------------------------
// Deterministic explicit accent in the WebUI decorative preview (M073 S02 T02).
//
// The engine (computeStepVelocity, engine.cpp) applies a per-step accent as a
// deterministic proportional-headroom boost, decoupled from the emphasisProb
// roll: vel += accentVal * kAccentVelocityBoost * (1 - vel), kAccentVelocityBoost
// = 0.6. PolyGrooveMath.hitVelocity now mirrors that exact formula by reading
// l.accents[hit.step], so toggling an accent visibly raises the drawn hit bar.
//
// Contract (slice Proof Level):
//   - hitVelocity with accents[step]=1 returns base + 0.6*(1-base) for that step
//   - the boost always increases velocity for a set step and stays <= 1 (no saturation)
//   - it is seed-independent: applies on top of spread/ghost shading, not gated by it
//   - toggling an accent through the host increases painted hit pixels on the loom
// ---------------------------------------------------------------------------

// kAccentVelocityBoost mirror — kept structurally separate from groove-math.js so
// this spec checks the number the engine uses (engine.cpp), not the JS constant
// against itself.
const ACCENT_BOOST = 0.6;

// Minimal pattern-lane fixture: envs must be an array (envVelFactor iterates it).
// A neutral fixture (no spread/ghost/timeline/cells, neutral env) isolates the
// accent term so the returned velocity is exactly base + boost.
const laneWith = (vel, accents) => ({ vel, spread: 0, ghost: false, timeline: false, cells: null, envs: [], accents });
const HIT = (step) => ({ step, acc: step === 0 });

test.describe('PolyGrooveMath.hitVelocity explicit accent boost', () => {
  test('accents[step]=1 applies the engine proportional-headroom boost and stays < 1', async ({ page }) => {
    await page.goto(pageUrl);
    const [plain, accented] = await page.evaluate(
      ([lPlain, lAccent, hit]) => [
        window.PolyGrooveMath.hitVelocity(lPlain, 0, 4, hit),
        window.PolyGrooveMath.hitVelocity(lAccent, 0, 4, hit),
      ],
      [laneWith(80, [0, 0, 0]), laneWith(80, [0, 0, 1]), HIT(2)],
    );
    const base = 80 / 127;
    // Numerically identical to the engine formula for accentVal = 1.
    expect(accented).toBeCloseTo(base + ACCENT_BOOST * (1 - base), 6);
    // Always boosts the set step...
    expect(accented).toBeGreaterThan(plain);
    // ...and never saturates a mid-velocity lane.
    expect(accented).toBeLessThan(1);
  });

  test('graduated accent value scales proportionally and clamps to <= 1', async ({ page }) => {
    await page.goto(pageUrl);
    const vals = await page.evaluate(
      ([mk, hit]) =>
        [0, 0.5, 1].map((a) => window.PolyGrooveMath.hitVelocity({ ...mk, accents: [a] }, 0, 4, hit)),
      [laneWith(80, [0]), HIT(0)],
    );
    const base = 80 / 127;
    expect(vals[0]).toBeCloseTo(base, 6);
    expect(vals[1]).toBeCloseTo(base + 0.5 * ACCENT_BOOST * (1 - base), 6);
    expect(vals[2]).toBeCloseTo(base + ACCENT_BOOST * (1 - base), 6);
    // Monotonic and bounded.
    expect(vals[1]).toBeGreaterThan(vals[0]);
    expect(vals[2]).toBeGreaterThan(vals[1]);
    expect(vals[2]).toBeLessThanOrEqual(1);
  });

  test('accent boost is seed-independent — stacks on spread/ghost shading rather than being gated by it', async ({ page }) => {
    await page.goto(pageUrl);
    // Same lane/li/tick with spread+ghost shading ON: the accent must strictly
    // raise the shaded velocity (it is decoupled from the shade() roll, not gated).
    const [shaded, shadedAccent] = await page.evaluate(
      ([lPlain, lAccent, hit]) => [
        window.PolyGrooveMath.hitVelocity(lPlain, 3, 7, hit),
        window.PolyGrooveMath.hitVelocity(lAccent, 3, 7, hit),
      ],
      [
        { vel: 80, spread: 0.6, ghost: true, timeline: false, cells: null, envs: [], accents: [0, 0, 0, 0] },
        { vel: 80, spread: 0.6, ghost: true, timeline: false, cells: null, envs: [], accents: [0, 0, 0, 1] },
        HIT(3),
      ],
    );
    expect(shadedAccent).toBeGreaterThan(shaded);
    expect(shadedAccent).toBeLessThanOrEqual(1);
  });

  test('absent or zero accents leaves velocity unchanged', async ({ page }) => {
    await page.goto(pageUrl);
    const [noArray, zeros] = await page.evaluate(
      ([hit]) => [
        window.PolyGrooveMath.hitVelocity({ vel: 80, spread: 0, ghost: false, timeline: false, cells: null, envs: [] }, 0, 4, hit),
        window.PolyGrooveMath.hitVelocity({ vel: 80, spread: 0, ghost: false, timeline: false, cells: null, envs: [], accents: [0, 0, 0] }, 0, 4, hit),
      ],
      [HIT(2)],
    );
    const base = 80 / 127;
    expect(noArray).toBeCloseTo(base, 6);
    expect(zeros).toBeCloseTo(base, 6);
  });
});
