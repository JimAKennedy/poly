import { test, expect } from '@playwright/test';
import { pageUrl, engineDriftOffsetRef } from './test-helpers.mjs';

// ---------------------------------------------------------------------------
// Drift-math parity tests (M053 S12 T01, D009)
//
// The ring and playhead must move by the SAME drift the C++ engine applies in
// engine.cpp computeDriftedCycleStep. Rather than add a wasm export, D009
// mirrors that one-line formula as a pure JS helper — PolyGrooveMath.driftOffset
// — so these specs guard the mirror against drift from the engine formula.
//
// The reference (engineDriftOffsetRef in test-helpers.mjs) is an independently
// written expression of engine.cpp:361-364; the helper under test lives in
// webui/groove-math.js. Parity across bars, rates, negatives and wraparound is
// what makes the viz trustworthy.
// ---------------------------------------------------------------------------

// steps mirrors ctx.stepsInCycle: cells.length for additive lanes, else steps.
const laneFor = (steps, driftRate, additive) =>
  additive
    ? { cells: new Array(steps).fill(1), driftRate }
    : { steps, driftRate };

// Matrix spans small/large cycles, +/- rates, sub-step and multi-step rates,
// and a wide barPos sweep including negatives so wraparound in both directions
// is exercised.
const STEP_COUNTS = [3, 4, 5, 7, 8, 12, 16];
const RATES = [-4, -2, -1.5, -0.25, 0, 0.25, 0.5, 1, 2.75, 4];
const BAR_POSITIONS = [-13.5, -8, -3.25, -1, -0.5, 0, 0.5, 1, 2.5, 7, 16.75, 40];

test.describe('PolyGrooveMath.driftOffset engine parity', () => {
  test('helper is exported on PolyGrooveMath', async ({ page }) => {
    await page.goto(pageUrl);
    const type = await page.evaluate(() => typeof window.PolyGrooveMath.driftOffset);
    expect(type).toBe('function');
  });

  test('matches the engine formula across bars/rates/steps (pattern lanes)', async ({ page }) => {
    await page.goto(pageUrl);
    for (const steps of STEP_COUNTS) {
      for (const rate of RATES) {
        for (const barPos of BAR_POSITIONS) {
          const actual = await page.evaluate(
            ([s, r, b]) => window.PolyGrooveMath.driftOffset({ steps: s, driftRate: r }, b),
            [steps, rate, barPos],
          );
          const expected = engineDriftOffsetRef(steps, rate, barPos);
          expect(actual, `steps=${steps} rate=${rate} barPos=${barPos}`).toBe(expected);
          // Offset is always a normalized cycle step in [0, steps).
          expect(actual).toBeGreaterThanOrEqual(0);
          expect(actual).toBeLessThan(steps);
        }
      }
    }
  });

  test('additive (cells) lanes use cells.length as the cycle length', async ({ page }) => {
    await page.goto(pageUrl);
    for (const steps of [3, 5, 7, 12]) {
      for (const rate of [-2, -0.25, 0.25, 1, 2.75]) {
        for (const barPos of [-3.25, -1, 0.5, 2.5, 16.75]) {
          const actual = await page.evaluate(
            ([s, r, b]) =>
              window.PolyGrooveMath.driftOffset(
                { cells: new Array(s).fill(1), driftRate: r },
                b,
              ),
            [steps, rate, barPos],
          );
          expect(actual, `cells=${steps} rate=${rate} barPos=${barPos}`).toBe(
            engineDriftOffsetRef(steps, rate, barPos),
          );
        }
      }
    }
  });

  test('zero drift rate and missing rate both yield offset 0', async ({ page }) => {
    await page.goto(pageUrl);
    const results = await page.evaluate(() => [
      window.PolyGrooveMath.driftOffset({ steps: 8, driftRate: 0 }, 5.5),
      window.PolyGrooveMath.driftOffset({ steps: 8 }, 5.5),
      window.PolyGrooveMath.driftOffset({ cells: [1, 1, 1], driftRate: 0 }, -9.25),
    ]);
    expect(results).toEqual([0, 0, 0]);
  });

  test('negative drift wraps forward into [0, steps) (no negative steps)', async ({ page }) => {
    await page.goto(pageUrl);
    // barPos=1, rate=-1 → floor(-1) = -1 → wraps to steps-1.
    const wrapped = await page.evaluate(() =>
      window.PolyGrooveMath.driftOffset({ steps: 12, driftRate: -1 }, 1),
    );
    expect(wrapped).toBe(11);
    expect(wrapped).toBe(laneFor(12, -1, false).steps - 1);
  });
});
